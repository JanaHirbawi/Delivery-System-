#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "raylib.h"
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"
#include "Visualizer.h"
#include "Animation.h"

#define MAX_TRAVELERS 100
#define PATH_SIZE 100

static void cleanup(Graph *graph, Traveler *travelers) {
    if (travelers != NULL) free(travelers);
    if (graph != NULL) freeGraph(graph);
}

static void sendTravelMessage(int writeFd, int travelerId, int currentNode,
                              int nextNode, int isDestination, int isFinished) {
    TravelMessage msg;

    msg.pid = getpid();
    msg.travelerId = travelerId;
    msg.currentNode = currentNode;
    msg.nextNode = nextNode;
    msg.isDestination = isDestination;
    msg.isFinished = isFinished;

    if (isFinished) {
        msg.status = STATUS_FINISHED;
    } else if (isDestination) {
        msg.status = STATUS_ENTERED;
    } else {
        msg.status = STATUS_LEAVING;
    }

    if (write(writeFd, &msg, sizeof(TravelMessage)) == -1) {
        perror("write");
    }
}

static void runChildProcess(Graph *graph, Traveler traveler, int writeFd) {
    int path[PATH_SIZE];
    int pathLength = 0;
    int totalDistance = 0;

    dijkstra(graph, traveler.src, traveler.dst, path, &pathLength, &totalDistance);

    for (int i = 0; i < pathLength; i++) {
        int isLastNode = (i == pathLength - 1);

        if (isLastNode) {
            sendTravelMessage(writeFd, traveler.id, path[i], -1, 1, 0);
            sleep(1);
            sendTravelMessage(writeFd, traveler.id, -1, -1, 1, 1);
        } else {
            sendTravelMessage(writeFd, traveler.id, path[i], path[i + 1], 0, 0);

         
         
            int weight = getEdgeWeight(graph, path[i], path[i + 1]);
if (weight <= 0) {
    weight = 1;
}

usleep((weight + 1) * 700000);
        }
    }

    close(writeFd);
    exit(0);
}

static int createChildProcesses(Graph *graph, Traveler *travelers, int travelerCount,
                                int pipes[MAX_TRAVELERS][2], pid_t pids[MAX_TRAVELERS]) {
    for (int i = 0; i < travelerCount; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return 0;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            return 0;
        }

        if (pid == 0) {
            close(pipes[i][0]);
            runChildProcess(graph, travelers[i], pipes[i][1]);
        }

        pids[i] = pid;
        travelers[i].pid = pid;

        close(pipes[i][1]);

        int flags = fcntl(pipes[i][0], F_GETFL, 0);
        if (flags == -1) {
            return 0;
        }

        if (fcntl(pipes[i][0], F_SETFL, flags | O_NONBLOCK) == -1) {
            return 0;
        }
    }

    return 1;
}

static void readMessagesFromChildren(int pipes[MAX_TRAVELERS][2],
                                     int travelerCount,
                                     int finished[MAX_TRAVELERS],
                                     int *finishedCount,
                                     TravelerEntity entities[MAX_TRAVELERS]) {
    for (int i = 0; i < travelerCount; i++) {
        if (finished[i]) {
            continue;
        }

        TravelMessage msg;
        ssize_t bytesRead = read(pipes[i][0], &msg, sizeof(TravelMessage));

        if (bytesRead == sizeof(TravelMessage)) {
            if (msg.status == STATUS_FINISHED || msg.isFinished) {
                printf("[PID=%d] finished\n", msg.pid);

                UpdateEntityFromMessage(entities, msg);

                finished[i] = 1;
                (*finishedCount)++;
                close(pipes[i][0]);
                continue;
            }

            if (msg.isDestination) {
                printf("[PID=%d] arrived at node %d | DESTINATION\n",
                       msg.pid, msg.currentNode);
            } else {
                printf("[PID=%d] arrived at node %d | next node: %d\n",
                       msg.pid, msg.currentNode, msg.nextNode);
            }

            UpdateEntityFromMessage(entities, msg);
        }
    }
}

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
        cleanup(graph, travelers);
        return 1;
    }

    int pipes[MAX_TRAVELERS][2];
    pid_t pids[MAX_TRAVELERS];
    int finished[MAX_TRAVELERS] = {0};

    if (!createChildProcesses(graph, travelers, travelerCount, pipes, pids)) {
        cleanup(graph, travelers);
        return 1;
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
            readMessagesFromChildren(pipes, travelerCount, finished, &finishedCount, entities);
        }

UpdateTravelerEntitiesM5(entities, travelerCount, deltaTime, graph);

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

    for (int i = 0; i < travelerCount; i++) {
        if (!finished[i]) {
            kill(pids[i], SIGTERM);
            close(pipes[i][0]);
        }

        waitpid(pids[i], NULL, 0);
    }

    CloseWindow();
    cleanup(graph, travelers);

    return 0;
}
