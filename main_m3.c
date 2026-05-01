#include <stdio.h>
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"

#ifdef MILESTONE_2_GUI
#include "raylib.h"
#include "Visualizer.h"
#include "Animation.h"
#endif

int main() {
    Graph *graph = NULL;
    int source, destination;

    if (!loadGraphFromFile("input.txt", &graph, &source, &destination)) {
        return 1;
    }

    int path[100];
    int pathLength = 0;
    int totalDistance = 0;

    dijkstra(graph, source, destination, path, &pathLength, &totalDistance);

#ifdef MILESTONE_2_GUI
    SetAnimationPath(path, pathLength);

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

        int currentNode = GetCurrentNode();
        if (currentNode != -1) {
            Vector2 pos = GetNodePosition(currentNode);
            DrawEntityAtSource(pos);
        }

        EndDrawing();
    }

    CloseWindow();
#else
    printf("Shortest Path:\n");
    for (int i = 0; i < pathLength; i++) {
        printf("%d ", path[i]);
    }
    printf("\nTotal Distance: %d\n", totalDistance);
#endif

    freeGraph(graph);
    return 0;
}
