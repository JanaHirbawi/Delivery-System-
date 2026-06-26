#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "Dijkstra.h"

static int findMinDistanceNode(int dist[], int visited[], int n) {
    int min = INT_MAX;
    int minIndex = -1;

    for (int i = 0; i < n; i++) {
        if (!visited[i] && dist[i] < min) {
            min = dist[i];
            minIndex = i;
        }
    }

    return minIndex;
}

static void buildPath(int parent[], int source, int destination,
                      int path[], int *pathLength) {
    int temp[100];
    int count = 0;
    int current = destination;

    while (current != -1) {
        temp[count++] = current;

        if (current == source) {
            break;
        }

        current = parent[current];
    }

    *pathLength = 0;

    for (int i = count - 1; i >= 0; i--) {
        path[*pathLength] = temp[i];
        (*pathLength)++;
    }
}

void dijkstra(Graph *graph, int source, int destination,
              int path[], int *pathLength, int *totalDistance) {
    *pathLength = 0;
    *totalDistance = -1;

    if (graph == NULL) {
        printf("No path found\n");
        return;
    }

    int n = graph->numNodes;

    if (source < 0 || source >= n || destination < 0 || destination >= n) {
       // printf("No path found\n");
        return;
    }

    if (source == destination) {
        path[0] = source;
        *pathLength = 1;
        *totalDistance = 0;

        printf("%d\n", source);
        printf("0\n");
        return;
    }

    int *dist = (int *)malloc(n * sizeof(int));
    int *visited = (int *)malloc(n * sizeof(int));
    int *parent = (int *)malloc(n * sizeof(int));

    if (dist == NULL || visited == NULL || parent == NULL) {
        printf("Error: memory allocation failed\n");
        free(dist);
        free(visited);
        free(parent);
        return;
    }

    for (int i = 0; i < n; i++) {
        dist[i] = INT_MAX;
        visited[i] = 0;
        parent[i] = -1;
    }

    dist[source] = 0;

    for (int count = 0; count < n - 1; count++) {
        int u = findMinDistanceNode(dist, visited, n);

        if (u == -1) {
            break;
        }

        visited[u] = 1;

        Edge *current = graph->adjList[u];

        while (current != NULL) {
            int v = current->to;
            int weight = current->weight;

            if (!visited[v] &&
                dist[u] != INT_MAX &&
                dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = u;
            }

            current = current->next;
        }
    }

    if (dist[destination] == INT_MAX) {
        printf("No path found\n");
    } else {
        buildPath(parent, source, destination, path, pathLength);
        *totalDistance = dist[destination];

    }

    free(dist);
    free(visited);
    free(parent);
}
