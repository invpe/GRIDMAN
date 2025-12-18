import subprocess
import time
import struct
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ThreadPoolExecutor, as_completed

# ---------------- CONFIG ----------------
SERVER = "server_ip_address"
TASK = "gpgpu_test5"

TILE_W = 32
TILE_H = 32
BATCH = TILE_W * TILE_H

TOTAL_TASKS = 32
POLL_DELAY = 0.1

GRID_COLS = 8
GRID_ROWS = (TOTAL_TASKS + GRID_COLS - 1) // GRID_COLS

SUBMIT_WORKERS = 4    
# ----------------------------------------

print("[INIT ] Prime GPGPU BULK submit + live observe (2D)")
print(f"[CONF ] server={SERVER} tasks={TOTAL_TASKS} grid={GRID_ROWS}x{GRID_COLS}")

# ---------- MATPLOTLIB SETUP ----------
fig, ax = plt.subplots()
fig.set_size_inches(10, 6)

canvas = np.zeros(
    (GRID_ROWS * TILE_H, GRID_COLS * TILE_W),
    dtype=np.uint8
)

img = ax.imshow(
    canvas,
    cmap="gray",
    vmin=0,
    vmax=255,
    interpolation="nearest"
)

ax.set_title("Prime GPGPU – bulk submit, live grid")
ax.axis("off")

plt.ion()
plt.show()
# --------------------------------------


def submit(tile_idx):
    start = tile_idx * BATCH
    payload = f"""@fbo,32,32;
uint,start,{start};
uint,width,32;"""

    out = subprocess.check_output(
        ["./gmlibtest", SERVER, "addtask", TASK, payload, "", ""],
        text=True
    )

    task_id = int(out.strip().split()[-1])
    print(f"[SUBMIT] task={task_id} tile={tile_idx} start={start}")
    return task_id, tile_idx


def get_task(task_id):
    return subprocess.check_output(
        ["./gmlibtest", SERVER, "gettask", str(task_id)],
        text=True
    )


# ---------- BULK PARALLEL SUBMIT ----------
task_map = {}  # task_id -> tile_idx

print("[PHASE] Submitting all tasks in parallel...")

with ThreadPoolExecutor(max_workers=SUBMIT_WORKERS) as executor:
    futures = [executor.submit(submit, i) for i in range(TOTAL_TASKS)]

    for fut in as_completed(futures):
        tid, tile_idx = fut.result()
        task_map[tid] = tile_idx

print(f"[PHASE] All tasks submitted: {len(task_map)}")
print("[PHASE] Observing execution...\n")

# ---------- LIVE OBSERVE + RENDER ----------
completed = set()

while len(completed) < TOTAL_TASKS:
    for tid, tile_idx in task_map.items():
        if tid in completed:
            continue

        out = get_task(tid)

        if "State 2" in out:
            print(f"[RUN ] task={tid}")

        elif "State 3" in out:
            print(f"[FAIL] task={tid}")
            completed.add(tid)

        elif "State 4" in out:
            hexdata = out.split("Outputpayload")[1].strip()
            raw = bytes.fromhex(hexdata)

            tile = np.zeros((TILE_H, TILE_W), dtype=np.uint8)
            prime_count = 0

            for i in range(0, len(raw), 16):
                r, g, b, a = struct.unpack("<4I", raw[i:i+16])
                y = (i // 16) // TILE_W
                x = (i // 16) % TILE_W

                if g == 1:
                    tile[y, x] = 255
                    prime_count += 1

            row = tile_idx // GRID_COLS
            col = tile_idx % GRID_COLS

            y0 = row * TILE_H
            y1 = y0 + TILE_H
            x0 = col * TILE_W
            x1 = x0 + TILE_W

            canvas[y0:y1, x0:x1] = tile
            img.set_data(canvas)
            plt.draw()
            plt.pause(0.01)

            print(
                f"[DONE ] task={tid} tile={tile_idx} "
                f"grid=({row},{col}) "
                f"range={tile_idx*BATCH}-{(tile_idx+1)*BATCH-1} "
                f"primes={prime_count}"
            )

            completed.add(tid)

    #time.sleep(POLL_DELAY)

# ---------- FINISH ----------
plt.ioff()
plt.savefig("prime_live_bulk_parallel.png")
print("\n[OK   ] Saved prime_live_bulk_parallel.png")
plt.show()
