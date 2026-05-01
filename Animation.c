#include "Animation.h"
#include "Visualizer.h"
#include "Graph.h"
#include <math.h>

// 1. Initialize the entity at the starting point
void InitEntity(MovingEntity *entity, Vector2 startPosition) {
    entity->currentPos = startPosition;
    entity->currentPathIndex = 0;
    entity->timer = 0.0f;
    entity->jumpStep = 0;
    entity->status = ENTITY_IDLE; // Wait for Play button
    entity->isFinished = false;
}

// 2. Core Logic: Handles Jumps (300ms) and Waiting (1s)
void UpdateEntity(MovingEntity *entity, int path[], int pathLength, float deltaTime, void *graphPtr) {
    // If finished or idle, do nothing
    if (entity->isFinished || entity->status == ENTITY_IDLE) return;

    Graph *graph = (Graph *)graphPtr;
    entity->timer += deltaTime;

    // Get current node and next node in the path
    int u = path[entity->currentPathIndex];
    int v = path[entity->currentPathIndex + 1];

    // Get edge weight (W) for jumps
    int W = 1;
    Edge *curr = graph->adjList[u];
    while (curr) {
        if (curr->to == v) { W = curr->weight; break; }
        curr = curr->next;
    }

    Vector2 startP = GetNodePosition(u);
    Vector2 endP = GetNodePosition(v);

    // --- CASE 1: MOVING IN JUMPS (300ms per jump) ---
    if (entity->status == ENTITY_MOVING) {
        if (entity->timer >= 0.3f) { // 300ms passed
            entity->jumpStep++;
            entity->timer = 0.0f;

            // Calculate new position based on jump step / total W
            float t = (float)entity->jumpStep / W;
            entity->currentPos.x = startP.x + t * (endP.x - startP.x);
            entity->currentPos.y = startP.y + t * (endP.y - startP.y);

            // Reached the next node?
            if (entity->jumpStep >= W) {
                if (entity->currentPathIndex + 1 == pathLength - 1) {
                    entity->isFinished = true;
                    entity->status = ENTITY_IDLE;
                } else {
                    entity->status = ENTITY_WAITING;
                }
                entity->timer = 0.0f;
            }
        }
    }
    // --- CASE 2: WAITING AT NODE (1 second) ---
    else if (entity->status == ENTITY_WAITING) {
        if (entity->timer >= 1.0f) { // 1 second wait over
            entity->currentPathIndex++;
            entity->jumpStep = 0;
            entity->timer = 0.0f;
            entity->status = ENTITY_MOVING;
        }
    }
}

// 3. Check if target coordinates reached
bool HasReachedTarget(MovingEntity entity, Vector2 target) {
    return (entity.currentPos.x == target.x && entity.currentPos.y == target.y);
}
