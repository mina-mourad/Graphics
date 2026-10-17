#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#undef GLAD_GL_IMPLEMENTATION
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include "src/GameConfig.h"
#include "src/Renderer2D.h"
#include "src/Input.h"

int main()
{
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int screenWidth = 1280;
    int screenHeight = 720;
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Tower Defense", nullptr, nullptr);
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

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        Input::Update();
        glfwPollEvents();

        // TODO Person 2: UpdatePlayerSystems(deltaTime);
        // TODO Person 3: UpdateEnemySystems(deltaTime);
        // TODO Person 4: UpdateUI(deltaTime);

        Renderer2D::BeginFrame();

        // Marvel lanes: keep the reference's orange color, 90% world width,
        // and 18% world height, centered at the team's shared lane positions.
        // Draw the background first so gameplay and UI render above it.
        for (int lane = 0; lane < NUM_LANES; ++lane) {
            Renderer2D::DrawQuad(
                glm::vec2(WORLD_WIDTH / 2.0f, LANE_Y[lane]),
                glm::vec2(WORLD_WIDTH * 0.9f, WORLD_HEIGHT * 0.18f),
                glm::vec3(1.0f, 0.5f, 0.2f)
            );
        }

        // TODO Person 3: RenderEnemies();
        // TODO Person 2: RenderTowers();
        // TODO Person 4: RenderUI();

        glfwSwapBuffers(window);
    }

    Renderer2D::Shutdown();
    glfwTerminate();
    return 0;
}
