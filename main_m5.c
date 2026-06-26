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

    if (write(writeFd, &msg, sizeof(TravelMessage)) == -1) {
        perror("write");
    }
}

/* CHANGE:
   Added ackReadFd parameter.
   Why: after the child sends a message to the parent, it must wait for approval
   before continuing to the next node. */
static void runChildProcess(Graph *graph, Traveler traveler, int writeFd, int ackReadFd) {
    int path[PATH_SIZE];
    int pathLength = 0;
    int totalDistance = 0;

    dijkstra(graph, traveler.src, traveler.dst, path, &pathLength, &totalDistance);

    for (int i = 0; i < pathLength; i++) {
        int isLastNode = (i == pathLength - 1);

        if (isLastNode) {
            sendTravelMessage(writeFd, traveler.id, path[i], -1, 1, 1);

            /* CHANGE:
               Child waits for ACK from the parent after sending the message.
               Why: this blocks the child until the parent confirms it handled the message. */
            char ack;
            read(ackReadFd, &ack, sizeof(char));
        } else {
            sendTravelMessage(writeFd, traveler.id, path[i], path[i + 1], 0, 0);

            /* CHANGE:
               Child waits for ACK from the parent before moving to the next node.
               Why: the task requires the child to continue only after parent approval. */
            char ack;
            read(ackReadFd, &ack, sizeof(char));

            int weight = getEdgeWeight(graph, path[i], path[i + 1]);
            if (weight <= 0) weight = 1;

            usleep((weight + 1) * 700000);
        }
    }

    /* CHANGE:
       Close the ACK pipe read end in the child.
       Why: the child no longer needs to receive approvals after finishing. */
    close(ackReadFd);

    close(writeFd);
    exit(0);
}

/* CHANGE:
   Added ackPipes parameter.
   Why: normal pipes are child-to-parent, and ackPipes are parent-to-child. */
static int createChildProcesses(Graph *graph, Traveler *travelers, int travelerCount,
                                int pipes[MAX_TRAVELERS][2],
                                int ackPipes[MAX_TRAVELERS][2],
                                pid_t pids[MAX_TRAVELERS]) {
    for (int i = 0; i < travelerCount; i++) {

        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return 0;
        }

        /* CHANGE:
           Create a second pipe for ACK messages from parent to child.
           Why: the parent needs a way to send approval back to the child. */
        if (pipe(ackPipes[i]) == -1) {
            perror("ack pipe");
            return 0;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            return 0;
        }

        if (pid == 0) {
            /* CHANGE:
               Child closes unused pipe ends.
               Why: child writes to pipes[i][1] and reads ACK from ackPipes[i][0]. */
            close(pipes[i][0]);
            close(ackPipes[i][1]);

            runChildProcess(graph, travelers[i], pipes[i][1], ackPipes[i][0]);
        }

        pids[i] = pid;
        travelers[i].pid = pid;

        /* CHANGE:
           Parent closes unused pipe ends.
           Why: parent reads from pipes[i][0] and writes ACK to ackPipes[i][1]. */
        close(pipes[i][1]);
        close(ackPipes[i][0]);

        int flags = fcntl(pipes[i][0], F_GETFL, 0);
        if (flags == -1) return 0;

        if (fcntl(pipes[i][0], F_SETFL, flags | O_NONBLOCK) == -1) {
            return 0;
        }
    }

    return 1;
}

/* CHANGE:
   Added ackPipes parameter.
   Why: parent must send ACK to each child after reading its message. */
static void readMessagesFromChildren(int pipes[MAX_TRAVELERS][2],
                                    int ackPipes[MAX_TRAVELERS][2],
                                    int travelerCount,
                                    int finished[MAX_TRAVELERS],
                                    int *finishedCount,
                                    TravelerEntity entities[MAX_TRAVELERS]) {
    for (int i = 0; i < travelerCount; i++) {
        if (finished[i]) continue;

        TravelMessage msg;
        ssize_t bytesRead = read(pipes[i][0], &msg, sizeof(TravelMessage));

        if (bytesRead == sizeof(TravelMessage)) {
            if (msg.isDestination) {
                printf("[PID=%d] arrived at node %d | DESTINATION\n",
                       msg.pid, msg.currentNode);
            } else {
                printf("[PID=%d] arrived at node %d | next node: %d\n",
                       msg.pid, msg.currentNode, msg.nextNode);
            }

            UpdateEntityFromMessage(entities, msg);

            /* CHANGE:
               Parent sends one-byte ACK after handling the child message.
               Why: this releases the child so it can continue to the next node. */
            char ack = 'A';
            write(ackPipes[i][1], &ack, sizeof(char));

            if (msg.isFinished) {
                printf("[PID=%d] finished\n", msg.pid);
                finished[i] = 1;
                (*finishedCount)++;

                /* CHANGE:
                   Close both communication directions after child finished.
                   Why: no more messages or ACKs are needed for this child. */
                close(pipes[i][0]);
                close(ackPipes[i][1]);
            }
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

    /* CHANGE:
       Added ACK pipes, one per child.
       Why: each child needs to receive approval from the parent before continuing. */
    int ackPipes[MAX_TRAVELERS][2];

    pid_t pids[MAX_TRAVELERS];
    int finished[MAX_TRAVELERS] = {0};

    /* CHANGE:
       Pass ackPipes to child creation.
       Why: createChildProcesses must create both normal pipes and ACK pipes. */
    if (!createChildProcesses(graph, travelers, travelerCount, pipes, ackPipes, pids)) {
        cleanup(graph, travelers);
        return 1;
    }

    const int screenWidth = 650;
    const int screenHeight = 550;

    InitWindow(screenWidth, screenHeight, "Delivery System Visualizer ");
    InitGraphLayout(screenWidth, screenHeight);
    SetTargetFPS(60);

    TravelerEntity entities[MAX_TRAVELERS];
    InitTravelerEntities(entities, travelerCount);

    int finishedCount = 0;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (finishedCount < travelerCount) {
            /* CHANGE:
               Pass ackPipes when reading messages.
               Why: parent reads a child message and then sends ACK back. */
            readMessagesFromChildren(pipes, ackPipes, travelerCount,
                                     finished, &finishedCount, entities);
        }

        UpdateTravelerEntities(entities, travelerCount, deltaTime, graph);

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

    /* Terminate remaining child processes immediately if window closed early */
    for (int i = 0; i < travelerCount; i++) {
        if (!finished[i]) {
            kill(pids[i], SIGTERM);
            close(pipes[i][0]);

            /* CHANGE:
               Close ACK pipe if the window was closed before child finished.
               Why: cleanup resources for unfinished children. */
            close(ackPipes[i][1]);
        }

        /* Clean up process resources securely to prevent zombies */
        waitpid(pids[i], NULL, 0);
    }

    CloseWindow();
    cleanup(graph, travelers);

    return 0;
}
