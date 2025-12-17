![image](https://github.com/user-attachments/assets/a8168ae2-be05-4bda-8307-61907e5147cb)



# GridMan

GridMan is a **private, lightweight distributed computing grid** designed to reuse
**idle consumer-grade hardware** for real computation.

It was built to be **practical, fast to deploy, and dependency-free** —
not to be a cloud platform, not to be secure-by-design, and not to solve
problems at mass scale.

GridMan exists to **make unused devices compute** in the easiest possible way,
no matter their hardware architecture. As long as they execute code, they can
accept jobs.

---

## Core Idea

> If a device can execute code, it can be a GridMan worker.

GridMan turns:
- devices sitting in drawers,
- machines idling most of the day,
- consumer electronics never meant for clusters

into a **personal compute grid**.

---

## Supported Hardware

GridMan intentionally targets **consumer-grade electronics**:

- **ESP32** (CPU, ELF task execution)
- **Raspberry Pi / ARM boards**
- **Any PC** (x86 / ARM)
- **iOS devices** (shader-based GPGPU via Metal backend)
- **Android devices** (userland execution)

The system does not require:
- datacenter hardware
- GPUs with enterprise drivers
- cloud infrastructure
- persistent connectivity

---

## Mobile & GPU Compute

GridMan supports **shader-driven GPGPU execution**.

- Compute logic is expressed as **GPU shaders**
- On iOS, shaders are executed using **Metal as a backend**
  
This approach keeps GPU workers:
- portable by design 
- compatible with OpenGL ES–class hardware 

---

## Private Grid by Design

GridMan is designed to run as a **private grid**. 

GridMan **does not focus on security**.

This is intentional.
Private does not imply hardened security — it implies control over participants.

- No sandboxing beyond what the OS already provides
- No encryption layers beyond transport needs
- No multi-tenant isolation
- No untrusted public workloads

GridMan assumes:
- trusted devices
- trusted tasks
- trusted network

The goal is **fast task deployment and execution** in controlled & trusted environment - not hardened isolation.

---

## What GridMan Optimizes For

GridMan is optimized for **practical distributed execution** across heterogeneous devices.

Specifically, it focuses on:

- **Heterogeneous worker orchestration**
  - CPUs and GPUs
  - x86, ARM, embedded MCUs
  - desktop, mobile, and constrained devices

- **Flexible task execution models**
  - native binaries per architecture
  - dynamic ELF loading on embedded devices
  - shader-based GPGPU workloads on mobile GPUs

- **Centralized task coordination**
  - single server acting as scheduler and dispatcher
  - explicit task lifecycle management
  - deterministic task assignment and execution

- **Low-overhead distributed execution**
  - minimal runtime abstraction
  - direct execution without heavy frameworks
  - fast task dispatch and result collection

- **Reuse of idle consumer hardware**
  - devices not designed for clusters
  - opportunistic computation on idle nodes
  - no assumption of constant availability

---

## What GridMan Does *Not* Try to Be

- Not a cloud platform
- Not a secure execution sandbox
- Not a Kubernetes alternative
- Not an enterprise product
- Not a research paper

GridMan is a **tool**, not a framework ecosystem. 

---

## Architecture Overview
Central server->Workers connect->receive tasks->execute them->and return results.

No worker is special.

---

## Why This Exists

GridMan was written **entirely by a single author** to explore
how much useful computation can be extracted from:

- idle devices
- discarded hardware
- consumer electronics
- minimal assumptions

It prioritizes **function over form** and **execution over abstraction**,
**Simplicity over complexity** and **fun over hard work**.

---

## Status

- Core server: stable
- CPU workers: stable
- ESP32 workers: stable
- Mobile GPU workers: stable
- Continuous evolution
 
