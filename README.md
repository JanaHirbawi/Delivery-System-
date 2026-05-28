# 🚚 Delivery System Simulation

A C-based simulation project that models a smart delivery system using graph algorithms and real-time visualization with **Raylib**.

---

## 💡 Project Idea

This project simulates a delivery system where a delivery entity moves from a **source node** to a **destination node** through the **shortest path**.

- The road network is represented as a **directed weighted graph**
- The shortest path is calculated using **Dijkstra’s Algorithm**
- The movement is visualized using **animation** that reflects real travel timing based on edge weights

---

## 🚀 Execution Commands

| Milestone | Compilation        | Execution              |
|---------- |--------------------|------------------------|
| **M1**    | `make milestone1`  | `./dijkstra input.txt` |
| **M2**    | `make milestone2`  | `./sim input.txt`      |
| **M3**    | `make milestone3`  | `./sim input.txt`      |

* Use `make clean` to remove compiled files.

---

## 🛠️ Milestones Overview 

### Milestone 1: Core Logic
- *Graph: Implemented using an **Adjacency List* for efficient memory usage.
- *Dijkstra*: Computes the shortest path and total weight between source and destination.
- *Input*: Loads graph data and queries directly from a .txt file.

### Milestone 2: Static GUI
- *Visualization: Renders nodes and weighted edges using **Raylib*.
- *Clear Layout*: Uses directional arrows and labels to ensure graph readability.

### Milestone 3: Animation & Timing
- *Path Movement*: An entity follows the exact path calculated by Dijkstra.
- *Speed Logic: Movement is divided into W jumps (based on edge weight), with **300ms* per jump.
- *Node Waiting: Mandatory **1-second wait* at each intermediate node.
- *Controls: Interactive **Play/Stop* button and destination arrival notification.
### Milestone 4: Multiple Processes & Concurrent Travelers

* *Multi-Process System*: Introduced parent and child processes using `fork()`.

* *Parent Process Responsibilities*:

  * Reads the extended input file.
  * Computes the shortest path for each traveler using Dijkstra’s Algorithm.
  * Creates a child process for each traveler.
  * Manages the Raylib GUI and traveler animations.
  * Sends signals to terminate child processes after route completion.
  * Waits for all child processes before exiting.

* *Child Process Responsibilities*:

  * Prints its PID immediately after creation.
  * Remains alive while the traveler is moving.
  * Does not perform GUI rendering or path calculations.

* *Concurrent Movement*:

  * Multiple travelers move simultaneously on the graph.
  * Each traveler is displayed using a different color.

| Milestone | Compilation       | Execution         |
| --------- | ----------------- | ----------------- |
| **M4**    | `make milestone4` | `./sim input.txt` |

 ### Milestone 5: Autonomous Travelers & IPC Communication

* *Autonomous Child Processes*: Each child process independently computes its own shortest path using Dijkstra’s Algorithm.

* *Parent Process Responsibilities*:

  * Reads the graph and travelers from the input file.
  * Creates pipes for inter-process communication.
  * Creates a child process for each traveler using `fork()`.
  * Receives traveler updates through pipes.
  * Prints all traveler logs in the terminal.
  * Updates the Raylib GUI according to incoming messages.

* *Child Process Responsibilities*:

  * Computes its own Dijkstra path independently.
  * Sends updates to the parent after reaching each node.
  * Sends:
    * current node
    * next node
    * destination status
  * Does not print logs or render graphics.

* *IPC Mechanism*:

  * Communication between parent and children is implemented using `pipe()`.
  * Pipes were chosen because they provide a simple and efficient one-way communication mechanism between processes.

* *Real-Time GUI Updates*:

  * Each traveler is visualized independently.
  * The GUI updates traveler movement according to messages received from child processes.
  * Each traveler appears with a unique color.

| Milestone | Compilation       | Execution         |
| --------- | ----------------- | ----------------- |
| **M5**    | `make milestone5` | `./sim input.txt` |
