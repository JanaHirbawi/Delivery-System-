# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I.

# Libraries (Raylib and system dependencies)
LIBS = ./libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11

# Source files shared across all milestones
COMMON_SRCS = Graph.c Dijkstra.c InputHandler.c

# Source files specific to the Visualizer (Milestone 2)
VISUAL_SRCS = Visualizer.c

# Default target
all: milestone1 milestone2

# Build Milestone 1: Console-based program
milestone1:
	$(CC) $(CFLAGS) $(COMMON_SRCS) main.c -o dijkstra $(LIBS)

# Build Milestone 2: GUI-based program
# Uses -D flag to define MILESTONE_2_GUI macro in main.c
milestone2:
	$(CC) $(CFLAGS) $(COMMON_SRCS) $(VISUAL_SRCS) main.c -DMILESTONE_2_GUI -o sim $(LIBS)

# Clean build artifacts
clean:
	rm -f dijkstra sim
