#include <stdio.h>
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"

// Conditional inclusion for Milestone 2
#ifdef MILESTONE_2_GUI
#include "raylib.h"
#include "Visualizer.h"
#endif

int main() {
    Graph *graph = NULL;
    int source, destination;

    // Common logic for both milestones
    if (!loadGraphFromFile("input.txt", &graph, &source, &destination)) {
        return 1;
    }

#ifdef MILESTONE_2_GUI
    // --- Milestone 2: Graphical User Interface ---
    const int screenWidth = 650;
    const int screenHeight = 550;

    InitWindow(screenWidth, screenHeight, "Delivery System Visualizer");
    InitGraphLayout(screenWidth, screenHeight);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        // Background Gradient
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight, 
                               (Color){18, 24, 38, 255}, 
                               (Color){45, 62, 90, 255});
        
        drawEdges(graph);
        DrawStaticGraph();
        
        EndDrawing();
    }
    CloseWindow();

#else
    // --- Milestone 1: Terminal Output (Dijkstra) ---
    dijkstra(graph, source, destination);
#endif

    freeGraph(graph);
    return 0;
}
