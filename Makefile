# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -I.

# Libraries (Raylib and system dependencies)
LIBS = ./libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11

# Source files shared across all milestones
COMMON_SRCS = Graph.c Dijkstra.c InputHandler.c

# Source files specific to the Visualizer (Milestone 2 & 3)
VISUAL_SRCS = Visualizer.c

# Source files specific to the Animation (Milestone 3)
ANIMATION_SRCS = Animation.c

# Default target 
all: milestone1 milestone2 milestone3 milestone4 milestone5 milestone6 milestone7

# Build Milestone 1: Console-based program
milestone1:
	$(CC) $(CFLAGS) $(COMMON_SRCS) main.c -o dijkstra $(LIBS)

# Build Milestone 2: GUI-based program
milestone2:
	$(CC) $(CFLAGS) $(COMMON_SRCS) $(VISUAL_SRCS) main.c -DMILESTONE_2_GUI -o sim $(LIBS)

# Build Milestone 3: Animation and Control
milestone3:
	$(CC) $(CFLAGS) $(COMMON_SRCS) $(VISUAL_SRCS) $(ANIMATION_SRCS) main_m3.c -o sim $(LIBS)

# Build Milestone 4: Multi-Process and Parent Process 
milestone4:
	$(CC) $(CFLAGS) $(COMMON_SRCS) $(VISUAL_SRCS) $(ANIMATION_SRCS) main_m4.c -o sim $(LIBS)
milestone5:
	$(CC) $(CFLAGS) $(COMMON_SRCS) $(VISUAL_SRCS) $(ANIMATION_SRCS) main_m5.c -o sim $(LIBS)
# Build Milestone 6: Multi-Process Synchronization 
milestone6:
	$(CC) $(CFLAGS) $(COMMON_SRCS) $(VISUAL_SRCS) $(ANIMATION_SRCS) main_m6.c -o sim $(LIBS)
# Build Milestone 7: Centralized CPU Scheduling Simulator (As requested)
milestone7: Graph.c Dijkstra.c InputHandler.c Visualizer.c Animation.c Scheduler.c main_m7.c
	$(CC) $(CFLAGS) Graph.c Dijkstra.c InputHandler.c Visualizer.c Animation.c Scheduler.c main_m7.c -o sim $(LIBS)

# Clean build artifacts
clean:
	rm -f dijkstra sim 
