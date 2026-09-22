"""
Dashboard backend. Reads telemetry that the C++ pipeline wrote to Postgres
and serves it as JSON. The two never talk directly — the database is the
handoff point, which is how real telemetry systems separate ingestion from
query.
"""

import os
import psycopg2                                   # PostgreSQL driver
from flask import Flask, jsonify, send_from_directory

# static_folder tells Flask where the HTML/JS lives.
app = Flask(__name__, static_folder='static')

# Same environment variable the C++ side uses, with the same local default.
# Keeps credentials out of the source when this runs anywhere real.
DB = os.environ.get('TELEMETRY_DB', 'dbname=vehicle_telemetry')


def query(sql, params=()):
    """Run one query and return all rows.

    The nested `with` blocks close the cursor and connection even if the
    query raises — Python's version of RAII.
    """
    with psycopg2.connect(DB) as conn:
        with conn.cursor() as cur:
            # params go separately from the SQL text so they can never be
            # interpreted as SQL. Never build queries with f-strings.
            cur.execute(sql, params)
            return cur.fetchall()


@app.route('/')
def index():
    # Serves the dashboard page itself.
    return send_from_directory('static', 'index.html')


@app.route('/api/latest')
def latest():
    """Current state: the newest session and its most recent reading
    per signal."""

    # Highest id is the newest session.
    rows = query("""
        SELECT id, vehicle_id, started_at, ended_at
        FROM sessions ORDER BY id DESC LIMIT 1
    """)

    # No sessions at all — the pipeline has never run against this database.
    if not rows:
        return jsonify({'session': None})

    sid, vehicle, started, ended = rows[0]

    # DISTINCT ON is Postgres-specific: one row per signal name, and the
    # ORDER BY decides which one — recorded_at DESC means the latest.
    # Gets "current value of every signal" in a single query.
    signals = query("""
        SELECT DISTINCT ON (s.name) s.name, t.value, s.unit
        FROM telemetry t
        JOIN signals s ON s.id = t.signal_id
        WHERE t.session_id = %s
        ORDER BY s.name, t.recorded_at DESC
    """, (sid,))

    return jsonify({
        'session': {
            'id': sid,
            'vehicle': vehicle,
            # isoformat() gives the browser a string it can parse.
            'started': started.isoformat(),
            'ended': ended.isoformat() if ended else None,
            # NULL ended_at means the pipeline is still writing.
            # This is what lets the page distinguish live from historical.
            'live': ended is None,
        },
        # Turn the rows into {name: {value, unit}} so the page can look
        # up a signal by name instead of hunting through a list.
        'signals': {name: {'value': val, 'unit': unit}
                    for name, val, unit in signals},
    })


if __name__ == '__main__':
    # Locally we bind to localhost; Compose sets FLASK_HOST=0.0.0.0 so the
    # container is reachable. The debugger allows arbitrary code execution,
    # so it is only ever enabled when bound to localhost.
    host = os.environ.get('FLASK_HOST', '127.0.0.1')
    app.run(host=host, port=5000, debug=(host == '127.0.0.1'))