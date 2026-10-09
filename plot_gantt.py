import csv
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

if len(sys.argv) != 3:
    print("Usage: python3 plot_gantt.py input.csv output.png")
    sys.exit(1)

input_file = sys.argv[1]
output_file = sys.argv[2]

with open(input_file, encoding="utf-8", newline="") as f:
    rows = list(csv.DictReader(f))

pids = sorted({
    int(row["pid"])
    for row in rows
    if int(row["pid"]) > 0
})

fig, ax = plt.subplots(
    figsize=(12, 2.5 + len(pids) * 0.5)
)

colors = plt.get_cmap("tab20", max(1, len(pids)))

for row in rows:
    pid = int(row["pid"])
    start = int(row["start"])
    end = int(row["end"])
    duration = end - start

    if pid == -1:
        continue

    if pid == -2:
        ax.barh(
            0, duration,
            left=start,
            height=0.7,
            color="lightgray",
            edgecolor="black"
        )
        continue

    index = pids.index(pid)

    ax.barh(
        index + 1,
        duration,
        left=start,
        height=0.7,
        color=colors(index),
        edgecolor="black"
    )

ax.set_yticks(range(len(pids) + 1))
ax.set_yticklabels(
    ["CS"] + [f"P{pid}" for pid in pids]
)

ax.set_xlabel("Time (ticks)")
ax.set_ylabel("Processes")
ax.set_title(Path(input_file).stem)
ax.grid(axis="x", linestyle=":", alpha=0.5)
ax.set_axisbelow(True)

plt.tight_layout()
plt.savefig(output_file, dpi=160)
plt.close()

print("Saved:", output_file)
