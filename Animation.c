#include "Animation.h"
#include "Visualizer.h"
#include "Graph.h"

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
    if (W <= 0) {
        W = 1;
    }

    if (entity->status == ENTITY_MOVING) {
        if (entity->timer >= 0.7f) {
            entity->jumpStep++;
            entity->timer = 0.0f;

            float t = (float)entity->jumpStep / W;

            if (t > 1.0f) {
                t = 1.0f;
            }

            if (u == 2 && v == 5) {
                Vector2 control = {(startP.x + endP.x) / 2, startP.y - 100};
                entity->currentPos = GetBezierPoint(startP, endP, control, t);
            } else if (u == 1 && v == 3) {
                Vector2 control = {(startP.x + endP.x) / 2, startP.y + 120};
                entity->currentPos = GetBezierPoint(startP, endP, control, t);
            } else {
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

void DrawMovingEntity(MovingEntity entity, Color color) {
    DrawCircleV(entity.currentPos, 12, color);
}

void DrawEntityAtSource(Vector2 sourcePosition) {
    MovingEntity entity;
    InitEntity(&entity, sourcePosition);
    DrawMovingEntity(entity, (Color){15, 35, 110, 225});
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

void InitTravelerEntities(TravelerEntity entities[], int travelerCount) {
    Color colors[] = {
        RED, BLUE, GREEN, ORANGE, PURPLE, PINK, YELLOW
    };

    for (int i = 0; i < travelerCount; i++) {
        entities[i].currentPos = (Vector2){0, 0};
        entities[i].startPos = (Vector2){0, 0};
        entities[i].targetPos = (Vector2){0, 0};

        entities[i].travelerId = i;
        entities[i].currentNode = -1;
        entities[i].nextNode = -1;

        entities[i].timer = 0.0f;
        entities[i].isMoving = false;
        entities[i].isFinished = false;

        entities[i].edgeWeight = 1;
        entities[i].jumpStep = 0;

        entities[i].color = colors[i % 7];
    }
}

void UpdateEntityFromMessage(TravelerEntity entities[], TravelMessage msg) {
    int id = msg.travelerId;

    entities[id].travelerId = id;
    entities[id].currentNode = msg.currentNode;
    entities[id].nextNode = msg.nextNode;

    entities[id].startPos = GetNodePosition(msg.currentNode);
    entities[id].currentPos = entities[id].startPos;
    entities[id].timer = 0.0f;
    entities[id].jumpStep = 0;

    if (msg.isDestination || msg.isFinished || msg.nextNode == -1) {
        entities[id].targetPos = entities[id].startPos;
        entities[id].isMoving = false;
        entities[id].isFinished = true;
        entities[id].edgeWeight = 1;
    } else {
        entities[id].targetPos = GetNodePosition(msg.nextNode);
        entities[id].isMoving = true;
        entities[id].isFinished = false;
    }
}

void UpdateTravelerEntities(TravelerEntity entities[], int travelerCount, float deltaTime, void *graph) {
    for (int i = 0; i < travelerCount; i++) {
        if (entities[i].isMoving) {
            entities[i].timer += deltaTime;

            int u = entities[i].currentNode;
            int v = entities[i].nextNode;

            int W = getEdgeWeight((Graph*)graph, u, v);

            if (W <= 0) {
                W = 1;
            }

            entities[i].edgeWeight = W;

            if (entities[i].timer >= 0.7f) {
                entities[i].jumpStep++;
                entities[i].timer = 0.0f;

                float t = (float)entities[i].jumpStep / W;

                if (t >= 1.0f) {
                    t = 1.0f;
                    entities[i].isMoving = false;
                }

                if (u == 2 && v == 5) {
                    Vector2 control = {
                        (entities[i].startPos.x + entities[i].targetPos.x) / 2,
                        entities[i].startPos.y - 100
                    };

                    entities[i].currentPos =
                        GetBezierPoint(entities[i].startPos, entities[i].targetPos, control, t);
                } else if (u == 1 && v == 3) {
                    Vector2 control = {
                        (entities[i].startPos.x + entities[i].targetPos.x) / 2,
                        entities[i].startPos.y + 120
                    };

                    entities[i].currentPos =
                        GetBezierPoint(entities[i].startPos, entities[i].targetPos, control, t);
                } else {
                    entities[i].currentPos.x =
                        entities[i].startPos.x +
                        (entities[i].targetPos.x - entities[i].startPos.x) * t;

                    entities[i].currentPos.y =
                        entities[i].startPos.y +
                        (entities[i].targetPos.y - entities[i].startPos.y) * t;
                }
            }
        }
    }
}

void DrawTravelerEntities(TravelerEntity entities[], int travelerCount) {
    for (int i = 0; i < travelerCount; i++) {
        if (entities[i].currentNode != -1) {
            DrawCircleV(entities[i].currentPos, 12, entities[i].color);
            DrawCircleLinesV(entities[i].currentPos, 14, WHITE);
        }
    }
}
