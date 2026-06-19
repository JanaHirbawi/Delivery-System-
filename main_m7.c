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

/* Function prototypes for process handling and message routing */
void runChildProcess(Graph *graph, Traveler traveler, int writePipe);
static void cleanup(Graph *graph, Traveler *travelers);

/* * readMessagesFromChildren: Centralized Parent function to read status updates from pipes.
 * NOTE: JANNAT will expand this protocol to support WAITING / GO / LEAVING handshake signals.
 */
void readMessagesFromChildren(int pipes[][2], int travelerCount, int finished[], 
                              int *finishedCount, TravelerEntity entities[]) {
    char buffer[256];
    for (int i = 0; i < travelerCount; i++) {
        if (finished[i]) continue;

        /* Non-blocking read check on child pipes */
        int flags = fcntl(pipes[i][0], F_GETFL, 0);
        fcntl(pipes[i][0], F_SETFL, flags | O_NONBLOCK);

        int bytesRead = read(pipes[i][0], buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            
            int id, u, v;
            float t;
            /* Parse standard animation tracking message */
            if (sscanf(buffer, "MOVE %d %d %d %f", &id, &u, &v, &t) == 4) {
                entities[id].currentNode = u;
                entities[id].nextNode = v;
                entities[id].timer = t;
                entities[id].isMoving = true;
                entities[id].status = ENTITY_MOVING;
            }
            /* Parse completion message */
            else if (strcmp(buffer, "FINISHED") == 0) {
                finished[i] = 1;
                (*finishedCount)++;
                entities[i].isMoving = false;
                entities[i].currentNode = entities[i].targetPos.x; /* Lock at destination */
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

    int pipes[MAX_TRAVELERS][2];
    pid_t pids[MAX_TRAVELERS];
    int finished[MAX_TRAVELERS] = {0};

    /* Spawn dedicated independent Child Processes for each Traveler entity */
    for (int i = 0; i < travelerCount; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Error: Core communication pipeline creation failed");
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
            close(pipes[i][0]);
            runChildProcess(graph, travelers[i], pipes[i][1]);
            exit(0); /* Safeguard child exit boundaries */
        } else {
            close(pipes[i][1]);
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
            readMessagesFromChildren(pipes, travelerCount, finished, &finishedCount, entities);
            
            /* 🚀 DIANA'S CORE PERIODIC SCHEDULER MONITORING:
             * The Parent checks every free node intersection and dispatches waiting jobs
             */
            for (int node = 0; node < graph->numNodes; node++) {
                if (!nodeQueues[node].isOccupied && nodeQueues[node].size > 0) {
                    int nextTraveler = selectNextTraveler(node);
                    if (nextTraveler != -1) {
                        /* Mark Node Occupied and signal chosen child via GO command */
                        nodeQueues[node].isOccupied = 1;
                        nodeQueues[node].currentOccupiedBy = nextTraveler;
                        
                        /* JANNAT will code the actual pipe transmission to release the child here */
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
        close(pipes[i][0]);
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

/* Temporary mockup child routine placeholder - Jannat will sync with M6 */
void runChildProcess(Graph *graph, Traveler traveler, int writePipe) {
    (void)graph; (void)traveler; (void)writePipe;
    /* Jannat's IPC code will live natively here */
}
