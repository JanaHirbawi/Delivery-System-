#ifndef ANIMATION_H
#define ANIMATION_H

#include "raylib.h"
#include <stdbool.h>

typedef enum {
    ENTITY_IDLE,
    ENTITY_MOVING,
    ENTITY_WAITING
} EntityStatus;

typedef struct {
    Vector2 currentPos;
    int currentPathIndex;
    float timer;
    int jumpStep;
    EntityStatus status;
    bool isFinished;
} MovingEntity;

void InitEntity(MovingEntity *entity, Vector2 startPosition);
void UpdateEntity(MovingEntity *entity, int path[], int pathLength, float deltaTime, void *graph);
bool HasReachedTarget(MovingEntity entity, Vector2 target);

void DrawMovingEntity(MovingEntity entity);
void DrawEntityAtSource(Vector2 sourcePosition);
void SetAnimationPath(int path[], int length);
int GetCurrentNode(void);
int GetNextNode(void);
void MoveToNextPathIndex(void);
bool IsPathFinished(void);
#endif
