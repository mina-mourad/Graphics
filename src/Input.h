#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Wraps GLFW's callbacks so the rest of the game never has to touch GLFW
// input functions directly.
//
// Call Init() once, right after the window is created. Call Update() once
// per frame, BEFORE polling GLFW events and before reading any state below
// -- it snapshots last frame's state so IsMouseButtonPressed() (edge-
// triggered) works correctly.
namespace Input
{
    void Init(GLFWwindow* window);

    // Call once per frame, before glfwPollEvents() and before any of the
    // queries below.
    void Update();

    // Mouse position converted from screen pixels into the same world-unit
    // coordinate system Renderer2D draws in (see GameConfig.h).
    glm::vec2 GetMouseWorldPos();

    bool IsMouseButtonDown(int button);     // true every frame the button is held (GLFW_MOUSE_BUTTON_LEFT, etc.)
    bool IsMouseButtonPressed(int button);  // true only on the frame it was first pressed
    bool IsKeyDown(int key);                // true every frame the key is held (GLFW_KEY_W, etc.)
}
