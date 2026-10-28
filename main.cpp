#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#undef GLAD_GL_IMPLEMENTATION
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include "src/GameConfig.h"
#include "src/Renderer2D.h"
#include "src/Input.h"
#include "src/Game.h"
#include "src/Audio.h"

int main()
{
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    // Modern OpenGL: a 3.3 core-profile context (the spec asks for 3.3 or newer).
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    int screenWidth = 1280;
    int screenHeight = 720;
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Guardians of the Sun Disc", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        std::cerr << "Failed to initialize OpenGL context\n";
        return -1;
    }

    Renderer2D::Init(screenWidth, screenHeight);
    Input::Init(window);
    Game::Init();

    float lastFrame = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        Input::Update();
        glfwPollEvents();

        Game::Update(deltaTime);
        if (Game::WantsExit())
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        Renderer2D::BeginFrame();
        Game::Render();

        glfwSwapBuffers(window);
    }

    Audio::Shutdown();
    Renderer2D::Shutdown();
    glfwTerminate();
    return 0;
}
