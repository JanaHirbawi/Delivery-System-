#include <stdio.h>
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"

int main() {
    Graph *graph = NULL;
    int source, destination;

    if (!loadGraphFromFile("input.txt", &graph, &source, &destination)) {
        return 1;
    }

    dijkstra(graph, source, destination);

    freeGraph(graph);
    return 0;
}
