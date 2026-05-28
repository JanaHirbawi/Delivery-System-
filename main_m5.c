#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "raylib.h"
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"
#include "Visualizer.h"
#include "Animation.h"

#define MAX_TRAVELERS 100
#define PATH_SIZE 100

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

    if (travelerCount > MAX_TRAVELERS) {
        printf("Too many travelers. Maximum is %d\n", MAX_TRAVELERS);
        free(travelers);
        freeGraph(graph);
        return 1;
    }

    int pipes[MAX_TRAVELERS][2];
    pid_t pids[MAX_TRAVELERS];
    int finished[MAX_TRAVELERS] = {0};

    for (int i = 0; i < travelerCount; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            free(travelers);
            freeGraph(graph);
            return 1;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            free(travelers);
            freeGraph(graph);
            return 1;
        }

        if (pid == 0) {
            close(pipes[i][0]);

            int path[PATH_SIZE];
            int pathLength = 0;
            int totalDistance = 0;

            dijkstra(
                graph,
                travelers[i].src,
                travelers[i].dst,
                path,
                &pathLength,
                &totalDistance
            );

            for (int j = 0; j < pathLength; j++) {
                TravelMessage msg;

                msg.pid = getpid();
                msg.travelerId = travelers[i].id;
                msg.currentNode = path[j];

                if (j == pathLength - 1) {
                    msg.nextNode = -1;
                    msg.isDestination = 1;
                    msg.isFinished = 1;
                } else {
                    msg.nextNode = path[j + 1];
                    msg.isDestination = 0;
                    msg.isFinished = 0;
                }

                write(pipes[i][1], &msg, sizeof(TravelMessage));
                usleep(1000000);
            }

            close(pipes[i][1]);
            free(travelers);
            freeGraph(graph);
            exit(0);
        }

        pids[i] = pid;
        travelers[i].pid = pid;

        close(pipes[i][1]);

        int flags = fcntl(pipes[i][0], F_GETFL, 0);
        fcntl(pipes[i][0], F_SETFL, flags | O_NONBLOCK);
    }

    const int screenWidth = 650;
    const int screenHeight = 550;

    InitWindow(screenWidth, screenHeight, "Delivery System Visualizer - Milestone 5");
    InitGraphLayout(screenWidth, screenHeight);
    SetTargetFPS(60);

    TravelerEntity entities[MAX_TRAVELERS];
    InitTravelerEntities(entities, travelerCount);

    int finishedCount = 0;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (finishedCount < travelerCount) {
            for (int i = 0; i < travelerCount; i++) {
                if (finished[i]) {
                    continue;
                }

                TravelMessage msg;
                ssize_t bytesRead = read(pipes[i][0], &msg, sizeof(TravelMessage));

                if (bytesRead == sizeof(TravelMessage)) {
                    if (msg.isDestination) {
                        printf("[PID=%d] arrived at node %d | DESTINATION\n",
                               msg.pid,
                               msg.currentNode);
                    } else {
                        printf("[PID=%d] arrived at node %d | next node: %d\n",
                               msg.pid,
                               msg.currentNode,
                               msg.nextNode);
                    }

                    UpdateEntityFromMessage(entities, msg);

                    if (msg.isFinished) {
                        printf("[PID=%d] finished\n", msg.pid);
                        finished[i] = 1;
                        finishedCount++;
                        close(pipes[i][0]);
                    }
                }
            }
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

        if (finishedCount == travelerCount) {
            DrawText("All travelers finished", 180, 20, 25, GREEN);
        }

        EndDrawing();
    }

    for (int i = 0; i < travelerCount; i++) {
        waitpid(pids[i], NULL, 0);
    }

    CloseWindow();

    free(travelers);
    freeGraph(graph);

    return 0;
}
