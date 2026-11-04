// [LAB] Mouse over a card = pointInTriangle on its two triangles (Lab 3),
//       hover color with u_Highlight (Lab 3).
// [NOT LAB] Text() for numbers (block letters made of squares).
// Below every part that uses lab code there is a "LAB CODE" comment with the
// lab lines exactly as they are written in the lab files.
// Parts with no lab code say "no lab code".
// (All drawing uses Rect / Tri / Circle; their lab code is in Shapes.cpp.)
#include "Hud.h"
#include "Game.h"
#include "Shapes.h"
#include "Text.h"
#include "Candies.h"
#include <GLFW/glfw3.h>
#include <string>
#include <cmath>

bool pointInTriangle(glm::vec2 pt, glm::vec2 a, glm::vec2 b, glm::vec2 c);   // in Shelves.cpp

bool InBox(float px, float py, float x, float y, float w, float h)
{
    glm::vec2 p(px, py), a(x - w / 2, y - h / 2), b(x + w / 2, y - h / 2), c(x + w / 2, y + h / 2), d(x - w / 2, y + h / 2);
    return pointInTriangle(p, a, b, c) || pointInTriangle(p, a, c, d);
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   bool hovering = pointInTriangle(cursorNDC, a, b, c);
    // (a box is a rectangle, so we test its 2 triangles)
}

const float CARD_Y = 0.85f, CARD_W = 0.25f, CARD_H = 0.27f;

// The lollipop is unlocked whenever you can pay for it. The bonbon and the
// chocolate also need that much sugar COLLECTED from defeated sodas first, and
// they lock again whenever your sugar falls below their cost (spec 4.2).
bool CardUnlocked(CandyType type)
{
    if (sugar < CANDY_COST[type]) return false;
    if (type == LOLLIPOP) return true;
    return sugarCollected >= CANDY_COST[type];
}
// no lab code

static float CardX(int i) { return -1.15f + i * 0.29f; }
// no lab code

void UpdateHud()
{
    // a card works only when there is enough sugar (unlocked)
    for (int i = 0; i < 3; ++i)
    {
        bool unlocked = CardUnlocked((CandyType)i);
        bool clicked = mouseClicked && InBox(mouseX, mouseY, CardX(i), CARD_Y, CARD_W, CARD_H);
        if (unlocked && (clicked || keyPressed[GLFW_KEY_1 + i])) {
            if (i == CHOCOLATE) placingChocolate = !placingChocolate;   // then click anywhere on a shelf (click the card again to cancel)
            else PlaceCandies((CandyType)i);
        }
        // LAB CODE (Lab 3 Notes/main.cpp):
        //   bool hovering = pointInTriangle(cursorNDC, a, b, c);
        //   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        // (InBox = the pointInTriangle test; keys 1 2 3 are pressed once, saved by keyCallback in main.cpp)
    }
}

void SugarCube(float x, float y, float size)
{
    Rect(x, y, size, size, glm::vec3(1.0f));
    Rect(x + size * 0.15f, y - size * 0.15f, size * 0.7f, size * 0.7f, glm::vec3(0.9f, 0.9f, 0.95f));
    Circle(x - size * 0.2f, y + size * 0.2f, size * 0.08f, size * 0.08f, glm::vec3(0.8f, 0.8f, 0.85f));
}
// no lab code

void DrawCubes()
{
    for (Cube& c : cubes)
        SugarCube(c.x, FloorY(c.lane) + 0.035f + 0.01f * std::sin(gameTime * 4.0f), 0.06f);
    // LAB CODE (Lab 2 Template/main.cpp):
    //   float offsetX = sin(t) * 0.5f;
    // (the cube bobs up and down with sin of the time)
}

void DrawHealth(float x, float y, int hp, int maxHp, bool isCandy)
{
    float w = 0.022f;
    Rect(x, y, maxHp * w + 0.01f, 0.03f, glm::vec3(0.15f));
    for (int i = 0; i < maxHp; ++i)
    {
        glm::vec3 color = i < hp ? (isCandy ? glm::vec3(0.3f, 0.85f, 0.3f) : glm::vec3(0.95f, 0.3f, 0.2f)) : glm::vec3(0.35f);
        Rect(x - maxHp * w / 2 + (i + 0.5f) * w, y, w - 0.005f, 0.018f, color);
    }
}
// no lab code

void DrawHud()
{
    // top bar
    Rect(0.0f, 0.86f, 2 * ASPECT, 0.28f, glm::vec3(0.45f, 0.25f, 0.35f));

    // candy cards: gray and not clickable when locked, highlighted on hover when unlocked
    for (int i = 0; i < 3; ++i)
    {
        bool unlocked = CardUnlocked((CandyType)i);
        bool hover = unlocked && InBox(mouseX, mouseY, CardX(i), CARD_Y, CARD_W, CARD_H);
        grayness = unlocked ? 0.0f : 1.0f;
        Rect(CardX(i), CARD_Y, CARD_W, CARD_H, glm::vec3(1.0f, 0.93f, 0.8f), 0.0f, hover ? 0.4f : 0.0f);
        // LAB CODE (Lab 3 Notes/main.cpp):
        //   bool hovering = pointInTriangle(cursorNDC, a, b, c);
        //   glUniform1f(locHighlight, triangleHighlight ? 1.0f : 0.0f);
        // (InBox = the pointInTriangle test; the highlight goes to u_Highlight inside Rect)
        DrawCandy((CandyType)i, CardX(i), CARD_Y - 0.07f, 0.55f, 0.0f, 1.0f);
        SugarCube(CardX(i) - 0.07f, CARD_Y - 0.1f, 0.035f);
        Text(std::to_string(CANDY_COST[i]), CardX(i) + 0.04f, CARD_Y - 0.1f, 0.009f, glm::vec3(0.3f, 0.15f, 0.1f));
        Text(std::to_string(i + 1), CardX(i) - 0.09f, CARD_Y + 0.1f, 0.007f, glm::vec3(0.3f, 0.15f, 0.1f));
        grayness = 0.0f;
    }

    // wave number and sugar counter
    Text("WAVE " + std::to_string(wave) + "/3", 0.2f, 0.86f, 0.014f, glm::vec3(1.0f));
    SugarCube(0.9f, 0.86f, 0.09f);
    Text(std::to_string(sugar), 1.1f, 0.86f, 0.016f, glm::vec3(1.0f));

    if (placingChocolate)
        Text("CLICK A SHELF TO PUT THE CHOCOLATE - CARD AGAIN: CANCEL", 0.0f, 0.66f, 0.0075f, glm::vec3(0.5f, 0.2f, 0.1f));

    // how to play
    Text("SELECT SHELVES - CARD OR 1 2 3: PLACE - W JUMP - S DUCK - ESC PAUSE",
         0.0f, -0.96f, 0.0055f, glm::vec3(0.2f));
    // no lab code (Text, grayness)
}
