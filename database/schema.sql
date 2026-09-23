-- One row per drive session.
CREATE TABLE sessions (
    id          SERIAL PRIMARY KEY,
    vehicle_id  TEXT        NOT NULL,
    started_at  TIMESTAMPTZ NOT NULL,
    ended_at    TIMESTAMPTZ            -- NULL while the session is live
);

-- The signals, defined once. Mirrors the C++ signal table.
CREATE TABLE signals (
    id          SERIAL PRIMARY KEY,
    name        TEXT NOT NULL UNIQUE,
    unit        TEXT NOT NULL,
    min_value   DOUBLE PRECISION NOT NULL,
    max_value   DOUBLE PRECISION NOT NULL
);

-- The bulk table: one row per signal reading. BIGSERIAL because at ~400
-- rows/sec a 32-bit key would run out in about two months.
--
-- Two timestamps, deliberately. vehicle_time is when the vehicle observed
-- the value; recorded_at is when this system stored it. The gap between them
-- is ingestion lag. Analysis indexes on vehicle_time, because batched inserts
-- share a single recorded_at across the whole transaction.
CREATE TABLE telemetry (
    id           BIGSERIAL PRIMARY KEY,
    session_id   INTEGER NOT NULL REFERENCES sessions(id),
    signal_id    INTEGER NOT NULL REFERENCES signals(id),
    vehicle_time DOUBLE PRECISION NOT NULL,
    recorded_at  TIMESTAMPTZ NOT NULL,
    value        DOUBLE PRECISION NOT NULL
);

-- Static DTC definitions: description and severity never change per code,
-- so they are stored once rather than on every event row.
CREATE TABLE dtc_catalog (
    code        TEXT PRIMARY KEY,
    description TEXT NOT NULL,
    severity    SMALLINT NOT NULL
);

-- One row per DTC state change. Only transitions are recorded, not the
-- ongoing condition.
CREATE TABLE dtc_events (
    id           SERIAL PRIMARY KEY,
    session_id   INTEGER NOT NULL REFERENCES sessions(id),
    code         TEXT    NOT NULL REFERENCES dtc_catalog(code),
    active       BOOLEAN NOT NULL,
    vehicle_time DOUBLE PRECISION NOT NULL,
    occurred_at  TIMESTAMPTZ NOT NULL
);

-- Column order matters: (session_id, vehicle_time) serves both the filter
-- and the ordering of the most common query. The reverse would not.
CREATE INDEX idx_telemetry_session_time ON telemetry(session_id, vehicle_time);
CREATE INDEX idx_telemetry_signal       ON telemetry(signal_id);
CREATE INDEX idx_dtc_events_session     ON dtc_events(session_id);