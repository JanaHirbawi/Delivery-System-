#include <stdio.h>
#include <math.h>
#include "raylib.h"
#include "Visualizer.h"
#include "Graph.h"

NodeVisual nodesLayout[NODE_COUNT];

static Color EDGE_COLOR = {220, 235, 245, 255};
static Color ARROW_COLOR = {255, 185, 90, 255};
static Color GLOW_COLOR = {90, 200, 255, 255};

static Color PATH_COLOR = {255, 220, 80, 255};
static Color PATH_GLOW = {255, 170, 40, 255};

static Color NODE_BORDER = {245, 245, 245, 255};
static Color TEXT_SHADOW = {10, 10, 10, 255};
static Color TITLE_COLOR = {245, 245, 245, 255};

static int shortestPath[] = {0, 2, 5};
static int shortestPathLength = 3;

void InitGraphLayout(int screenWidth, int screenHeight) {
    float centerY = screenHeight * 0.55f;

    float startX = 80.0f;
    float endX = screenWidth - 120.0f;
    float hSpacing = (endX - startX) / 3.0f;

    nodesLayout[1] = (NodeVisual){1, {startX, centerY}, "Customer 1", (Color){230, 235, 245, 255}};
    nodesLayout[2] = (NodeVisual){2, {startX + hSpacing, centerY}, "Hub Alpha", (Color){170, 210, 255, 255}};
    nodesLayout[3] = (NodeVisual){3, {startX + 2 * hSpacing, centerY}, "Customer B", (Color){230, 235, 245, 255}};
    nodesLayout[5] = (NodeVisual){5, {startX + 3 * hSpacing, centerY}, "Destination Hub", (Color){130, 145, 170, 255}};

    nodesLayout[0] = (NodeVisual){0, {nodesLayout[1].position.x + hSpacing / 2, centerY - 160}, "Warehouse", (Color){65, 210, 245, 255}};
    nodesLayout[4] = (NodeVisual){4, {nodesLayout[3].position.x, centerY + 130}, "Hub Beta", (Color){205, 245, 220, 255}};
}

int isShortestPathEdge(int from, int to) {
    for (int i = 0; i < shortestPathLength - 1; i++) {
        if (shortestPath[i] == from && shortestPath[i + 1] == to) {
            return 1;
        }
    }
    return 0;
}

static Vector2 getBezierPoint(Vector2 start, Vector2 end, Vector2 control, float t) {
    float u = 1.0f - t;

    Vector2 p = {
        u * u * start.x + 2 * u * t * control.x + t * t * end.x,
        u * u * start.y + 2 * u * t * control.y + t * t * end.y
    };

    return p;
}

void drawArrowHead(Vector2 start, Vector2 end, Color color) {
    float angle = atan2f(end.y - start.y, end.x - start.x);
    float arrowLength = 18.0f;
    float arrowAngle = 0.55f;

    Vector2 tip = {
        end.x - NODE_RADIUS * cosf(angle),
        end.y - NODE_RADIUS * sinf(angle)
    };

    Vector2 left = {
        tip.x - arrowLength * cosf(angle - arrowAngle),
        tip.y - arrowLength * sinf(angle - arrowAngle)
    };

    Vector2 right = {
        tip.x - arrowLength * cosf(angle + arrowAngle),
        tip.y - arrowLength * sinf(angle + arrowAngle)
    };

    DrawLineEx(tip, left, 4, color);
    DrawLineEx(tip, right, 4, color);
}

void drawCurvedLine(Vector2 start, Vector2 end, Vector2 control, Color edgeColor, Color glowColor, Color arrowColor) {
    Vector2 previous = start;
    Vector2 beforeEnd = start;

    for (int i = 1; i <= 30; i++) {
        float t = i / 30.0f;
        Vector2 point = getBezierPoint(start, end, control, t);

        DrawLineEx(previous, point, 12, Fade(glowColor, 0.35f));
        DrawLineEx(previous, point, 4, edgeColor);

        if (i == 29) beforeEnd = previous;
        previous = point;
    }

    drawArrowHead(beforeEnd, end, arrowColor);
}

void drawEdges(Graph *graph) {
    if (graph == NULL) return;

    for (int i = 0; i < graph->numNodes; i++) {
        Edge *current = graph->adjList[i];

        while (current != NULL) {
            int from = i;
            int to = current->to;

            Vector2 start = nodesLayout[from].position;
            Vector2 end = nodesLayout[to].position;

            Color edgeColor  = EDGE_COLOR;
            Color glowColor  = GLOW_COLOR;
            Color arrowColor = ARROW_COLOR;

            if (from == 2 && to == 5) {
                Vector2 control = {(start.x + end.x) / 2, start.y - 100};
                drawCurvedLine(start, end, control, edgeColor, glowColor, arrowColor);
            }
            else if (from == 1 && to == 3) {
                Vector2 control = {(start.x + end.x) / 2, start.y + 120};
                drawCurvedLine(start, end, control, edgeColor, glowColor, arrowColor);
            }
            else {
                DrawLineEx(start, end, 12, Fade(glowColor, 0.35f));
                DrawLineEx(start, end, 4, edgeColor);
                drawArrowHead(start, end, arrowColor);
            }

            char weightText[10];
            sprintf(weightText, "%d", current->weight);

            float midX;
            float midY;

            if (from == 2 && to == 5) {
                Vector2 control = {(start.x + end.x) / 2, start.y - 100};
                Vector2 p = getBezierPoint(start, end, control, 0.5f);

                midX = p.x;
                midY = p.y - 25;
            }
            else if (from == 1 && to == 3) {
                Vector2 control = {(start.x + end.x) / 2, start.y + 120};
                Vector2 p = getBezierPoint(start, end, control, 0.5f);

                midX = p.x;
                midY = p.y - 1;
            }
            else {
                midX = (start.x + end.x) / 2;
                midY = (start.y + end.y) / 2 - 8;
            }

            DrawText(weightText, (int)midX + 1, (int)midY + 1, 20, RAYWHITE);
            DrawText(weightText, (int)midX, (int)midY, 20, BLACK);

            current = current->next;
        }
    }
}

void DrawStaticGraph(void) {
    const char *title = "DELIVERY SYSTEM VISUALIZATION";
    int titleSize = 32;
    int titleX = GetScreenWidth() / 2 - MeasureText(title, titleSize) / 2;
    int titleY = 40;

    DrawText(title, titleX + 3, titleY + 3, titleSize, TEXT_SHADOW);
    DrawText(title, titleX, titleY, titleSize, TITLE_COLOR);

    for (int i = 0; i < NODE_COUNT; i++) {
        DrawCircleV(nodesLayout[i].position, NODE_RADIUS + 9, Fade(nodesLayout[i].color, 0.22f));
        DrawCircleV(nodesLayout[i].position, NODE_RADIUS + 4, NODE_BORDER);
        DrawCircleV(nodesLayout[i].position, NODE_RADIUS, nodesLayout[i].color);
        DrawCircleLinesV(nodesLayout[i].position, NODE_RADIUS, RAYWHITE);

        char idText[4];
        sprintf(idText, "%d", nodesLayout[i].id);

        int idSize = 24;
        int idX = (int)(nodesLayout[i].position.x - MeasureText(idText, idSize) / 2);
        int idY = (int)(nodesLayout[i].position.y - 12);

        DrawText(idText, idX + 1, idY + 1, idSize, TEXT_SHADOW);
        DrawText(idText, idX, idY, idSize, BLACK);

        int nameSize = 18;
        int nameX = (int)(nodesLayout[i].position.x - MeasureText(nodesLayout[i].name, nameSize) / 2);
        int nameY = (int)(nodesLayout[i].position.y + 42);

        DrawText(nodesLayout[i].name, nameX + 1, nameY + 1, nameSize, TEXT_SHADOW);
        DrawText(nodesLayout[i].name, nameX, nameY, nameSize, RAYWHITE);
    }
}

Vector2 GetNodePosition(int nodeIndex) {
    if (nodeIndex >= 0 && nodeIndex < NODE_COUNT) {
        return nodesLayout[nodeIndex].position;
    }

    return (Vector2){0.0f, 0.0f};
}


