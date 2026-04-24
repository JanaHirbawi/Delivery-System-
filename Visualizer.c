#include "Visualizer.h"
#include <stdio.h>

NodeVisual nodesLayout[NODE_COUNT];

void InitGraphLayout(int screenWidth, int screenHeight) {
    float centerY = screenHeight * 0.55f;

    // margins
    float startX = 80.0f;
    float endX = screenWidth - 120.0f;
    float hSpacing = (endX - startX) / 3.0f;

    // middle row
    nodesLayout[1] = (NodeVisual){1, {startX, centerY}, "Customer 1", LIGHTGRAY};
    nodesLayout[2] = (NodeVisual){2, {startX + hSpacing, centerY}, "Hub Alpha", LIGHTGRAY};
    nodesLayout[3] = (NodeVisual){3, {startX + 2 * hSpacing, centerY}, "Customer B", LIGHTGRAY};
    nodesLayout[5] = (NodeVisual){5, {startX + 3 * hSpacing, centerY}, "Destination Hub", DARKGRAY};

    // top node
    nodesLayout[0] = (NodeVisual){0, {nodesLayout[1].position.x + hSpacing / 2, centerY - 160}, "Warehouse", SKYBLUE};

    // bottom node
    nodesLayout[4] = (NodeVisual){4, {nodesLayout[3].position.x, centerY + 130}, "Hub Beta", LIGHTGRAY};
}

void DrawStaticGraph(void) {
    const char *title = "DELIVERY SYSTEM VISUALIZATION";
    int titleSize = 32;
    int titleX = GetScreenWidth() / 2 - MeasureText(title, titleSize) / 2;
    int titleY = 40;

    // title shadow
    DrawText(title, titleX + 2, titleY + 2, titleSize, BLACK);
    // title main
    DrawText(title, titleX, titleY, titleSize, WHITE);

    for (int i = 0; i < NODE_COUNT; i++) {
        // draw node
        DrawCircleV(nodesLayout[i].position, NODE_RADIUS, nodesLayout[i].color);
        DrawCircleLinesV(nodesLayout[i].position, NODE_RADIUS, WHITE);

        // node id
        char idText[4];
        sprintf(idText, "%d", nodesLayout[i].id);

        int idSize = 24;
        int idX = (int)(nodesLayout[i].position.x - MeasureText(idText, idSize) / 2);
        int idY = (int)(nodesLayout[i].position.y - 12);

        // id shadow
        DrawText(idText, idX + 1, idY + 1, idSize, DARKGRAY);
        // id main
        DrawText(idText, idX, idY, idSize, BLACK);

        // node name
        int nameSize = 18;
        int nameX = (int)(nodesLayout[i].position.x - MeasureText(nodesLayout[i].name, nameSize) / 2);
        int nameY = (int)(nodesLayout[i].position.y + 42);

        // name shadow
        DrawText(nodesLayout[i].name, nameX + 1, nameY + 1, nameSize, BLACK);
        // name main
        DrawText(nodesLayout[i].name, nameX, nameY, nameSize, WHITE);
    }
}
