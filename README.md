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
