-- One row per drive session.
CREATE TABLE sessions (
    id          SERIAL PRIMARY KEY,
    vehicle_id  TEXT        NOT NULL,
    started_at  TIMESTAMPTZ NOT NULL,
    ended_at    TIMESTAMPTZ
);

-- The 14 signals, defined once. Mirrors the C++ signal table.
CREATE TABLE signals (
    id          SERIAL PRIMARY KEY,
    name        TEXT NOT NULL UNIQUE,
    unit        TEXT NOT NULL,
    min_value   DOUBLE PRECISION NOT NULL,
    max_value   DOUBLE PRECISION NOT NULL
);

-- The bulk table: one row per signal reading.
CREATE TABLE telemetry (
    id          BIGSERIAL PRIMARY KEY,
    session_id  INTEGER NOT NULL REFERENCES sessions(id),
    signal_id   INTEGER NOT NULL REFERENCES signals(id),
    recorded_at TIMESTAMPTZ NOT NULL,
    value       DOUBLE PRECISION NOT NULL
);

-- Static DTC definitions.
CREATE TABLE dtc_catalog (
    code        TEXT PRIMARY KEY,
    description TEXT NOT NULL,
    severity    SMALLINT NOT NULL
);

-- One row per DTC state change.
CREATE TABLE dtc_events (
    id          SERIAL PRIMARY KEY,
    session_id  INTEGER NOT NULL REFERENCES sessions(id),
    code        TEXT    NOT NULL REFERENCES dtc_catalog(code),
    active      BOOLEAN NOT NULL,
    occurred_at TIMESTAMPTZ NOT NULL
);

CREATE INDEX idx_telemetry_session_time ON telemetry(session_id, recorded_at);
CREATE INDEX idx_telemetry_signal       ON telemetry(signal_id);
CREATE INDEX idx_dtc_events_session     ON dtc_events(session_id);