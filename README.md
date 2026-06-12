# 🚚 Delivery System Simulation

A C-based simulation project that models a smart delivery system using graph algorithms and real-time visualization with **Raylib**.

---

## 💡 Project Idea

This project simulates a delivery system where a delivery entity moves from a **source node** to a **destination node** through the **shortest path**.

- The road network is represented as a **directed weighted graph**.
- The shortest path is calculated using **Dijkstra’s Algorithm**.
- The movement is visualized using **animation** that reflects real travel timing based on edge weights.

---

## 🚀 Execution Commands

| Milestone | Compilation        | Execution             |
|---------- |--------------------|-----------------------|
| **M1** | `make milestone1`  | `./dijkstra input.txt`|
| **M2** | `make milestone2`  | `./sim input.txt`     |
| **M3** | `make milestone3`  | `./sim input.txt`     |
| **M4** | `make milestone4`  | `./sim input.txt`     |
| **M5** | `make milestone5`  | `./sim input.txt`     |
| **M6** | `make milestone6`  | `./sim input.txt`     |

* Use `make clean` to remove compiled files.

---

## 🛠️ Milestones Overview 

### Milestone 1: Core Logic
- **Graph:** Implemented using an *Adjacency List* for efficient memory usage.
- **Dijkstra:** Computes the shortest path and total weight between source and destination.
- **Input:** Loads graph data and queries directly from a `.txt` file.

### Milestone 2: Static GUI
- **Visualization:** Renders nodes and weighted edges using **Raylib**.
- **Clear Layout:** Uses directional arrows and labels to ensure graph readability.

### Milestone 3: Animation & Timing
- **Path Movement:** An entity follows the exact path calculated by Dijkstra.
- **Speed Logic:** Movement is divided into $W$ jumps (based on edge weight), with **300ms** per jump.
- **Node Waiting:** Mandatory **1-second wait** at each intermediate node.
- **Controls:** Interactive **Play/Stop** button and destination arrival notification.

### Milestone 4: Multiple Processes & Concurrent Travelers
- **Multi-Process System:** Introduced parent and child processes using `fork()`.
- **Parent Process Responsibilities:**
  - Reads the extended input file and computes paths using Dijkstra’s Algorithm.
  - Creates a child process for each traveler and manages the Raylib GUI animations.
  - Sends signals to terminate child processes after route completion and waits for them before exiting.
- **Child Process Responsibilities:**
  - Prints its PID immediately after creation and remains alive while moving.
  - Does not perform GUI rendering or path calculations.
- **Concurrent Movement:** Multiple travelers move simultaneously on the graph, each displayed using a unique color.

### Milestone 5: Autonomous Travelers & IPC Communication
- **Autonomous Model:** Each child process independently computes its own shortest path using Dijkstra's Algorithm and dynamically reports its tracking status back to the parent.
- **Parent Role:** Reads the input file, creates the pipes, forks child processes, reads their real-time updates to refresh the Raylib GUI, and prints synchronized logs to the terminal.
- **IPC Mechanism (Why We Chose Pipes):**
  1. *Unidirectional Flow:* Communication is strictly one-way (Multiple Child processes writing updates to a single Parent coordinator). Pipes are the most lightweight tool for this parent-child hierarchy.
  2. *Simplicity over Shared Memory:* Pipes avoid complex synchronization issues, race conditions, and the need for global access controls required by Shared Memory.

### Milestone 6: Advanced OS Resource Management & Synchronization
- **Overview:** This final milestone simulates a real-world synchronized delivery or traffic network. Each traveler is a standalone living entity (`Child Process`) with its own lifecycle, driving its path autonomously. The main challenge solved here is **Resource Management**: ensuring that multiple independent processes navigate through the same shared graph infrastructure in real-time without collisions or software crashes.
- **Synchronization Mechanism (POSIX Semaphores):**
  - **Chosen Mechanism:** POSIX Semaphores (`sem_t`) mapped per graph node.
  - **Why this choice?** Graph nodes represent shared critical resources (hubs) that cannot fit overlapping travelers.
- **How it works:**
  1. A traveler process must execute `sem_wait()` before entering a node. If occupied, the OS blocks the process, and the GUI renders it in a circular queue around the node (`STATUS_WAITING`).
  2. To avoid visual overlapping on edges, the traveler holds the semaphore lock **while traveling on the edge** (`usleep`).
  3. Only upon reaching the next threshold does it execute `sem_post()` to wake up the next waiting process safely, preventing any Race Conditions or graphical glitches.
