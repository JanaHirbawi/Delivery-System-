#include <stdio.h>
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"
#include "raylib.h"
#include "Visualizer.h"
#include "Animation.h"

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

    const int screenWidth = 650;
    const int screenHeight = 550;

    InitWindow(screenWidth, screenHeight, "Delivery System Visualizer");
    InitGraphLayout(screenWidth, screenHeight);
    SetTargetFPS(60);

    MovingEntity entity;
    InitEntity(&entity, GetNodePosition(path[0]));

bool isPlaying = false;
float finishTimer = 0.0f;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();


        Rectangle playButton = {screenWidth / 2 - 30, screenHeight - 80, 60, 60};


        if (CheckCollisionPointRec(GetMousePosition(), playButton) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

            isPlaying = !isPlaying;
            finishTimer = 0.0f;

            if (isPlaying && !entity.isFinished) {
                entity.status = ENTITY_MOVING;
            } else {
                entity.status = ENTITY_IDLE;
            }
        }


       if (isPlaying) {
    UpdateEntity(&entity, path, pathLength, deltaTime, graph);

    if (entity.isFinished) {
        finishTimer += deltaTime;

        if (finishTimer >= 2.0f) {
            isPlaying = false;
            finishTimer = 0.0f;
            InitEntity(&entity, GetNodePosition(path[0]));
        }
    }
}

        BeginDrawing();

        ClearBackground((Color){20, 25, 40, 255});

        drawEdges(graph);
        DrawStaticGraph();


        Vector2 center = {
            playButton.x + playButton.width / 2,
            playButton.y + playButton.height / 2
        };

        float radius = playButton.width / 2 - 5;

        DrawCircleLines(center.x, center.y, radius, RAYWHITE);

        if (!isPlaying) {
            // PLAY 
            Vector2 p1 = {center.x - 8, center.y - 12};
            Vector2 p2 = {center.x - 8, center.y + 12};
            Vector2 p3 = {center.x + 12, center.y};

            DrawTriangle(p1, p2, p3, RED);

        } else {
            // STOP 
            DrawRectangle(center.x - 10, center.y - 12, 6, 24, GREEN);
            DrawRectangle(center.x + 4, center.y - 12, 6, 24, GREEN);
        }

        DrawMovingEntity(entity);

        EndDrawing();
    }

    CloseWindow();
    freeGraph(graph);
    return 0;
}
