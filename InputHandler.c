#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Graph.h"

int loadGraphFromFile(const char *filename, Graph **graph, Traveler **travelers, int *travelerCount) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: cannot open file\n");
        return 0;
    }

    char line[256];
    int N = 0, M = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if (sscanf(line, "%d %d", &N, &M) == 2) break;
    }

    if (N <= 0 || M < 0) {
        printf("Error: invalid graph size\n");
        fclose(file);
        return 0;
    }

    *graph = createGraph(N);
    if (*graph == NULL) {
        fclose(file);
        return 0;
    }

    int edgesRead = 0;
    while (edgesRead < M && fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        int src, dst, weight;
        if (sscanf(line, "%d %d %d", &src, &dst, &weight) == 3) {
            if (src >= 0 && src < N && dst >= 0 && dst < N && weight >= 0) {
                addEdge(*graph, src, dst, weight);
                edgesRead++;
            } else {
                printf("Error: invalid edge boundaries\n");
                freeGraph(*graph);
                fclose(file);
                return 0;
            }
        }
    }

    int tCount = 0;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if (sscanf(line, "%d", &tCount) == 1) break;
    }

    if (tCount <= 0) {
        printf("Error: invalid traveler count\n");
        freeGraph(*graph);
        fclose(file);
        return 0;
    }

    *travelerCount = tCount;
    *travelers = (Traveler *)malloc(tCount * sizeof(Traveler));
    if (*travelers == NULL) {
        printf("Error: memory allocation failed for travelers\n");
        freeGraph(*graph);
        fclose(file);
        return 0;
    }

    int travelersRead = 0;
    while (travelersRead < tCount && fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        int src, dst;
        if (sscanf(line, "%d %d", &src, &dst) == 2) {
            (*travelers)[travelersRead].id = travelersRead;
            (*travelers)[travelersRead].src = src;
            (*travelers)[travelersRead].dst = dst;
            (*travelers)[travelersRead].pathLength = 0;
            (*travelers)[travelersRead].totalDistance = -1;
            
          
            (*travelers)[travelersRead].pid = -1; 
            memset((*travelers)[travelersRead].path, 0, sizeof((*travelers)[travelersRead].path)); // تصفير الأمان
            
            travelersRead++;
        }
    }

    fclose(file);
    return 1;
}
