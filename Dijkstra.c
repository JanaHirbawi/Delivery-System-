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

static void printPath(int parent[], int node) {
    if (parent[node] == -1) {
        printf("%d", node);
        return;
    }

    printPath(parent, parent[node]);
    printf(" -> %d", node);
}

void dijkstra(Graph *graph, int source, int destination) {
    if (graph == NULL) {
        printf("No path found\n");
        return;
    }

    int n = graph->numNodes;

    if (source < 0 || source >= n || destination < 0 || destination >= n) {
        printf("No path found\n");
        return;
    }

    if (source == destination) {
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

        Edge *current = getNeighbors(graph, u);
        while (current != NULL) {
            int v = current->to;
            int weight = current->weight;

            if (!visited[v] && dist[u] != INT_MAX && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = u;
            }

            current = current->next;
        }
    }

    if (dist[destination] == INT_MAX) {
        printf("No path found\n");
    } else {
        printPath(parent, destination);
        printf("\n%d\n", dist[destination]);
    }

    free(dist);
    free(visited);
    free(parent);
}
