import numpy as np
import matplotlib.pyplot as plt


# ============================================================
# Einstellungen
# ============================================================

FILE_1 = "driftlength_relation_X1_improved.txt" ## SIMULATION
FILE_2 = "driftlength_relation_X1.txt" ## MESSUNG

PHI_ANGLES = [20, 25, 30, 35, 40, 45, 50, 55, 60, 65]


# ============================================================
# Datei einlesen
# ============================================================

def read_drift_file(filename):
    """
    Liest eine Driftzeit -> Spurversatz-Tabelle ein.

    Erste Zeile:
        t(ns)   20.000000   21.000000 ...

    Danach:
        Driftzeit   Spurversatz(phi=20)   Spurversatz(phi=21) ...
    """

    # Kopfzeile separat lesen
    with open(filename, "r") as f:
        header = f.readline().split()

    # Erste Spalte ist die Driftzeit
    angles = np.array([float(x) for x in header[1:]])

    # Daten einlesen
    data = np.loadtxt(filename, skiprows=1)

    drift_time = data[:, 0]
    spurversatz = data[:, 1:]

    return drift_time, angles, spurversatz


# ============================================================
# Daten einlesen
# ============================================================

time1, angles1, offset1 = read_drift_file(FILE_1)
time2, angles2, offset2 = read_drift_file(FILE_2)


# ============================================================
# Plot
# ============================================================

plt.figure(figsize=(10, 7))


for phi in PHI_ANGLES:

    # --------------------------------------------------------
    # Winkel in Datei 1 suchen
    # --------------------------------------------------------

    idx1 = np.where(np.isclose(angles1, phi, atol=1e-6))[0]

    # --------------------------------------------------------
    # Winkel in Datei 2 suchen
    # --------------------------------------------------------

    idx2 = np.where(np.isclose(angles2, phi, atol=1e-6))[0]

    # --------------------------------------------------------
    # Falls Winkel nicht vorhanden
    # --------------------------------------------------------

    if len(idx1) == 0:
        print(f"Warnung: phi = {phi}° nicht in {FILE_1} gefunden.")

    if len(idx2) == 0:
        print(f"Warnung: phi = {phi}° nicht in {FILE_2} gefunden.")

    # --------------------------------------------------------
    # Daten aus Datei 1 plotten
    # --------------------------------------------------------

    if len(idx1) > 0:
        i = idx1[0]

        plt.plot(
            time1,
            offset1[:, i],
            label=fr"$\phi={phi}^\circ$ – Datei 1"
        )

    # --------------------------------------------------------
    # Daten aus Datei 2 plotten
    # --------------------------------------------------------

    if len(idx2) > 0:
        i = idx2[0]

        plt.plot(
            time2,
            offset2[:, i],
            linestyle="--",
            label=fr"$\phi={phi}^\circ$ – Datei 2"
        )


# ============================================================
# Plot-Einstellungen
# ============================================================

plt.xlabel("Driftzeit [ns]")
plt.ylabel("Spurversatz [mm]")

plt.title("Spurversatz vs. Driftzeit")

plt.grid(True, alpha=0.3)

plt.legend(
    ncol=2,
    fontsize=9
)

plt.tight_layout()

plt.show()
