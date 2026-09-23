#pragma once
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <unordered_map>
#include "can_decode.hpp"

class TelemetryWriter {
public:
    explicit TelemetryWriter(const std::string& conn_string);
    ~TelemetryWriter();

    int  start_session(const std::string& vehicle_id);
    void add(const DecodedFrame& values, double time_s);
    void flush();
    void end_session();
    void log_dtc(const std::string& code, bool active, double vehicle_time);

private:
    pqxx::connection conn_;
    int              session_id_ = 0;
    std::unordered_map<std::string, int> signal_ids_;

    struct Row { int signal_id; double time_s; double value; };
    std::vector<Row> buffer_;

    static constexpr size_t kBatchSize = 100;
};