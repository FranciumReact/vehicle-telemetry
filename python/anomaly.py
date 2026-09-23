"""Statistical anomaly detection over stored telemetry sessions.

Three approaches, deliberately compared. None of them diagnoses anything:
they identify samples that are statistically unlike the rest, which is not
the same as identifying a fault, predicting a failure, or naming a cause.
"""

import sys
import numpy as np
import pandas as pd

from analyse import load_session


def zscore(series, threshold=3.0):
    """Distance from the mean in standard deviations.

    Assumes roughly normal data. Its weakness is masking: the outliers
    inflate the very standard deviation used to judge them, so a large or
    sustained anomaly can score lower than a brief one.
    """
    mean, std = series.mean(), series.std()
    if std == 0:
        return pd.Series(False, index=series.index)
    return (series - mean).abs() / std > threshold


def modified_zscore(series, threshold=3.5):
    """Median-based equivalent, robust to contaminated baselines.

    The median and MAD barely move when a minority of samples are extreme,
    so the baseline stays anchored to healthy data. 0.6745 scales MAD to be
    comparable to a standard deviation for normal data.
    """
    median = series.median()
    mad = (series - median).abs().median()
    if mad == 0:
        return pd.Series(False, index=series.index)
    return 0.6745 * (series - median).abs() / mad > threshold

def rolling_zscore(series, window=200, threshold=3.0):
    """Compare each point to the preceding window rather than the whole
    session. Catches gradual drift that global statistics smooth away;
    misses slow shifts that span the entire session.
    """
    # shift(1) excludes the current point from the baseline it is judged
    # against — otherwise every sample helps define its own normal.
    mean = series.rolling(window).mean().shift(1)
    std = series.rolling(window).std().shift(1)

    # The first `window` samples have no baseline yet, so they cannot be
    # judged. fillna(False) marks them as not-anomalous rather than unknown.
    return ((series - mean).abs() / std > threshold).fillna(False)


if __name__ == '__main__':
    session = int(sys.argv[1]) if len(sys.argv) > 1 else 1
    df = load_session(session)

    if df.empty:
        print(f"Session {session} has no telemetry.")
        sys.exit(1)

    methods = {
        'z-score':       zscore,
        'modified z':    modified_zscore,
        'rolling z':     rolling_zscore,
    }

    print(f"Session {session}: samples flagged as anomalous\n")
    print(f"{'signal':<14}" + ''.join(f"{m:>14}" for m in methods))

    for name in sorted(df.columns):
        counts = [str(int(fn(df[name]).sum())) for fn in methods.values()]
        print(f"{name:<14}" + ''.join(f"{c:>14}" for c in counts))