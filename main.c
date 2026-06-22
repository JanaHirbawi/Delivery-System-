#include <stdio.h>
#include "raylib.h"
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"
#include "Visualizer.h"

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
