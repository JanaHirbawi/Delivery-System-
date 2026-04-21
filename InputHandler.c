#include <stdio.h>
#include <stdlib.h>
#include "Graph.h"

int loadGraphFromFile(const char *filename, Graph **graph, int *source, int *destination) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error: cannot open file\n");
        return 0;
    }

    int N, M;

    if (fscanf(file, "%d %d", &N, &M) != 2) {
        printf("Error: invalid file format\n");
        fclose(file);
        return 0;
    }

    if (N <= 0 || M < 0) {
        printf("Error: invalid graph size\n");
        fclose(file);
        return 0;
    }

    *graph = createGraph(N);
    if (*graph == NULL) {
        printf("Error: memory allocation failed\n");
        fclose(file);
        return 0;
    }

    for (int i = 0; i < M; i++) {
        int src, dst, weight;

        if (fscanf(file, "%d %d %d", &src, &dst, &weight) != 3) {
            printf("Error: invalid edge format\n");
            freeGraph(*graph);
            fclose(file);
            return 0;
        }

        if (src < 0 || src >= N || dst < 0 || dst >= N) {
            printf("Error: invalid node index\n");
            freeGraph(*graph);
            fclose(file);
            return 0;
        }

        if (weight < 0) {
            printf("Error: negative weights are not allowed\n");
            freeGraph(*graph);
            fclose(file);
            return 0;
        }

        addEdge(*graph, src, dst, weight);
    }

    if (fscanf(file, "%d %d", source, destination) != 2) {
        printf("Error: invalid source/destination format\n");
        freeGraph(*graph);
        fclose(file);
        return 0;
    }

    if (*source < 0 || *source >= N || *destination < 0 || *destination >= N) {
        printf("Error: invalid source or destination node\n");
        freeGraph(*graph);
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}
