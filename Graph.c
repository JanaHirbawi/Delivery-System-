#include<stdio.h>
#include<stdlib.h>
#include "Graph.h"

Graph* createGraph(int nodes) {
    if (nodes <= 0) {
        printf("Invalid number of nodes\n");
        return NULL;
    }
    Graph* g=(Graph*)malloc(sizeof(Graph));
    if (g == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    g->numNodes=nodes;
    g->adjList=(Edge**)malloc(sizeof(Edge*)*nodes);
    if (g->adjList == NULL) {
        printf("Memory allocation failed\n");
        free(g);
        return NULL;
    }

    for (int i = 0; i < nodes ;i++)
        g->adjList[i]=NULL;

    return g;
}
void addEdge(Graph* g, int src, int dst, int weight) {
    if (g == NULL) {
        printf("Graph is NULL\n");
        return;
    }
    if (src < 0 || src >= g->numNodes || dst < 0 || dst >= g->numNodes) {
        printf("Invalid node index\n");
        return;
    }
    if (weight < 0) {
        printf("Invalid weight\n");
        return;
    }
    Edge* newEdge=(Edge*)malloc(sizeof(Edge));
    if ( newEdge == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    newEdge->to=dst;
    newEdge->weight=weight;
    newEdge->next=g->adjList[src];
    g->adjList[src]=newEdge;

}

Edge* getNeighbors(Graph* g, int node) {
    if (g == NULL || node < 0 || node >= g->numNodes) {
        return NULL;
    }
    return g->adjList[node];
}

void printGraph(Graph* g) {
    if (g == NULL) {
        printf("Graph is empty.\n");
        return;
    }
    printf("--- Current Graph Structure ---\n");
    for (int i = 0; i < g->numNodes; i++) {
        Edge* curr = g->adjList[i];
        printf("Node %d:", i);
        while (curr != NULL) {
            printf(" -> %d (weight: %d)", curr->to, curr->weight);
            curr = curr->next;
        }
        printf("\n");
    }
    printf("-------------------------------\n");
}
void freeGraph(Graph* g) {
    if (g == NULL) {
        return;
    }
    for ( int i=0 ; i<g->numNodes ; i++) {
        Edge* cur=g->adjList[i];
        while (cur!=NULL) {
            Edge* tmp=cur;
            cur=cur->next;
            free(tmp);
        }
    }
    free(g->adjList);
    free(g);
}
int getEdgeWeight(Graph *graph, int from, int to) {
    Edge *current = graph->adjList[from];

    while (current != NULL) {
        if (current->to == to) {
            return current->weight;
        }
        current = current->next;
    }

    return 1; // fallback
}
