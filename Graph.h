#ifndef OS_PROJECT_GRAPH_H
#define OS_PROJECT_GRAPH_H

#include <sys/types.h>

// Represents a single edge in the graph (part of adjacency list)
typedef struct Edge {
    int weight;              // weight (cost) of the edge
    int to;                  // destination node
    struct Edge* next;       // pointer to next edge
} Edge;

// Represents the graph using adjacency list
typedef struct Graph {
    int numNodes;            // total number of nodes
    Edge** adjList;          // adjacency list
} Graph;

/* ----------- API --------- */

Graph* createGraph(int nodes);

void addEdge(Graph* g, int src, int dst, int weight);

int getEdgeWeight(Graph *graph, int from, int to);

void freeGraph(Graph* g);

Edge* getNeighbors(Graph* g, int node);

void printGraph(Graph* g);

typedef struct {
    int id;
    int src;
    int dst;
    int path[100];
    int pathLength;
    int totalDistance;
    pid_t pid;
} Traveler;

typedef struct {
    pid_t pid;
    int travelerId;
    int currentNode;
    int nextNode;
    int isDestination;
    int isFinished;

    // =====================================================
    // EXAM CHANGE (Milestone 5)
    // Special IPC flag.
    // Child sets noPath = 1 when no route exists.
    // Parent checks this field and handles the message
    // differently from normal movement messages.
    // =====================================================
    int noPath;

} TravelMessage;

#endif
