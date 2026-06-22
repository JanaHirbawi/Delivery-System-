#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/stat.h>

#include "raylib.h"
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"
#include "Visualizer.h"
#include "Animation.h"

#define MAX_TRAVELERS 100
#define PATH_SIZE 100

sem_t *node_semaphores[MAX_TRAVELERS];

static void cleanup(Graph *graph, Traveler *travelers) {
    if (travelers != NULL) free(travelers);
    if (graph != NULL) freeGraph(graph);
}

void init_semaphores(int numNodes) {
    char sem_name[32];

    for (int i = 0; i < numNodes; i++) {
        sprintf(sem_name, "/sem_node_%d", i);

        sem_unlink(sem_name);

        node_semaphores[i] = sem_open(sem_name, O_CREAT | O_EXCL, 0644, 1);
        if (node_semaphores[i] == SEM_FAILED) {
            perror("Error: sem_open failed");
            exit(1);
        }
    }
}

void cleanup_semaphores(int numNodes) {
    char sem_name[32];

    for (int i = 0; i < numNodes; i++) {
        sprintf(sem_name, "/sem_node_%d", i);
        sem_close(node_semaphores[i]);
        sem_unlink(sem_name);
    }
}

static void sendTravelMessage(int writeFd, int travelerId, int currentNode,
                              int nextNode, int isDestination, int isFinished,
                              TravelerStatus status) {
    TravelMessage msg;

    msg.pid = getpid();
    msg.travelerId = travelerId;
    msg.currentNode = currentNode;
    msg.nextNode = nextNode;
    msg.isDestination = isDestination;
    msg.isFinished = isFinished;
    msg.status = status;

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
        int currNode = path[i];
        int nextNode = (i < pathLength - 1) ? path[i + 1] : -1;
        int isDest = (i == pathLength - 1);

        char sem_name[32];
        sprintf(sem_name, "/sem_node_%d", currNode);
        sem_t *sem = sem_open(sem_name, 0);

        if (sem == SEM_FAILED) {
            perror("sem_open in child");
            exit(1);
        }

        sendTravelMessage(writeFd, traveler.id, currNode, nextNode,
                          isDest, 0, STATUS_WAITING);

        sem_wait(sem);

        sendTravelMessage(writeFd, traveler.id, currNode, nextNode,
                          isDest, 0, STATUS_ENTERED);

        sleep(1);

        sendTravelMessage(writeFd, traveler.id, currNode, nextNode,
                  isDest, 0, STATUS_LEAVING);

/* Give the GUI time to draw the traveler outside the node */

usleep(700000);
sem_post(sem);


        sem_close(sem);

        if (nextNode != -1) {
            int weight = getEdgeWeight(graph, currNode, nextNode);
            usleep(weight * 1000000);
        }
    }

    sendTravelMessage(writeFd, traveler.id, -1, -1,
                      1, 1, STATUS_FINISHED);

    exit(0);
}
static int readMessagesFromChildren(int pipes[][2], int travelerCount, int finished[],
                                    int *finishedCount, TravelerEntity entities[]) {
    int activity = 0;

    for (int i = 0; i < travelerCount; i++) {
        if (finished[i]) continue;

        int flags = fcntl(pipes[i][0], F_GETFL, 0);
        fcntl(pipes[i][0], F_SETFL, flags | O_NONBLOCK);

        TravelMessage msg;
        ssize_t bytesRead = read(pipes[i][0], &msg, sizeof(TravelMessage));

        if (bytesRead == sizeof(TravelMessage)) {
            activity = 1;

            if (msg.status == STATUS_WAITING) {
                printf("[PARENT] Traveler %d WAITING for node %d\n",
                       msg.travelerId, msg.currentNode);
            } else if (msg.status == STATUS_ENTERED) {
                printf("[PARENT] Traveler %d ENTERED node %d\n",
                       msg.travelerId, msg.currentNode);
            } else if (msg.status == STATUS_LEAVING) {
                printf("[PARENT] Traveler %d LEAVING node %d\n",
                       msg.travelerId, msg.currentNode);
            } else if (msg.status == STATUS_FINISHED) {
                printf("[PARENT] Traveler %d FINISHED\n",
                       msg.travelerId);
            }

            fflush(stdout);

            if (msg.isFinished) {
                finished[i] = 1;
                (*finishedCount)++;
                entities[i].isFinished = true;
                entities[i].currentNode = -1;
            } else {
                UpdateEntityFromMessage(entities, msg);
            }
        }
    }

    return activity;
}

int main(int argc, char *argv[]) {
    Graph *graph = NULL;
    Traveler *travelers = NULL;
    int travelerCount = 0;

    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    if (!loadGraphFromFile(argv[1], &graph, &travelers, &travelerCount)) {
        return 1;
    }

    init_semaphores(graph->numNodes);

    int pipes[MAX_TRAVELERS][2];
    pid_t pids[MAX_TRAVELERS];
    int finished[MAX_TRAVELERS] = {0};

    for (int i = 0; i < travelerCount; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            cleanup(graph, travelers);
            cleanup_semaphores(graph->numNodes);
            return 1;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            cleanup(graph, travelers);
            cleanup_semaphores(graph->numNodes);
            return 1;
        }

        if (pid == 0) {
            close(pipes[i][0]);
            runChildProcess(graph, travelers[i], pipes[i][1]);
        } else {
            close(pipes[i][1]);
            pids[i] = pid;
        }
    }

    const int screenWidth = 650;
    const int screenHeight = 550;

    InitWindow(screenWidth, screenHeight, "Delivery System Visualizer - Milestone 6");
    InitGraphLayout(screenWidth, screenHeight);
    SetTargetFPS(60);

    TravelerEntity entities[MAX_TRAVELERS];
    InitTravelerEntities(entities, travelerCount);

    int finishedCount = 0;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (finishedCount < travelerCount) {
            readMessagesFromChildren(pipes, travelerCount, finished,
                                     &finishedCount, entities);
        }

UpdateTravelerEntitiesM6(entities, travelerCount, deltaTime, graph);

        BeginDrawing();

        DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
                               (Color){18, 24, 38, 255},
                               (Color){45, 62, 90, 255});

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

    cleanup_semaphores(graph->numNodes);
    cleanup(graph, travelers);
    CloseWindow();

    return 0;
}
