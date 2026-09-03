#!/usr/bin/env python3

import numpy as np

#file_low = "1stCorr_0_5_ns_binning/80_91/driftlength_relation_X2.txt"
#file_high  = "1stCorr_0_5_ns_binning/80_130/driftlength_relation_X2.txt"

#file_low = "4thCorr_0_5_ns_binning/80_to_91/driftlength_relation_U.txt"
#file_high  = "4thCorr_0_5_ns_binning/80_to_130/driftlength_relation_U.txt"

file_low = "4thCorr_0_5_ns_binning_250_ns_max/80_to_91/driftlength_relation_U.txt"
file_high  = "4thCorr_0_5_ns_binning_250_ns_max/80_to_130/driftlength_relation_U.txt"



outfile   = "driftlength_relation_U_merged.txt"


def read_table(filename):
    with open(filename) as f:
        header = f.readline().strip().split("\t")

    angles = np.array([float(x) for x in header[1:]])
    data = np.loadtxt(filename, skiprows=1)

    times = data[:, 0]
    values = data[:, 1:]

    return times, angles, values


# Tabellen laden
times1, angles1, values1 = read_table(file_low)
times2, angles2, values2 = read_table(file_high)

# Sicherheit
if not np.allclose(times1, times2):
    raise RuntimeError("Zeitachsen stimmen nicht überein!")

# gewünschte Winkel auswählen
#mask1 = (angles1 >= 20) & (angles1 <= 50) # | (angles1 >= 26) & (angles1 <= 51) | (angles1 >= 56)
#mask2 = (angles2 >= 51) & (angles2 <= 65) # & (angles2 <= 55)
mask1 = (angles1 >= 20) & (angles1 <= 49)
mask2 = (angles2 >= 50) & (angles2 <= 65)


angles = np.concatenate((angles1[mask1], angles2[mask2]))
values = np.hstack((values1[:, mask1], values2[:, mask2]))

# schreiben
with open(outfile, "w") as f:
    f.write("t(ns)\t" + "\t".join(f"{a:.6f}" for a in angles) + "\n")

    for t, row in zip(times1, values):
        f.write(f"{t:.1f}")
        for v in row:
            f.write(f"\t{v:.10f}")
        f.write("\n")

print(f"Geschrieben: {outfile}")
