#include <stdio.h>
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"

int main(int argc, char *argv[]) {
    Graph *graph = NULL;
    int source, destination;

     if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    if (!loadGraphFromFile(argv[1], &graph, &source, &destination)) {
        return 1;
    }

    dijkstra(graph, source, destination);

    freeGraph(graph);
    return 0;
}
