"""Offline analysis of stored telemetry sessions.

The pipeline processes one frame at a time with no memory beyond the last
value — that's streaming. This loads a whole session into memory and computes
over all of it at once — that's batch. Some questions only batch can answer.
"""

import os
import sys
import pandas as pd
import psycopg2

DB = os.environ.get('TELEMETRY_DB', 'dbname=vehicle_telemetry')


def load_session(session_id):
    """Return one session's telemetry as a wide DataFrame:
    one row per vehicle timestamp, one column per signal."""
    sql = """
        SELECT t.vehicle_time, s.name, t.value
        FROM telemetry t
        JOIN signals s ON s.id = t.signal_id
        WHERE t.session_id = %s
        ORDER BY t.vehicle_time
    """
    with psycopg2.connect(DB) as conn:
        with conn.cursor() as cur:
            # Parameters travel separately from the SQL so they can never
            # be interpreted as SQL.
            cur.execute(sql, (session_id,))
            long = pd.DataFrame(cur.fetchall(),
                                columns=['vehicle_time', 'name', 'value'])

    # Long -> wide. The database stores one row per reading so that adding a
    # signal needs no schema change; analysis wants one column per signal.
    # Indexed on vehicle_time, not recorded_at: batched inserts share a single
    # recorded_at across the whole transaction, so it isn't unique per sample.
    return long.pivot(index='vehicle_time', columns='name', values='value')


def summarise(df):
    """Per-signal statistics. describe() computes them for every column at
    once; .T transposes so signals are rows, which reads better."""
    return df.describe().T[['count', 'mean', 'std', 'min', 'max']]


def load_lag(session_id):
    """Ingestion lag: how long after the vehicle observed a value did it
    reach the database?

    vehicle_time is seconds since session start; recorded_at is an absolute
    timestamp. started_at is the common origin that makes them comparable.
    """
    sql = """
        SELECT t.vehicle_time,
               EXTRACT(EPOCH FROM (t.recorded_at - s.started_at))
                   - t.vehicle_time AS lag_s
        FROM telemetry t
        JOIN sessions s ON s.id = t.session_id
        WHERE t.session_id = %s
        ORDER BY t.vehicle_time
    """
    with psycopg2.connect(DB) as conn:
        with conn.cursor() as cur:
            cur.execute(sql, (session_id,))
            return pd.DataFrame(cur.fetchall(),
                                columns=['vehicle_time', 'lag_s'])


if __name__ == '__main__':
    session = int(sys.argv[1]) if len(sys.argv) > 1 else 1
    df = load_session(session)

    # Missing data and a crash should not look the same to the user.
    if df.empty:
        print(f"Session {session} has no telemetry.")
        sys.exit(1)

    duration = df.index.max() - df.index.min()
    print(f"Session {session}: {len(df)} samples over {duration:.1f}s, "
          f"{len(df.columns)} signals\n")
    print(summarise(df).round(2))

    # Postgres returns NUMERIC as Decimal, which pandas cannot average.
    lag = load_lag(session)
    lag['lag_s'] = lag['lag_s'].astype(float)

    print(f"\nIngestion lag (seconds):")
    print(f"  mean   {lag['lag_s'].mean():.4f}")
    print(f"  median {lag['lag_s'].median():.4f}")
    print(f"  p95    {lag['lag_s'].quantile(0.95):.4f}")
    print(f"  max    {lag['lag_s'].max():.4f}")