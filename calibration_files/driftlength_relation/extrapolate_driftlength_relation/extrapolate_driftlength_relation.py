#!/usr/bin/env python3

import numpy as np


# ============================================================
# Parameters
# ============================================================

INPUT_FILE = "driftlength_relation_U.txt"
OUTPUT_FILE = "driftlength_relation_U_extrapolated.txt"

# Range used for the linear fit
FIT_MIN = 100.0       # ns
FIT_MAX = 200.0       # ns

# Start and end of the extrapolation
EXTRAPOLATE_FROM = 200.0   # ns
EXTRAPOLATE_TO   = 350.0   # ns


# ============================================================
# Read input table
# ============================================================

with open(INPUT_FILE, "r") as f:
    lines = f.readlines()


# Header
header = lines[0].rstrip("\n").split("\t")

# Drift times and table data
times = []
data = []

for line in lines[1:]:
    line = line.strip()

    if not line:
        continue

    values = line.split("\t")

    times.append(float(values[0]))
    data.append([float(x) for x in values[1:]])


times = np.asarray(times)
data = np.asarray(data)

# Angles from header
angles = np.asarray([float(x) for x in header[1:]])

n_angles = len(angles)

if data.shape[1] != n_angles:
    raise RuntimeError(
        f"Number of angles ({n_angles}) does not match "
        f"number of data columns ({data.shape[1]})."
    )


# ============================================================
# Check parameters
# ============================================================

if EXTRAPOLATE_FROM > EXTRAPOLATE_TO:
    raise RuntimeError(
        "EXTRAPOLATE_FROM must be smaller than EXTRAPOLATE_TO."
    )

if FIT_MIN > FIT_MAX:
    raise RuntimeError(
        "FIT_MIN must be smaller than FIT_MAX."
    )


# ============================================================
# Construct output time axis
# ============================================================

# Determine time step from input table
if len(times) < 2:
    raise RuntimeError("Input table contains fewer than two time points.")

dt = times[1] - times[0]

if dt <= 0:
    raise RuntimeError("Invalid time spacing in input table.")


# Keep all original time points below EXTRAPOLATE_FROM.
output_times = list(times[times < EXTRAPOLATE_FROM])

# Add extrapolated time points from EXTRAPOLATE_FROM to EXTRAPOLATE_TO.
extrap_times = np.arange(
    EXTRAPOLATE_FROM,
    EXTRAPOLATE_TO + 0.5 * dt,
    dt
)

# Avoid duplicate time point if EXTRAPOLATE_FROM already exists
output_times.extend(extrap_times)

output_times = np.asarray(output_times)


# ============================================================
# Extrapolate each angle independently
# ============================================================

output_data = np.zeros(
    (len(output_times), n_angles)
)


for i, angle in enumerate(angles):

    # --------------------------------------------------------
    # Select fit range
    # --------------------------------------------------------

    fit_mask = (
        (times >= FIT_MIN) &
        (times <= FIT_MAX)
    )

    fit_times = times[fit_mask]
    fit_lengths = data[fit_mask, i]

    if len(fit_times) < 2:
        raise RuntimeError(
            f"Not enough points for angle {angle:.1f} deg "
            f"in fit range {FIT_MIN}--{FIT_MAX} ns."
        )

    # --------------------------------------------------------
    # Linear fit
    # --------------------------------------------------------

    slope, intercept = np.polyfit(
        fit_times,
        fit_lengths,
        1
    )

    print(
        f"phi = {angle:6.2f} deg : "
        f"slope = {slope:.8e} mm/ns, "
        f"intercept = {intercept:.8e} mm"
    )

    # --------------------------------------------------------
    # Fill output table
    # --------------------------------------------------------

    # Original values below EXTRAPOLATE_FROM
    original_mask = output_times < EXTRAPOLATE_FROM

    # Find corresponding original values
    for j, t in enumerate(output_times[original_mask]):

        # Find matching input time
        idx = np.argmin(np.abs(times - t))

        if not np.isclose(times[idx], t):
            raise RuntimeError(
                f"Could not match time {t} ns to input table."
            )

        output_data[j, i] = data[idx, i]

    # Extrapolated values
    extrap_mask = output_times >= EXTRAPOLATE_FROM

    output_data[extrap_mask, i] = (
        slope * output_times[extrap_mask] + intercept
    )


# ============================================================
# Write output table
# ============================================================

with open(OUTPUT_FILE, "w") as f:

    # Header
    f.write("\t".join(header) + "\n")

    # Data
    for j, time in enumerate(output_times):

        row = [f"{time:.1f}"]

        for i in range(n_angles):
            row.append(
                f"{output_data[j, i]:.10f}"
            )

        f.write("\t".join(row) + "\n")


# ============================================================
# Done
# ============================================================

print()
print("Done.")
print(f"Input :  {INPUT_FILE}")
print(f"Output:  {OUTPUT_FILE}")
print()
print(f"Fit range        : {FIT_MIN:.1f} -- {FIT_MAX:.1f} ns")
print(f"Extrapolation    : {EXTRAPOLATE_FROM:.1f} -- {EXTRAPOLATE_TO:.1f} ns")
print(f"Time step        : {dt:.1f} ns")
print(f"Output rows      : {len(output_times)}")
