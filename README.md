# OS Thread Synchronization — Dynamic Slot Management

[![C](https://img.shields.io/badge/C-99-blue)](https://en.wikipedia.org/wiki/C99)
[![pthreads](https://img.shields.io/badge/pthreads-POSIX-orange)](https://man7.org/linux/man-pages/man7/pthreads.7.html)
[![Course](https://img.shields.io/badge/course-SOF201%20Operating%20System-blueviolet)]()

> Extended producer-consumer problem with **dynamic buffer resizing** — 5 robots produce items, 2 workers consume them. Buffer expands/shrinks based on occupancy. Synchronized with **semaphores + mutex**.

---

## 🧠 Problem Statement

In an automated factory:

- **5 robots** (producers) continuously generate items with random IDs
- **2 workers** (consumers) take items for further processing
- Robots are faster than workers — the shared buffer **dynamically expands and shrinks**:
  - **≥90% full** → expand by 2 slots (max 15)
  - **≤50% full** → shrink to half (min 5)
- Robots **slow down** after high-speed operation, allowing workers to catch up and shrink the buffer

---

## 🔧 Synchronization Mechanism

| Mechanism | Purpose |
|-----------|---------|
| `sem_t empty` | Counts empty slots — robots wait if buffer is full |
| `sem_t full` | Counts filled slots — workers wait if buffer is empty |
| `pthread_mutex_t` | Protects critical section during slot read/write/resize |
| `rCount` / `wCount` | Circular pointers for robot/worker position in slot |

### Resize Safety
When the slot resizes:
1. Existing items are copied to new memory
2. Old semaphores are **destroyed** and reinitialized with new bounds
3. Pointers are reset: `rCount` → first empty, `wCount` → first item

---

## 📊 Output Comparison

| Without Sync | With Sync |
|-------------|-----------|
| Race conditions — robots and workers overwrite each other | Clean sequential access |
| Slot state inconsistent | Slot state always valid |
| Workers may take from slots being written to | Mutex guarantees mutual exclusion |

---

## 🚀 Compile & Run

```bash
gcc dynamic_slot_sync.c -o dynamic_slot_sync -lpthread
./dynamic_slot_sync
```

> Requires POSIX threads (`-lpthread`). Works on Linux, macOS, WSL, or MinGW.

---

## 📁 Files

| File | Description |
|------|-------------|
| `dynamic_slot_sync.c` | Full implementation with dynamic buffer resizing |
| `README.md` | This file |

---

## 📝 Context

- **Course**: SOF201 Operating System, Xiamen University Malaysia
- **Semester**: 2024/04
- **Lecturer**: Venantius Kumar Sevamalai
- **Author**: Katherine Tan (SWE2304438)

---

## 👤 Author

**Katherine** — [@katherine101001](https://github.com/katherine101001)
