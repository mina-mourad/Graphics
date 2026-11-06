#include "Hud.h"
#include "Game.h"
#include "Shapes.h"
#include "Text.h"
#include "Candies.h"
#include <GLFW/glfw3.h>
#include <string>
#include <cmath>

bool pointInTriangle(glm::vec2 pt, glm::vec2 a, glm::vec2 b, glm::vec2 c);

bool InBox(float px, float py, float x, float y, float w, float h)
{
    glm::vec2 p(px, py), a(x - w / 2, y - h / 2), b(x + w / 2, y - h / 2), c(x + w / 2, y + h / 2), d(x - w / 2, y + h / 2);
    return pointInTriangle(p, a, b, c) || pointInTriangle(p, a, c, d);
}

const float CardY = 0.85f, CardW = 0.25f, CardH = 0.27f;

bool CardUnlocked(CandyType type)
{
    if (sugar < CandyCost[type]) 
    {
        return false;
    }
    if (type == Lollipop) 
    {
        return true;
    }
    return sugarCollected >= CandyCost[type];
}

static float CardX(int i) { 
    return -1.15f + i * 0.29f; 
}

void UpdateHud()
{
    for (int i = 0; i < 3; ++i)
    {
        bool unlocked = CardUnlocked((CandyType)i);
        bool clicked = mouseClicked && InBox(mouseX, mouseY, CardX(i), CardY, CardW, CardH);
        if (unlocked && clicked) {
            if (i == Chocolate) 
            {
                placingChocolate = !placingChocolate;
            }
            else 
            {
                PlaceCandies((CandyType)i);
            }
        }
    }
}

void SugarCube(float x, float y, float size)
{
    Rect(x, y, size, size, glm::vec3(1.0f));
}

void DrawCubes()
{
    for (int i = 0; i < cubes.size(); i++)
    {
        Cube& c = cubes[i];
        SugarCube(c.x,FloorY(c.lane) + 0.035f + 0.01f * std::sin(gameTime * 4.0f),0.06f);
    }
}

void DrawHealth(float x, float y, int hp, int maxHp, bool isCandy)
{
    float w = 0.022f;
    Rect(x, y, maxHp * w + 0.01f, 0.03f, glm::vec3(0.15f));
    for (int i = 0; i < maxHp; ++i)
    {
        glm::vec3 color;
        if (i < hp) {
            if (isCandy) {
                color = glm::vec3(0.3f, 0.85f, 0.3f);
            }
            else {
                color = glm::vec3(0.95f, 0.3f, 0.2f);
            }
        }
        else {
            color = glm::vec3(0.35f);
        }
        Rect(x - maxHp * w / 2 + (i + 0.5f) * w, y, w - 0.005f, 0.018f, color);
    }
}

void DrawHud()
{
    Rect(0.0f, 0.86f, 2 * Aspect, 0.28f, glm::vec3(0.45f, 0.25f, 0.35f));

    for (int i = 0; i < 3; ++i)
    {
        bool unlocked = CardUnlocked((CandyType)i);
        bool hover = unlocked && InBox(mouseX, mouseY, CardX(i), CardY, CardW, CardH);
        if (unlocked) {
            grayness = 0.0f;
        }
        else {
            grayness = 1.0f;
        }
        float highlight;
        if (hover) {
            highlight = 0.4f;
        }
        else {
            highlight = 0.0f;
        }
        Rect(CardX(i), CardY, CardW, CardH, glm::vec3(1.0f, 0.93f, 0.8f), 0.0f, highlight);
        DrawCandy((CandyType)i, CardX(i), CardY - 0.07f, 0.55f, 0.0f, 1.0f);
        SugarCube(CardX(i) - 0.07f, CardY - 0.1f, 0.035f);
        Text(std::to_string(CandyCost[i]), CardX(i) + 0.04f, CardY - 0.1f, 0.009f, glm::vec3(0.3f, 0.15f, 0.1f));
        grayness = 0.0f;
    }

    Text("WAVE " + std::to_string(wave) + "/3", 0.2f, 0.86f, 0.014f, glm::vec3(1.0f));
    SugarCube(0.9f, 0.86f, 0.09f);
    Text(std::to_string(sugar), 1.1f, 0.86f, 0.016f, glm::vec3(1.0f));

    if (placingChocolate)
        Text("CLICK A SHELF TO PUT THE CHOCOLATE - CARD AGAIN: CANCEL", 0.0f, 0.66f, 0.0075f, glm::vec3(0.5f, 0.2f, 0.1f));
}
