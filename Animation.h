#ifndef ANIMATION_H
#define ANIMATION_H

#include "raylib.h"
#include <stdbool.h>

// Entity status: moving, waiting, or idle
typedef enum {
    ENTITY_IDLE,    
    ENTITY_MOVING,  
    ENTITY_WAITING  
} EntityStatus;

// Main structure for movement and timing
typedef struct {
    Vector2 currentPos;      // (x, y) on screen
    int currentPathIndex;    // Current node in path array
    float timer;             // Timer for 300ms jumps and 1s wait
    int jumpStep;            // Jump counter (0 to W)
    EntityStatus status;     // Current movement state
    bool isFinished;         // True when destination reached
} MovingEntity;

// Initialize entity at the start
void InitEntity(MovingEntity *entity, Vector2 startPosition);

// Core logic for jumps and node waiting
void UpdateEntity(MovingEntity *entity, int path[], int pathLength, float deltaTime, void *graph);

// Check if a specific target is reached
bool HasReachedTarget(MovingEntity entity, Vector2 target);

#endif
