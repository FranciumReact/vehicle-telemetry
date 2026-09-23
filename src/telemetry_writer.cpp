#include "telemetry_writer.hpp"
#include <cstdio>
#include <string>

TelemetryWriter::TelemetryWriter(const std::string& conn_string)
    : conn_(conn_string) {
    // Cache the signal name -> id mapping once, so inserts don't
    // need a lookup query per row.
    pqxx::work tx(conn_);
    pqxx::result res = tx.exec("SELECT id, name FROM signals");
    for (const auto& row : res) {
        signal_ids_[row[1].as<std::string>()] = row[0].as<int>();
    }
    tx.commit();
}

TelemetryWriter::~TelemetryWriter() {
    // RAII: whatever is still buffered gets written, and the session is
    // closed, before we die.
    try {
        flush();
        end_session();
    } catch (const std::exception& e) {
        // A destructor must never throw, but failing silently loses data
        // with no trace, so at least report it.
        fprintf(stderr, "TelemetryWriter shutdown failed: %s\n", e.what());
    }
}

int TelemetryWriter::start_session(const std::string& vehicle_id) {
    pqxx::work tx(conn_);
    pqxx::row row = tx.exec_params1(
        "INSERT INTO sessions (vehicle_id, started_at) "
        "VALUES ($1, now()) RETURNING id",
        vehicle_id);
    session_id_ = row[0].as<int>();
    tx.commit();
    return session_id_;
}

void TelemetryWriter::add(const DecodedFrame& values, double time_s) {
    for (const auto& [name, value] : values) {
        auto it = signal_ids_.find(name);
        if (it == signal_ids_.end()) continue;   // signal not in the table

        buffer_.push_back({it->second, time_s, value});
    }

    // Bounded loss: never hold more than kBatchSize rows in memory.
    if (buffer_.size() >= kBatchSize) flush();
}

void TelemetryWriter::flush() {
    if (buffer_.empty()) return;

    // One multi-row INSERT rather than one statement per row: a single
    // round trip and a single commit for the whole batch.
    std::string sql =
        "INSERT INTO telemetry "
        "(session_id, signal_id, vehicle_time, recorded_at, value) VALUES ";

    pqxx::params params;
    int n = 1;

    for (size_t i = 0; i < buffer_.size(); i++) {
        const Row& r = buffer_[i];

        if (i > 0) sql += ", ";
        sql += "($" + std::to_string(n)      + ", $" + std::to_string(n + 1)
             + ", $" + std::to_string(n + 2) + ", now(), $" + std::to_string(n + 3)
             + ")";
        n += 4;

        // Only the placeholder text is built here. The values themselves are
        // still sent separately, so this is not string interpolation.
        params.append(session_id_);
        params.append(r.signal_id);
        params.append(r.time_s);
        params.append(r.value);
    }

    pqxx::work tx(conn_);
    tx.exec_params(sql, params);
    tx.commit();
    buffer_.clear();
}

void TelemetryWriter::end_session() {
    pqxx::work tx(conn_);
    tx.exec_params(
        "UPDATE sessions SET ended_at = now() WHERE id = $1",
        session_id_);
    tx.commit();
}

void TelemetryWriter::log_dtc(const std::string& code, bool active,
                              double vehicle_time) {
    pqxx::work tx(conn_);
    tx.exec_params(
        "INSERT INTO dtc_events "
        "(session_id, code, active, vehicle_time, occurred_at) "
        "VALUES ($1, $2, $3, $4, now())",
        session_id_, code, active, vehicle_time);
    tx.commit();
}