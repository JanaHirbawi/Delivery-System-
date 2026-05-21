#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "Graph.h"
#include "InputHandler.h"
#include "Dijkstra.h"

int main() {

    Graph *graph = NULL;
    Traveler *travelers = NULL;
    int travelerCount = 0;

    if (!loadGraphFromFile("input.txt", &graph, &travelers, &travelerCount)) {
        return 1;
    }

    printf("=== Milestone 4: Multi Process System ===\n");
    printf("Loaded %d travelers successfully.\n\n", travelerCount);

    for (int i = 0; i < travelerCount; i++) {

        printf("[Traveler ID: %d] Calculating shortest path from %d to %d:\n",
               travelers[i].id,
               travelers[i].src,
               travelers[i].dst);

        dijkstra(graph,
                 travelers[i].src,
                 travelers[i].dst,
                 travelers[i].path,
                 &travelers[i].pathLength,
                 &travelers[i].totalDistance);

        printf("Total Distance: %d\n", travelers[i].totalDistance);
        printf("-----------------------------------------------\n\n");
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

    printf("Parent created all child processes.\n");

    for (int i = 0; i < travelerCount; i++) {

        sleep(2);

        printf("Parent terminating traveler PID %d\n",
               travelers[i].pid);

        kill(travelers[i].pid, SIGTERM);
    }

    for (int i = 0; i < travelerCount; i++) {

        waitpid(travelers[i].pid, NULL, 0);
    }

    printf("All child processes finished.\n");


    free(travelers);
    freeGraph(graph);

    return 0;
}