#ifndef OS_PROJECT_GRAPH_H
#define OS_PROJECT_GRAPH_H
#include <sys/types.h>
// Represents a single edge in the graph (part of adjacency list)
typedef struct Edge {
    int weight; // weight (cost) of the edge
    int to;     // destination node
    struct Edge* next; // pointer to next edge in the list
} Edge;

// Represents the graph using adjacency list
typedef struct Graph {
    int numNodes;  // total number of nodes in the graph
    Edge** adjList; // array of linked lists (each index = node)
} Graph;

/* ----------- API --------- */

// Creates a new graph with a given number of nodes
// Initializes adjacency list with NULL (no edges yet)
Graph* createGraph(int nodes);


// Inserts the directed weighted edge at the beginning of the adjacency list
void addEdge(Graph* g, int src, int dst, int weight);
int getEdgeWeight(Graph *graph, int from, int to);
//  free all allocated memory
void freeGraph(Graph* g);

// Returns the list of neighbors (edges) for a given node
Edge* getNeighbors(Graph* g, int node);

// Prints the graph structure (for debugging purposes)
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
    int noPath;
} TravelMessage;

#endif

