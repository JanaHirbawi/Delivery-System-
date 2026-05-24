// بداية ملف main_m5.c المقترح لتسليمه لزميلاتك:
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "Graph.h"
#include "InputHandler.h"

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


    return 0;
}
