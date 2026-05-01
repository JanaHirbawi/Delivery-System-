# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I.

# Libraries (Raylib and system dependencies)
LIBS = ./libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11

# Source files shared across all milestones
COMMON_SRCS = Graph.c Dijkstra.c InputHandler.c

# Source files specific to the Visualizer (Milestone 2 & 3)
VISUAL_SRCS = Visualizer.c

# Source files specific to the Animation (Milestone 3)
ANIMATION_SRCS = Animation.c

# Default target
all: milestone1 milestone2 milestone3

# Build Milestone 1: Console-based program
milestone1:
	$(CC) $(CFLAGS) $(COMMON_SRCS) main.c -o dijkstra $(LIBS)

# Build Milestone 2: GUI-based program
milestone2:
	$(CC) $(CFLAGS) $(COMMON_SRCS) $(VISUAL_SRCS) main.c -DMILESTONE_2_GUI -o sim $(LIBS)

# Build Milestone 3: Animation and Control
# نستخدم main_m3.c ليكون منفصلاً عن main القديم
milestone3:
	$(CC) $(CFLAGS) $(COMMON_SRCS) $(VISUAL_SRCS) $(ANIMATION_SRCS) main_m3.c -o sim $(LIBS)

# Clean build artifacts
clean:
	rm -f dijkstra sim
