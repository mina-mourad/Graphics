#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include "src/GameConfig.h"
#include "src/Renderer2D.h"
#include "src/Input.h"
#include "src/Game.h"

// Shabab El Hara - Defend the Koshk
// DMET 502 Computer Graphics, Project 1 (2D tower defense).
//
// main.cpp only opens the window and runs the loop (same order as the labs:
// poll events, update, draw, swap buffers). The game itself is in src/.
int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    // Modern OpenGL, core profile (spec: OpenGL 3.3+).
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    int screenWidth = 1280;
    int screenHeight = 720;

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Shabab El Hara - Defend the Koshk", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (gladLoadGL(glfwGetProcAddress) == 0)
    {
        std::cerr << "Failed to initialize OpenGL context\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    Renderer2D::Init(screenWidth, screenHeight);
    Input::Init(window);
    Game::Init();

    float lastFrame = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window) && !Game::WantsExit())
    {
        // deltaTime from glfwGetTime, as in the labs
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        Game::Update(deltaTime);

        Renderer2D::BeginFrame();
        Game::Render();

        glfwSwapBuffers(window);

        // Clear this frame's click / key-press events.
        Input::Update();
    }

    Game::Shutdown();
    Renderer2D::Shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
