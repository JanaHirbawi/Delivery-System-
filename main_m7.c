#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#include "raylib.h"
#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"
#include "Visualizer.h"
#include "Animation.h"
#include "Scheduler.h"

#define MAX_TRAVELERS 100
#define PATH_SIZE 100
double nodeReleaseTime[MAX_NODES] = {0.0};

/* Function prototypes for process handling and message routing */
void runChildProcess(Graph *graph, Traveler traveler, int writePipe, int readPipe);
static void cleanup(Graph *graph, Traveler *travelers);

static int findTravelerIndex(Traveler *travelers, int travelerCount, int travelerId) {
    for (int i = 0; i < travelerCount; i++) {
        if (travelers[i].id == travelerId) {
            return i;
        }
    }

    if (travelerId >= 0 && travelerId < travelerCount) {
        return travelerId;
    }

    return -1;
}

static int calculateRemainingDistance(Graph *graph, int currentNode, int destinationNode) {
    int path[PATH_SIZE];
    int pathLength = 0;
    int totalDistance = 0;

    dijkstra(graph, currentNode, destinationNode, path, &pathLength, &totalDistance);

    return totalDistance;
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

/* * readMessagesFromChildren: Centralized Parent function to read status updates from pipes.
 * JANNAT: handles WAITING / GO / LEAVING handshake signals.
 */
void readMessagesFromChildren(int childToParent[][2],
                              int travelerCount,
                              int finished[],
                              int *finishedCount,
                              TravelerEntity entities[],
                              Graph *graph,
                              Traveler *travelers) {
    for (int i = 0; i < travelerCount; i++) {
        if (finished[i]) continue;

        int flags = fcntl(childToParent[i][0], F_GETFL, 0);
        fcntl(childToParent[i][0], F_SETFL, flags | O_NONBLOCK);

        TravelMessage msg;
        ssize_t bytesRead = read(childToParent[i][0], &msg, sizeof(TravelMessage));

        if (bytesRead == sizeof(TravelMessage)) {
            if (msg.status == STATUS_WAITING) {
                int travelerIndex = findTravelerIndex(travelers, travelerCount, msg.travelerId);
                int remainingDistance = 0;

                if (travelerIndex != -1 && msg.currentNode != -1) {
                    remainingDistance = calculateRemainingDistance(graph,
                                                                   msg.currentNode,
                                                                   travelers[travelerIndex].dst);
                }

                enqueueTraveler(msg.currentNode, msg.travelerId, remainingDistance);

                printf("[PARENT] Traveler %d WAITING for node %d\n",
                       msg.travelerId, msg.currentNode);
            }
            else if (msg.status == STATUS_ENTERED) {
                printf("[PARENT] Traveler %d ENTERED node %d\n",
                       msg.travelerId, msg.currentNode);
            }
            else if (msg.status == STATUS_LEAVING) {
    printf("[PARENT] Traveler %d LEAVING node %d\n",
           msg.travelerId, msg.currentNode);

    if (msg.currentNode >= 0 && msg.currentNode < MAX_NODES) {
        nodeReleaseTime[msg.currentNode] = GetTime() + 1.2;
    }
}
            else if (msg.status == STATUS_FINISHED) {
                printf("[PARENT] Traveler %d FINISHED\n", msg.travelerId);
            }

            fflush(stdout);

            if (msg.isFinished) {
                finished[i] = 1;
                (*finishedCount)++;
                entities[i].isFinished = true;
                entities[i].currentNode = -1;
            }
            else {
                UpdateEntityFromMessage(entities, msg);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    
    if (argc != 4 || strcmp(argv[1], "-schd") != 0) {
        printf("Usage: ./sim -schd <fcfs|sjf> <file_name>\n");
        return 1;
    }

    if (strcmp(argv[2], "fcfs") == 0) {
        schedulerType = SCHED_FCFS;
    }
    else if (strcmp(argv[2], "sjf") == 0) {
        schedulerType = SCHED_SJF;
    }
    else {
        printf("Usage: ./sim -schd <fcfs|sjf> <file_name>\n");
        return 1;
    }

    char *filename = argv[3];

    Graph *graph = NULL;
    Traveler *travelers = NULL;
    int travelerCount = 0;

    if (!loadGraphFromFile(filename, &graph, &travelers, &travelerCount)) {
        fprintf(stderr, "Error: Failed to safely load configuration file.\n");
        return 1;
    }

    /* 🚀 DIANA'S MODULE INITIALIZATION: Clear and boot up state-queues and timers */
    InitScheduler();

    int childToParent[MAX_TRAVELERS][2];
    int parentToChild[MAX_TRAVELERS][2];
    pid_t pids[MAX_TRAVELERS];
    int finished[MAX_TRAVELERS] = {0};

    /* Spawn dedicated independent Child Processes for each Traveler entity */
    for (int i = 0; i < travelerCount; i++) {
        if (pipe(childToParent[i]) == -1) {
            perror("Error: childToParent pipe creation failed");
            cleanup(graph, travelers);
            return 1;
        }

        if (pipe(parentToChild[i]) == -1) {
            perror("Error: parentToChild pipe creation failed");
            cleanup(graph, travelers);
            return 1;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Error: Process replication via fork failed");
            cleanup(graph, travelers);
            return 1;
        }

        if (pid == 0) {
            close(childToParent[i][0]);
            close(parentToChild[i][1]);

            runChildProcess(graph, travelers[i], childToParent[i][1], parentToChild[i][0]);

            close(childToParent[i][1]);
            close(parentToChild[i][0]);

            exit(0);
        } else {
            close(childToParent[i][1]);
            close(parentToChild[i][0]);
            pids[i] = pid;
        }
    }

    /* GUI Canvas Dimensions configuration */
    const int screenWidth = 700;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Delivery System Visualizer - Milestone 7 (CPU Scheduling)");
    InitGraphLayout(screenWidth, screenHeight);
    SetTargetFPS(60);

    TravelerEntity entities[MAX_TRAVELERS];
    InitTravelerEntities(entities, travelerCount);

    int finishedCount = 0;

    /* Main Execution Thread / Graphical Render Loop */
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        /* Process inbound child IPC update streams */
        if (finishedCount < travelerCount) {
           readMessagesFromChildren(childToParent, travelerCount,
                                     finished, &finishedCount, entities,
                                     graph, travelers);
            
            /* 🚀 DIANA'S CORE PERIODIC SCHEDULER MONITORING:
             * The Parent checks every free node intersection and dispatches waiting jobs
             */
            for (int node = 0; node < graph->numNodes; node++) {

    if (nodeQueues[node].isOccupied &&
        nodeReleaseTime[node] > 0 &&
        GetTime() >= nodeReleaseTime[node]) {

        nodeQueues[node].isOccupied = 0;
        nodeQueues[node].currentOccupiedBy = -1;
        nodeReleaseTime[node] = 0;
    }

    if (!nodeQueues[node].isOccupied && nodeQueues[node].size > 0) {
        int nextTraveler = selectNextTraveler(node);

                    if (nextTraveler != -1) {
                        int travelerIndex = findTravelerIndex(travelers, travelerCount, nextTraveler);

                        if (travelerIndex != -1) {
                            /* Mark Node Occupied and signal chosen child via GO command */
                            nodeQueues[node].isOccupied = 1;
                            nodeQueues[node].currentOccupiedBy = nextTraveler;

                            if (write(parentToChild[travelerIndex][1], "GO", 2) == -1) {
                                perror("write GO");
                            }

                            printf("[PARENT] GO sent to Traveler %d for Node %d\n",
                                   nextTraveler, node);
                            fflush(stdout);
                        }
                    }
                }
            }
        }

        UpdateTravelerEntitiesM6(entities, travelerCount, deltaTime, graph);

        /* Render Pass */
        BeginDrawing();
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
                               (Color){15, 20, 32, 255},
                               (Color){38, 52, 75, 255});

        drawEdges(graph);
        DrawStaticGraph();
        DrawTravelerEntities(entities, travelerCount);

        /* 🛠️ JANA'S GUI HUD TARGET: Display active scheduling algorithm metric on screen */
        DrawText(TextFormat("ACTIVE SCHEDULER: %s", (schedulerType == SCHED_SJF ? "SHORTEST JOB FIRST (SJF)" : "FIRST-COME, FIRST-SERVED (FCFS)")), 
                 20, 20, 16, (schedulerType == SCHED_SJF ? GOLD : SKYBLUE));

        EndDrawing();
    }

    /* 📊 STATISTICAL PRINTS AT TERMINATION */
    printf("\n===============================================\n");
    printf("   FINAL MILESTONE 7 PERFORMANCE SIMULATION LOG\n");
    printf("===============================================\n");
    double aggregateWait = 0;
    for(int i = 0; i < travelerCount; i++) {
        printf("Traveler %d -> Accumulated Node Queue Delay: %.2f seconds\n", i, totalWaitingTime[i]);
        aggregateWait += totalWaitingTime[i];
    }
    printf("-----------------------------------------------\n");
    printf(">> AVERAGE SYSTEM-WIDE WAITING TIME: %.2f seconds\n", 
           (travelerCount > 0 ? (aggregateWait / travelerCount) : 0.0));
    printf("===============================================\n");

    /* Cleanup running context resources safely */
    for (int i = 0; i < travelerCount; i++) {
        if (!finished[i]) {
            kill(pids[i], SIGTERM);
        }

        close(childToParent[i][0]);
        close(parentToChild[i][1]);
        waitpid(pids[i], NULL, 0);
    }

    cleanup(graph, travelers);
    CloseWindow();

    return 0;
}

static void cleanup(Graph *graph, Traveler *travelers) {
    if (travelers != NULL) free(travelers);
    if (graph != NULL) freeGraph(graph);
}

/* Jannat's IPC code: child waits for GO from parent before entering each node */
void runChildProcess(Graph *graph, Traveler traveler, int writePipe, int readPipe) {
    int path[PATH_SIZE];
    int pathLength = 0;
    int totalDistance = 0;

    dijkstra(graph, traveler.src, traveler.dst, path, &pathLength, &totalDistance);

    for (int i = 0; i < pathLength; i++) {
        int currNode = path[i];
        int nextNode = (i < pathLength - 1) ? path[i + 1] : -1;
        int isDest = (i == pathLength - 1);

        sendTravelMessage(writePipe, traveler.id, currNode, nextNode,
                          isDest, 0, STATUS_WAITING);

        char command[8];
        ssize_t bytesRead = read(readPipe, command, sizeof(command) - 1);

        if (bytesRead <= 0) {
            perror("read GO");
            exit(1);
        }

        command[bytesRead] = '\0';

        sendTravelMessage(writePipe, traveler.id, currNode, nextNode,
                          isDest, 0, STATUS_ENTERED);

        sleep(1);

        sendTravelMessage(writePipe, traveler.id, currNode, nextNode,
                          isDest, 0, STATUS_LEAVING);

        usleep(700000);

        if (nextNode != -1) {
            int weight = getEdgeWeight(graph, currNode, nextNode);
            usleep(weight * 1000000);
        }
    }

    sendTravelMessage(writePipe, traveler.id, -1, -1,
                      1, 1, STATUS_FINISHED);

    exit(0);
}
