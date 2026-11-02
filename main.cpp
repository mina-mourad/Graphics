// Soda vs Candy - DMET 502 Project 1 (simple, lab-level version)
//
// [LAB] This file follows the Lab 2 template and Lab 3 code:
//   glfwInit / window hints / gladLoad / glViewport  -> Lab 2 template
//   keyCallback, mouseButtonCallback (GLFW_PRESS)    -> Lab 3
//   cursor position -> normalized coordinates        -> Lab 3 (cursor_position_callback)
//   deltaTime with glfwGetTime                       -> Lab 2 template
//   render loop: clear, draw, swap, poll             -> Lab 2 / Lab 3
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "src/Game.h"
#include "src/Shapes.h"
#include "src/Sound.h"

// Keyboard: remember keys pressed this frame (Lab 3 keyCallback).
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key >= 0 && key < 512)
        keyPressed[key] = true;
}

// Mouse button: a left click (Lab 3 mouseButtonCallback).
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        mouseClicked = true;
}

// Mouse position: pixels -> -1..1 (Lab 3), then x times ASPECT (see Shapes.cpp).
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    mouseX = ((float)xpos / (WINDOW_W / 2.0f) - 1.0f) * ASPECT;
    mouseY = 1.0f - (float)ypos / (WINDOW_H / 2.0f);
}

int main()
{
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // keeps the mouse math simple

    GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Soda vs Candy", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);
    glfwSwapInterval(1);

    InitShapes();
    InitSound();
    ResetGame();

    float lastFrame = (float)glfwGetTime();
    while (!glfwWindowShouldClose(window) && !GameWantsExit())
    {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        GameUpdate(deltaTime);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        GameDraw();
        glfwSwapBuffers(window);

        // the click / key presses of this frame are used up
        mouseClicked = false;
        for (int i = 0; i < 512; ++i) keyPressed[i] = false;
    }

    CloseSound();
    glfwTerminate();
    return 0;
}
