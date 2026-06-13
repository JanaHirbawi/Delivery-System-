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
3. Real-Time Updates:
   Each child process sends node arrival updates through its dedicated pipe, while the parent process receives the messages, updates the GUI, and prints execution logs.

## Milestone 6 – Node Synchronization
### Synchronization Mechanism

In this milestone we added synchronization between traveler processes to ensure that no more than one traveler can stay inside the same node at the same time.

We used POSIX named semaphores (`sem_open`, `sem_wait`, `sem_post`) and created one semaphore for each node in the graph.

Each traveler process performs the following steps:

1. Arrives at a node and sends a WAITING message to the parent process.
2. Calls `sem_wait()` on the semaphore of that node.
3. Enters the node and sends an ENTERED message.
4. Remains inside the node for one full second (critical section).
5. Sends a LEAVING message.
6. Releases the node using `sem_post()`.
7. Continues to the next node.

This guarantees mutual exclusion and prevents multiple travelers from occupying the same node simultaneously.

### IPC Mechanism

Communication between child processes and the parent process is implemented using Pipes.
Each child process sends status messages to the parent process:

* WAITING
* ENTERED
* LEAVING
* FINISHED

The parent process receives these messages, updates the GUI, and prints the execution log to the terminal.

### GUI Visualization

Travelers waiting for a node are displayed with a WAIT indicator near the node.
Travelers inside or moving between nodes are displayed using their assigned colors.

### Correctness

* At most one traveler can be inside a node at any time.
* Waiting travelers remain outside the node until access is granted.
* Every waiting traveler eventually enters the node (no starvation).
* The GUI reflects waiting and movement states during execution.

