#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "raylib.h"
#include "Graph.h"

#define NODE_COUNT 6
#define NODE_RADIUS 32

typedef struct {
    int id;
    Vector2 position;
    const char *name;
    Color color;
} NodeVisual;

extern NodeVisual nodesLayout[NODE_COUNT];

void InitGraphLayout(int screenWidth, int screenHeight);
void drawEdges(Graph *graph);
void DrawStaticGraph(void);
Vector2 GetNodePosition(int nodeIndex);
#endif
