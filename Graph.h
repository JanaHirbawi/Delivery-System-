#ifndef OS_PROJECT_GRAPH_H
#define OS_PROJECT_GRAPH_H

#include <sys/types.h>

typedef enum {
    STATUS_WAITING,
    STATUS_ENTERED,
    STATUS_LEAVING,
    STATUS_FINISHED
} TravelerStatus;

// Represents a single edge in the graph (part of adjacency list)
typedef struct Edge {
    int weight;
    int to;
    struct Edge* next;
} Edge;

// Represents the graph using adjacency list
typedef struct Graph {
    int numNodes;
    Edge** adjList;
} Graph;

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
    TravelerStatus status;
} TravelMessage;

#endif
