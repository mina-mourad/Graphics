#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include "src/GameConfig.h"
#include "src/Renderer2D.h"
#include "src/Input.h"
#include "src/MainMenu.h"


// GLAD 2.0.8 puts its implementation outside the declaration include guard.
// Include the header again with this macro after project headers have included
// its declarations, so the implementation is emitted exactly once here.
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>


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

        MainMenu::Render(); // here we are using the function (render which is implemented in cpp)
       

        // TODO Person 3: RenderEnemies();
        // TODO Person 2: RenderTowers();
        // TODO Person 4: RenderUI();

        // Sanity-check placeholder: proves the whole pipeline (build -> shader
        // -> renderer -> window) actually works end to end. Delete this once
        // real gameplay is drawing something of its own.
        

        glfwSwapBuffers(window);
    }

    Renderer2D::Shutdown();
    glfwTerminate();
    return 0;
}
