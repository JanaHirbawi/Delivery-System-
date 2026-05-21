#include <stdio.h>
#include <stdlib.h>
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"

int main() {
    Graph *graph = NULL;
    Traveler *travelers = NULL;
    int travelerCount = 0;

    
    if (!loadGraphFromFile("input.txt", &graph, &travelers, &travelerCount)) {
        return 1;
    }

    printf("=== Milestone 4: Core Logic Check ===\n");
    printf("Successfully loaded %d travelers from file.\n\n", travelerCount);

    for (int i = 0; i < travelerCount; i++) {
        printf("[Traveler ID: %d] Calculating shortest path from %d to %d:\n", travelers[i].id, travelers[i].src, travelers[i].dst);
        
        dijkstra(graph, travelers[i].src, travelers[i].dst, 
                 travelers[i].path, &travelers[i].pathLength, &travelers[i].totalDistance);
        
        printf("Total Distance: %d\n", travelers[i].totalDistance);
        printf("-----------------------------------------------\n\n");
    }

  
    free(travelers);
    freeGraph(graph);
    
    return 0;
}
