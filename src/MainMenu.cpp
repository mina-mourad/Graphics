#include "MainMenu.h"
#include "Renderer2D.h"
#include "Input.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <cstddef>

// This is from the lab where we test if a point is in a triangle.
static float triSign(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y)
         - (p2.x - p3.x) * (p1.y - p3.y);
}

// This is also from the lab where we check if we are in a triangle.
static bool pointInTriangle(
    glm::vec2 pt,
    glm::vec2 a,
    glm::vec2 b,
    glm::vec2 c)
{
    float d1 = triSign(pt, a, b);
    float d2 = triSign(pt, b, c);
    float d3 = triSign(pt, c, a);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(hasNeg && hasPos);
}

namespace MainMenu
{
    void Init()
    {
        // Nothing to initialize yet.
    }

    void Render()
    {
        glm::vec2 mousePos = Input::GetMouseWorldPos();

        // Start Game button

        glm::vec2 startA(6.0f, 5.5f);
        glm::vec2 startB(10.0f, 5.5f);
        glm::vec2 startC(10.0f, 6.5f);
        glm::vec2 startD(6.0f, 6.5f);

        bool startHover =
            pointInTriangle(mousePos, startA, startB, startC) ||
            pointInTriangle(mousePos, startA, startC, startD);

        Renderer2D::DrawQuad(
            glm::vec2(8.0f, 6.0f),
            glm::vec2(4.0f, 1.0f),
            glm::vec3(0.93f, 0.11f, 0.14f),
            startHover ? 1.0f : 0.0f
        );

        // Sound Settings button

        glm::vec2 soundA(6.0f, 4.0f);
        glm::vec2 soundB(10.0f, 4.0f);
        glm::vec2 soundC(10.0f, 5.0f);
        glm::vec2 soundD(6.0f, 5.0f);

        bool soundHover =
            pointInTriangle(mousePos, soundA, soundB, soundC) ||
            pointInTriangle(mousePos, soundA, soundC, soundD);

        Renderer2D::DrawQuad(
            glm::vec2(8.0f, 4.5f),
            glm::vec2(4.0f, 1.0f),
            glm::vec3(0.20f, 0.20f, 0.23f),
            soundHover ? 1.0f : 0.0f
        );

        // Exit button

        glm::vec2 exitA(6.0f, 2.5f);
        glm::vec2 exitB(10.0f, 2.5f);
        glm::vec2 exitC(10.0f, 3.5f);
        glm::vec2 exitD(6.0f, 3.5f);

        bool exitHover =
            pointInTriangle(mousePos, exitA, exitB, exitC) ||
            pointInTriangle(mousePos, exitA, exitC, exitD);

        if (exitHover &&
            Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            glfwSetWindowShouldClose(
                glfwGetCurrentContext(),
                GLFW_TRUE
            );
        }

        Renderer2D::DrawQuad(
            glm::vec2(8.0f, 3.0f),
            glm::vec2(4.0f, 1.0f),
            glm::vec3(0.56f, 0.09f, 0.13f),
            exitHover ? 1.0f : 0.0f
        );
    }

    void Shutdown()
    {
        // Nothing to clean up yet.
    }
}
