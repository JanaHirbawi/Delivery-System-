#include "Animation.h"
#include "Visualizer.h"
static Vector2 GetBezierPoint(Vector2 start, Vector2 end, Vector2 control, float t) {
    float u = 1.0f - t;

    Vector2 p = {
        u * u * start.x + 2 * u * t * control.x + t * t * end.x,
        u * u * start.y + 2 * u * t * control.y + t * t * end.y
    };

    return p;
}

void InitEntity(MovingEntity *entity, Vector2 startPosition) {
    entity->currentPos = startPosition;
    entity->currentPathIndex = 0;
    entity->timer = 0.0f;
    entity->jumpStep = 0;
    entity->status = ENTITY_IDLE;
    entity->isFinished = false;
}

void UpdateEntity(MovingEntity *entity, int path[], int pathLength, float deltaTime, void *graph) {
    if (entity->isFinished || entity->status == ENTITY_IDLE) {
        return;
    }

    if (pathLength < 2) {
        entity->isFinished = true;
        entity->status = ENTITY_IDLE;
        return;
    }

    entity->timer += deltaTime;

    int u = path[entity->currentPathIndex];
    int v = path[entity->currentPathIndex + 1];

    Vector2 startP = GetNodePosition(u);
    Vector2 endP = GetNodePosition(v);

int W = getEdgeWeight((Graph*)graph, u, v);

    if (entity->status == ENTITY_MOVING) {
        if (entity->timer >= 0.3f) {
            entity->jumpStep++;
            entity->timer = 0.0f;

            float t = (float)entity->jumpStep / W;

            if (t > 1.0f) {
                t = 1.0f;
            }

            if (u == 2 && v == 5) {
    Vector2 control = {(startP.x + endP.x) / 2, startP.y - 100};
    entity->currentPos = GetBezierPoint(startP, endP, control, t);
}
else if (u == 1 && v == 3) {
    Vector2 control = {(startP.x + endP.x) / 2, startP.y + 120};
    entity->currentPos = GetBezierPoint(startP, endP, control, t);
}
else {
    entity->currentPos.x = startP.x + t * (endP.x - startP.x);
    entity->currentPos.y = startP.y + t * (endP.y - startP.y);
}

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
    } else if (entity->status == ENTITY_WAITING) {
        if (entity->timer >= 1.0f) {
            entity->currentPathIndex++;
            entity->jumpStep = 0;
            entity->timer = 0.0f;
            entity->status = ENTITY_MOVING;
        }
    }
}

bool HasReachedTarget(MovingEntity entity, Vector2 target) {
    return entity.currentPos.x == target.x && entity.currentPos.y == target.y;
}

void DrawMovingEntity(MovingEntity entity) {
    DrawCircleV(entity.currentPos, 12, (Color){15, 35, 110, 255});
}

void DrawEntityAtSource(Vector2 sourcePosition) {
    MovingEntity entity;
    InitEntity(&entity, sourcePosition);
    DrawMovingEntity(entity);
}
static int animationPath[100];
static int animationPathLength = 0;
static int currentPathIndex = 0;

void SetAnimationPath(int path[], int length) {
    animationPathLength = length;
    currentPathIndex = 0;

    for (int i = 0; i < length; i++) {
        animationPath[i] = path[i];
    }
}

int GetCurrentNode(void) {
    if (animationPathLength == 0) {
        return -1;
    }

    return animationPath[currentPathIndex];
}

int GetNextNode(void) {
    if (animationPathLength == 0 || currentPathIndex >= animationPathLength - 1) {
        return -1;
    }

    return animationPath[currentPathIndex + 1];
}

void MoveToNextPathIndex(void) {
    if (currentPathIndex < animationPathLength - 1) {
        currentPathIndex++;
    }
}

bool IsPathFinished(void) {
    return currentPathIndex >= animationPathLength - 1;
}
