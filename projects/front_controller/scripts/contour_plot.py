import csv
import numpy as np
import matplotlib.pyplot as plt

PEDAL_COL = 1   # pedal input (normalized)
ACCEL_COL = 2   # accel percentage
STATE_COL = 3   # 0 = accel, 1 = braking
SPEED_COL = 4   # speed (0.0 to 1.0 in 0.1 steps)

rows = []
with open('../test/test_results/opd_curve.csv', 'r', encoding='utf-8') as f:
    reader = csv.reader(f)
    next(reader)
    for r in reader:
        pedal = float(r[PEDAL_COL])
        accel = float(r[ACCEL_COL])
        state = int(r[STATE_COL])
        speed = float(r[SPEED_COL])
        if accel == -1:
            continue
        signed_accel = accel if state == 0 else -accel
        rows.append((pedal, speed, signed_accel))

pedals = sorted({p for p, s, a in rows})
speeds = sorted({s for p, s, a in rows})
P = {p: i for i, p in enumerate(pedals)}
S = {s: i for i, s in enumerate(speeds)}

A = np.full((len(speeds), len(pedals)), np.nan)
for p, s, a in rows:
    A[S[s], P[p]] = a

min_a = np.nanmin(A)
max_a = np.nanmax(A)
abs_max = max(abs(min_a), abs(max_a))
levels = np.linspace(-abs_max, abs_max, 21)

fig, ax = plt.subplots(figsize=(7, 5))
c = ax.contourf(pedals, speeds, A, levels=levels, cmap='RdYlGn')
cb = fig.colorbar(c, ax=ax)
cb.set_label('Signed Acceleration (pos=Accelerating, neg=Braking)')

ax.set_xlabel('Pedal input (normalized)')
ax.set_ylabel('Speed (normalized)')
ax.set_title('Pedal-Speed Contour')
ax.grid(True)
plt.tight_layout()
plt.show()
