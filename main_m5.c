#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "Graph.h"
#include "InputHandler.h"
#include "Visualizer.h"
#include "Animation.h"

#define MAX_TRAVELERS 100

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    Graph *graph = NULL;
    Traveler *travelers = NULL;
    int travelerCount = 0;

    if (!loadGraphFromFile(argv[1], &graph, &travelers, &travelerCount)) {
        printf("Failed to load graph from file.\n");
        return 1;
    }

    const int screenWidth = 650;
    const int screenHeight = 550;

    InitWindow(screenWidth, screenHeight, "Delivery System Visualizer - Milestone 5");
    InitGraphLayout(screenWidth, screenHeight);
    SetTargetFPS(60);

    TravelerEntity entities[MAX_TRAVELERS];
    InitTravelerEntities(entities, travelerCount);

    int currentTraveler = 0;
    float messageTimer = 0.0f;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        messageTimer += deltaTime;

        if (messageTimer >= 1.0f && currentTraveler < travelerCount) {
            TravelMessage msg;

            msg.pid = 0;
            msg.travelerId = travelers[currentTraveler].id;
            msg.currentNode = travelers[currentTraveler].src;
            msg.nextNode = travelers[currentTraveler].dst;
            msg.isDestination = 0;
            msg.isFinished = 0;

            UpdateEntityFromMessage(entities, msg);

            currentTraveler++;
            messageTimer = 0.0f;
        }

        UpdateTravelerEntities(entities, travelerCount, deltaTime);

       BeginDrawing();

DrawRectangleGradientV(
    0, 0,
    screenWidth, screenHeight,
    (Color){18, 24, 38, 255},
    (Color){45, 62, 90, 255}
);

drawEdges(graph);
DrawStaticGraph();
DrawTravelerEntities(entities, travelerCount);

EndDrawing();
    }

    CloseWindow();

    free(travelers);
    freeGraph(graph);

    return 0;
}
