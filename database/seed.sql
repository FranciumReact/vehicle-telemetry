INSERT INTO signals (name, unit, min_value, max_value) VALUES
    ('EngineRPM',         'rpm',  0,    8000),
    ('ThrottlePos',       '%',    0,    100),
    ('EngineLoad',        '%',    0,    100),
    ('CoolantTemp',       'C',    -40,  130),
    ('VehicleSpeed',      'km/h', 0,    300),
    ('AcceleratorPos',    '%',    0,    100),
    ('BrakePressed',      'bool', 0,    1),
    ('BatteryVoltage',    'V',    0,    20),
    ('AmbientTemp',       'C',    -40,  80),
    ('TransmissionState', 'enum', 0,    4),
    ('FuelRate',          'L/h',  0,    60),
    ('DtcCode',           'enum', 0,    65535),
    ('DtcSeverity',       'enum', 0,    3),
    ('DtcActive',         'bool', 0,    1),
    ('SourceEcu',         'enum', 0,    15);

INSERT INTO dtc_catalog (code, description, severity) VALUES
    ('P0001', 'Coolant temperature above limit',     2),
    ('P0002', 'Battery voltage below limit',         1),
    ('P0003', 'Engine RPM above limit',              3),
    ('P0004', 'Sensor value outside physical range', 1),
    ('P0005', 'Implausible rate of change',          1);