#include <stdio.h>
#include "raylib.h"
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"
#include "Visualizer.h"

int main() {
    Graph *graph = NULL;
    int source, destination;

    if (!loadGraphFromFile("input.txt", &graph, &source, &destination)) {
        return 1;
    }

    const int screenWidth = 650;
    const int screenHeight = 550;

    InitWindow(screenWidth, screenHeight, "Delivery System Visualizer");
    InitGraphLayout(screenWidth, screenHeight);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        DrawRectangleGradientV(
            0, 0,
            screenWidth, screenHeight,
            (Color){18, 24, 38, 255},
            (Color){45, 62, 90, 255}
        );

        drawEdges(graph);
        DrawStaticGraph();

        EndDrawing();
    }

    CloseWindow();
    freeGraph(graph);

    return 0;
}
