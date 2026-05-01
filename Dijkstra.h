#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "Graph.h"

void dijkstra(Graph *graph, int source, int destination);
int GetPathArray(Graph *graph, int source, int destination, int path[]);

#endif
