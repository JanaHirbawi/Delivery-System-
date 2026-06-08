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

// Array of semaphore pointers - allocated based on maximum graph nodes
sem_t *node_semaphores[MAX_TRAVELERS];

static void cleanup(Graph *graph, Traveler *travelers) {
    if (travelers != NULL) free(travelers);
    if (graph != NULL) freeGraph(graph);
}

// Function to initialize semaphores with unique names and an initial value of 1 (available)
void init_semaphores(int numNodes) {
    char sem_name[32];
    for (int i = 0; i < numNodes; i++) {
        sprintf(sem_name, "/sem_node_%d", i);
        
        // Unlink any leftover semaphore from previous runs to prevent deadlocks
        sem_unlink(sem_name); 
        
        // Create the named semaphore
        node_semaphores[i] = sem_open(sem_name, O_CREAT | O_EXCL, 0644, 1);
        if (node_semaphores[i] == SEM_FAILED) {
            perror("Error: sem_open failed");
            exit(1);
        }
    }
}

// Function to close and unlink semaphores from the system upon program termination
void cleanup_semaphores(int numNodes) {
    char sem_name[32];
    for (int i = 0; i < numNodes; i++) {
        sprintf(sem_name, "/sem_node_%d", i);
        sem_close(node_semaphores[i]);
        sem_unlink(sem_name); 
    }
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

    if (write(writeFd, &msg, sizeof(TravelMessage)) == -1) {
        perror("write");
    }
}

// Child process execution logic with embedded critical sections
static void runChildProcess(Graph *graph, Traveler traveler, int writeFd) {
    int path[PATH_SIZE];
    int pathLength = 0;
    int totalDistance = 0;

    dijkstra(graph, traveler.src, traveler.dst, path, &pathLength, &totalDistance);

    for (int i = 0; i < pathLength; i++) {
        int currNode = path[i];
        int nextNode = (i < pathLength - 1) ? path[i + 1] : -1;
        int isDest = (i == pathLength - 1);

        // Open the semaphore associated with the current node inside the child process
        char sem_name[32];
        sprintf(sem_name, "/sem_node_%d", currNode);
        sem_t *sem = sem_open(sem_name, 0); 

        /* ---------------------------------------------------------------
           [NOTE FOR BAT_OUL]: You can send the WAITING status here 
           before blocking on the semaphore.
           --------------------------------------------------------------- */

        // [DIANA'S TASK]: Lock the node (process blocks here if the node is occupied)
        sem_wait(sem);

        /* ---------------------------------------------------------------
           [NOTE FOR BAT_OUL]: You can send the ENTERED status here 
           after successfully acquiring the semaphore lock.
           --------------------------------------------------------------- */
        sendTravelMessage(writeFd, traveler.id, currNode, nextNode, isDest, 0);

        // Spend mandatory 1-second interval inside the node (Actual Critical Section)
        sleep(1);

        /* ---------------------------------------------------------------
           [NOTE FOR BAT_OUL]: You can send the LEAVING status here 
           right before releasing the semaphore lock.
           --------------------------------------------------------------- */

        // [DIANA'S TASK]: Release the node for the next traveler
        sem_post(sem);
        sem_close(sem);

        // Simulate edge traversal movement based on edge weight
        if (nextNode != -1) {
            int weight = getEdgeWeight(graph, currNode, nextNode);
            usleep(weight * 300000); 
        }
    }

    /* ---------------------------------------------------------------
       [NOTE FOR BAT_OUL]: You can send the FINISHED status here 
       when the traveler reaches the final destination.
       --------------------------------------------------------------- */
    sendTravelMessage(writeFd, traveler.id, -1, -1, 1, 1);
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

int main() {
    Graph *graph = NULL;
    Traveler *travelers = NULL;
    int travelerCount = 0;

    if (!loadGraphFromFile("input.txt", &graph, &travelers, &travelerCount)) {
        return 1;
    }

    // [DIANA'S TASK]: Initialize semaphores based on total nodes in the graph
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
            readMessagesFromChildren(pipes, travelerCount, finished, &finishedCount, entities);
        }

        UpdateTravelerEntities(entities, travelerCount, deltaTime, graph);

        BeginDrawing();
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
                               (Color){18, 24, 38, 255}, (Color){45, 62, 90, 255});

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

    // [DIANA'S TASK]: Final cleanup and removal of system named semaphores
    cleanup_semaphores(graph->numNodes);
    cleanup(graph, travelers);
    CloseWindow();

    return 0;
}
