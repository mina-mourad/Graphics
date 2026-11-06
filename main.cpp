#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "src/Game.h"
#include "src/Shapes.h"
#include "src/Sound.h"

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key >= 0 && key < 512)
        keyPressed[key] = true;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        mouseClicked = true;
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    mouseX = ((float)xpos / (WindowW / 2.0f) - 1.0f) * Aspect;
    mouseY = 1.0f - (float)ypos / (WindowH / 2.0f);
}

int main()
{
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(WindowW, WindowH, "Soda vs Candy", nullptr, nullptr);
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

        mouseClicked = false;
        for (int i = 0; i < 512; ++i) keyPressed[i] = false;
    }

    CloseSound();
    glfwTerminate();
    return 0;
}
