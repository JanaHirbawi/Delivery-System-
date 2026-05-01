#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "Graph.h"

void dijkstra(Graph *graph, int source, int destination,
              int path[], int *pathLength, int *totalDistance);

#endif
