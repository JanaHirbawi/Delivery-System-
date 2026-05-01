#include <stdio.h>
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"

#ifdef MILESTONE_2_GUI
#include "raylib.h"
#include "Visualizer.h"
#endif

int main() {
    Graph *graph = NULL;
    int source, destination;

    if (!loadGraphFromFile("input.txt", &graph, &source, &destination)) {
        return 1;
    }

#ifdef MILESTONE_2_GUI
    const int screenWidth = 650;
    const int screenHeight = 550;

    InitWindow(screenWidth, screenHeight, "Delivery System Visualizer");
    InitGraphLayout(screenWidth, screenHeight);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
                               (Color){18, 24, 38, 255},
                               (Color){45, 62, 90, 255});

        drawEdges(graph);
        DrawStaticGraph();

        Vector2 sourcePos = GetNodePosition(source);
        DrawEntityAtSource(sourcePos);

        EndDrawing();
    }

    CloseWindow();

#else
    dijkstra(graph, source, destination);
#endif

    freeGraph(graph);
    return 0;
}