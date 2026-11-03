// [LAB] pointInTriangle / triSign: Lab 3 (mouse over a triangle).
//       A shelf is a rectangle = 2 triangles, like the square in Lab 3.
//       u_Highlight (yellow mix) for hover / selected shelves: Lab 3.
#include "Shelves.h"
#include "Game.h"
#include "Shapes.h"
#include "Text.h"
#include "Sound.h"
#include "Candies.h"
#include <cmath>

// Lab 3: which side of the line p2-p3 the point p1 is on.
float triSign(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

// Lab 3: the point is inside if it is on the same side of all three edges.
bool pointInTriangle(glm::vec2 pt, glm::vec2 a, glm::vec2 b, glm::vec2 c)
{
    float d1 = triSign(pt, a, b), d2 = triSign(pt, b, c), d3 = triSign(pt, c, a);
    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    return !(hasNeg && hasPos);
}

void UpdateShelves()
{
    glm::vec2 mouse(mouseX, mouseY);
    hoverLane = -1;
    for (int lane = 0; lane < LANES; ++lane)
    {
        float bottom = LANE_Y[lane] - LANE_H / 2, top = LANE_Y[lane] + LANE_H / 2;
        glm::vec2 a(LANE_LEFT, bottom), b(LANE_RIGHT, bottom), c(LANE_RIGHT, top), d(LANE_LEFT, top);
        if (pointInTriangle(mouse, a, b, c) || pointInTriangle(mouse, a, c, d))
            hoverLane = lane;
    }
    // placing a chocolate bar: the click says where on the shelf it stands
    if (placingChocolate) {
        if (mouseClicked && hoverLane != -1) PlaceChocolate(hoverLane, mouseX);
        return;
    }
    // a click toggles the shelf; several shelves can be selected at once
    if (mouseClicked && hoverLane != -1) {
        laneSelected[hoverLane] = !laneSelected[hoverLane];
        PlaySound("lane");
    }
}

static void DrawFridge(float doorOpen)
{
    Rect(1.25f, 0.0f, 0.2f, 1.45f, glm::vec3(0.85f, 0.87f, 0.9f));        // body
    Rect(1.25f, 0.69f, 0.2f, 0.07f, glm::vec3(0.2f, 0.45f, 0.8f));        // blue top light
    // glass door: it gets narrower when it opens (like turning around its hinge)
    float w = 0.18f * std::cos(doorOpen * 1.4f);
    Rect(1.16f + w / 2, 0.0f, w, 1.36f, glm::vec3(0.75f, 0.9f, 0.95f));
    for (int i = 0; i < 4; ++i)                                           // bottles seen through the glass
        for (int k = 0; k < 3; ++k)
            Rect(1.19f + k * 0.06f, -0.5f + i * 0.33f, 0.03f, 0.12f, glm::vec3(0.2f + 0.2f * k, 0.6f, 0.3f));
    Rect(1.16f + w - 0.015f, 0.0f, 0.015f, 0.25f, glm::vec3(0.4f));       // handle
}

static void DrawCandyJar()
{
    // home base: a big glass jar full of sweets, with a red lid and a label
    Rect(-1.22f, -0.02f, 0.22f, 1.3f, glm::vec3(0.75f, 0.9f, 0.95f));      // glass
    glm::vec3 sweets[4] = { {0.95f, 0.3f, 0.5f}, {1.0f, 0.65f, 0.1f}, {0.4f, 0.8f, 0.3f}, {0.5f, 0.4f, 0.9f} };
    for (int i = 0; i < 16; ++i)
        Circle(-1.28f + (i % 3) * 0.06f, -0.6f + (i / 3) * 0.2f + (i % 2) * 0.05f, 0.03f, 0.03f, sweets[i % 4]);
    Rect(-1.22f, 0.68f, 0.26f, 0.08f, glm::vec3(0.85f, 0.15f, 0.15f));     // lid
    Rect(-1.22f, 0.0f, 0.2f, 0.12f, glm::vec3(1.0f, 0.95f, 0.8f));         // label
    Text("JAR", -1.22f, 0.0f, 0.012f, glm::vec3(0.8f, 0.1f, 0.1f));
}

// A bag of chips standing on the floor.
static void DrawChips(float x)
{
    Rect(x, -0.79f, 0.08f, 0.11f, glm::vec3(0.95f, 0.75f, 0.1f));    // bag
    Rect(x, -0.79f, 0.08f, 0.025f, glm::vec3(0.85f, 0.15f, 0.15f));  // red band
}

void DrawStore(float doorOpen)
{
    // back wall with tiles
    Rect(0.0f, 0.0f, 2 * ASPECT, 2.0f, glm::vec3(0.96f, 0.9f, 0.8f));
    for (float y = -0.7f; y < 1.0f; y += 0.15f)
        Rect(0.0f, y, 2 * ASPECT, 0.006f, glm::vec3(0.88f, 0.82f, 0.72f));

    // floor with tiles, and 4 bags of chips on the left
    Rect(0.0f, -0.86f, 2 * ASPECT, 0.28f, glm::vec3(0.7f, 0.7f, 0.72f));
    for (int i = 0; i < 14; ++i)
        Rect(-1.3f + i * 0.2f, -0.86f, 0.1f, 0.28f, glm::vec3(0.62f, 0.62f, 0.65f));
    for (int i = 0; i < 4; ++i)
        DrawChips(-1.15f + i * 0.13f);

    // the three shelves: back panel (lights up when hovered / selected),
    // the wooden board and price tags on its edge
    for (int lane = LANES - 1; lane >= 0; --lane)
    {
        float highlight = 0.0f;
        if (hoverLane == lane) highlight += 0.15f;
        if (laneSelected[lane]) highlight += 0.25f;
        float mid = (LANE_LEFT + LANE_RIGHT) / 2, width = LANE_RIGHT - LANE_LEFT;
        Rect(mid, LANE_Y[lane] + 0.02f, width, LANE_H - 0.04f, glm::vec3(0.9f, 0.83f, 0.72f), 0.0f, highlight);
        float board = FloorY(lane) - 0.025f;
        Rect(mid, board, width, 0.05f, glm::vec3(0.6f, 0.4f, 0.2f), 0.0f, highlight);
        Rect(mid, board - 0.03f, width, 0.012f, glm::vec3(0.4f, 0.25f, 0.12f));
        for (int i = 0; i < 6; ++i)
        {
            float x = LANE_LEFT + 0.15f + i * 0.4f;
            Rect(x + 0.18f, board, 0.08f, 0.035f, glm::vec3(1.0f));                   // price tag
            Rect(x + 0.16f, board, 0.02f, 0.02f, glm::vec3(0.9f, 0.2f, 0.2f));
        }
    }

    DrawFridge(doorOpen);
    DrawCandyJar();
}
