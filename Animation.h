#ifndef ANIMATION_H
#define ANIMATION_H

#include "raylib.h"
#include <stdbool.h>
#include "InputHandler.h"

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



typedef struct {
    Vector2 currentPos;
    Vector2 startPos;
    Vector2 targetPos;

    int travelerId;
    int currentNode;
    int nextNode;

    float timer;
    bool isMoving;
    bool isFinished;

    Color color;
} TravelerEntity;

void InitTravelerEntities(TravelerEntity entities[], int travelerCount);

void UpdateEntityFromMessage(TravelerEntity entities[],TravelMessage msg);

void UpdateTravelerEntities(TravelerEntity entities[],int travelerCount,float deltaTime);

void DrawTravelerEntities(TravelerEntity entities[],int travelerCount);


void InitEntity(MovingEntity *entity, Vector2 startPosition);
void UpdateEntity(MovingEntity *entity, int path[], int pathLength, float deltaTime, void *graph);
bool HasReachedTarget(MovingEntity entity, Vector2 target);


void DrawMovingEntity(MovingEntity entity, Color color);
void DrawEntityAtSource(Vector2 sourcePosition);
void SetAnimationPath(int path[], int length);
int GetCurrentNode(void);
int GetNextNode(void);
void MoveToNextPathIndex(void);
bool IsPathFinished(void);

#endif
