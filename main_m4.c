#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "raylib.h"
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"
#include "Visualizer.h"
#include "Animation.h"

int main() {
    Graph *graph = NULL;
    Traveler *travelers = NULL;
    int travelerCount = 0;

    if (!loadGraphFromFile("input.txt", &graph, &travelers, &travelerCount)) {
        return 1;
    }

    for (int i = 0; i < travelerCount; i++) {
        dijkstra(graph,
                 travelers[i].src,
                 travelers[i].dst,
                 travelers[i].path,
                 &travelers[i].pathLength,
                 &travelers[i].totalDistance);
    }

    for (int i = 0; i < travelerCount; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            return 1;
        }

        if (pid == 0) {
           printf("[%d] started\n", getpid()); 
           pause();
           exit(0);
        } else {
            travelers[i].pid = pid;
        }
    }

    const int screenWidth = 650;
    const int screenHeight = 550;

    InitWindow(screenWidth, screenHeight, "Delivery System Visualizer");
    InitGraphLayout(screenWidth, screenHeight);
    SetTargetFPS(60);

    MovingEntity entities[100];

    Color travelerColors[6] = {
        RED, BLUE, GREEN, ORANGE, PURPLE, MAROON
    };

    for (int i = 0; i < travelerCount; i++) {
        InitEntity(&entities[i], GetNodePosition(travelers[i].path[0]));
        entities[i].status = ENTITY_MOVING;
    }

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        BeginDrawing();

        DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
                               (Color){18, 24, 38, 255},
                               (Color){45, 62, 90, 255});

        drawEdges(graph);
        DrawStaticGraph();

        for (int i = 0; i < travelerCount; i++) {
            if (!entities[i].isFinished) {
                UpdateEntity(&entities[i],
                             travelers[i].path,
                             travelers[i].pathLength,
                             deltaTime,
                             graph);
            }

            DrawMovingEntity(entities[i], travelerColors[i % 6]);

           
            if (entities[i].isFinished && travelers[i].pid > 0) {
                printf("Traveler %d arrived. Terminating PID %d\n",
                       travelers[i].id,
                       travelers[i].pid);

                kill(travelers[i].pid, SIGTERM);
             
                travelers[i].pid = -1; 
            }
        }

        EndDrawing();
    }

   
    for (int i = 0; i < travelerCount; i++) {
        
      
        if (travelers[i].pid != -1) {
            kill(travelers[i].pid, SIGTERM);
        }
        
        waitpid(-1, NULL, 0); 
    }

    CloseWindow();

    free(travelers);
    freeGraph(graph);

    return 0;
}
