import csv
import matplotlib.pyplot as plt
from collections import defaultdict

PEDAL_COL = 1   # pedal input (normalized)
ACCEL_COL = 2   # accel percentage
STATE_COL = 3   # 0 = accel, 1 = braking
SPEED_COL = 4   # speed (0.0 to 1.0 in 0.1 steps)

groups = defaultdict(list)
with open('test_results/opd_curve.csv', 'r') as f:
    reader = csv.reader(f)
    next(reader, None)
    for row in reader:
        x = float(row[PEDAL_COL])
        y = float(row[ACCEL_COL])
        state = int(row[STATE_COL])
        speed = round(float(row[SPEED_COL]), 1)
        groups[speed].append((x, y, state))

for speed in sorted(groups.keys()):
    pts = sorted(groups[speed], key=lambda p: p[0]) 
    for i in range(len(pts) - 1):
        x0, y0, s0 = pts[i]
        x1, y1, s1 = pts[i + 1]

        if y0 == -1 or y1 == -1:
            continue

        color = 'green' if s0 == 0 else 'red'
        alpha = 0.2 + 0.8 * speed
        plt.plot([x0, x1], [y0, y1], color=color, alpha=alpha)

plt.xlabel('Pedal input (normalized)')
plt.ylabel('Accel percentage')
plt.title('Pedal input vs Accel percentage\nColor = State, Opacity = Speed')
plt.grid(True)

import matplotlib.lines as mlines
green_line = mlines.Line2D([], [], color='green', label='State 0')
red_line   = mlines.Line2D([], [], color='red',   label='State 1')
plt.legend(handles=[green_line, red_line], title="State")

plt.show()
