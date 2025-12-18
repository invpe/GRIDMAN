# GPGPU Prime Heatmap Demo (GridMan task)

This project demonstrates a **fully generic GPGPU task execution pipeline on iOS**, powered by **OpenGL ES fragment shaders** and orchestrated by the **GridMan**.

The demo computes **prime numbers** over large numeric ranges using **GPU on iPhone**, distributes the workload via GridMan, and visualizes the results **live** as a 2D heatmap.

---

## 🔥 What This Demonstrates

- ✅ Generic GPGPU execution on iOS  
- ✅ Runtime shader delivery from GridMan
- ✅ Self-describing payloads (no recompilation required)
- ✅ Parallel task execution across devices
- ✅ Live visualization of GPU-computed results
- ✅ iPhone used as a real compute node (not a toy demo)

This is **not a proof-of-concept** — it is a working distributed GPGPU system.

---


## 🧩 The GPGPU Task (Prime Heatmap)

Each GPU task:

- Processes a **32×32 tile (1024 numbers)**
- Tests each number for primality
- Writes results into an FBO:
  - **R** → number
  - **G** → `1` if prime, `0` otherwise
  - **B** → auxiliary/debug
  - **A** → marker

The host combines many such tiles into a large 2D grid.

---

## 🎛 Payload Format  

Example:

```
@fbo,32,32; < framebuffer size
uint,start,0;  < start search space index
uint,width,32; < end search space index
```
 
