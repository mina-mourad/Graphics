
#include <glad/gl.h>
#undef GLAD_GL_IMPLEMENTATION
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include "src/GameConfig.h"
#include "src/Renderer2D.h"
#include "src/Input.h"
#include "src/EnemySystem.h"
#include "src/MainMenu.h"
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

// Person 1 lane interactions: lab algorithms plus game-specific state.
namespace
{
    // Original algorithms from lab 3/main.cpp:118-133.
    float triSign(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
    {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    }

    bool pointInTriangle(glm::vec2 pt, glm::vec2 a, glm::vec2 b, glm::vec2 c)
    {
        float d1 = triSign(pt, a, b);
        float d2 = triSign(pt, b, c);
        float d3 = triSign(pt, c, a);

        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(hasNeg && hasPos);
    }

    struct LaneInteraction
    {
        int hoveredLane = -1;
        int selectedLane = -1;
        bool previousW = false;
        bool previousS = false;

        void Update()
        {
            const glm::vec2 mouse = Input::GetMouseWorldPos();
            hoveredLane = -1;
            for (int lane = 0; lane < NUM_LANES; ++lane) {
                // Adapt Lab 3 main.cpp:247-264 to the rendered rectangle's
                // world-space corners and its two triangles.
                const float left = (WORLD_WIDTH - LANE_WIDTH) * 0.5f;
                const float right = left + LANE_WIDTH;
                const float bottom = LANE_Y[lane] - LANE_HEIGHT * 0.5f;
                const float top = LANE_Y[lane] + LANE_HEIGHT * 0.5f;
                const glm::vec2 a(left, bottom), b(right, bottom), c(right, top), d(left, top);
                if (pointInTriangle(mouse, a, b, c) || pointInTriangle(mouse, a, c, d)) {
                    hoveredLane = lane;
                    break;
                }
            }

            // Lab 3 processInput():75-84 and Lab4Camera.cpp:384-407 poll keys.
            // Adapt the team's previous/current mouse-state idea to W/S so a
            // held key moves only once. Record state even at lane boundaries.
            const bool wDown = Input::IsKeyDown(GLFW_KEY_W);
            const bool sDown = Input::IsKeyDown(GLFW_KEY_S);
            const bool wPressed = wDown && !previousW;
            const bool sPressed = sDown && !previousS;
            previousW = wDown;
            previousS = sDown;

            // Game-specific selection: mouse wins over keyboard in the same
            // frame; outside clicks preserve selection; both keys suppress moves.
            if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && hoveredLane != -1) {
                selectedLane = selectedLane == hoveredLane ? -1 : hoveredLane;
            }
            else if (!(wDown && sDown) && (wPressed || sPressed)) {
                if (selectedLane == -1) selectedLane = NUM_LANES / 2;
                else if (wPressed && selectedLane < NUM_LANES - 1) ++selectedLane;
                else if (sPressed && selectedLane > 0) --selectedLane;
            }
        }

        float Highlight(int lane) const
        {
            // Renderer2D already implements Lab 3's mix() shader. Persistent
            // selection takes priority without erasing a separate hover.
            if (lane == selectedLane) return 1.0f;
            return lane == hoveredLane ? 0.45f : 0.0f;
        }
    };
}
// End Person 1 lane interactions.

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
    EnemySystem::Init();
    Input::Init(window);

    float lastFrame = 0.0f;
    LaneInteraction lanes;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        Input::Update();
        glfwPollEvents();
        lanes.Update();

        // TODO Person 2: UpdatePlayerSystems(deltaTime);
        // TODO Person 3: UpdateEnemySystems(deltaTime);
        EnemySystem::Update(deltaTime);
        // TODO Person 4: UpdateUI(deltaTime);

        Renderer2D::BeginFrame();

        // Marvel lanes: keep the reference's orange color, 90% world width,
        // and 18% world height, centered at the team's shared lane positions.
        // Draw the background first so gameplay and UI render above it.
        for (int lane = 0; lane < NUM_LANES; ++lane) {
            Renderer2D::DrawQuad(
                glm::vec2(WORLD_WIDTH / 2.0f, LANE_Y[lane]),
                glm::vec2(LANE_WIDTH, LANE_HEIGHT),
                glm::vec3(1.0f, 0.5f, 0.2f),
                lanes.Highlight(lane)
            );
        }

       

        // TODO Person 3: RenderEnemies();
        EnemySystem::Render();
        // TODO Person 2: RenderTowers();
        // TODO Person 4: RenderUI();

        glfwSwapBuffers(window);
    }
    EnemySystem::Shutdown();

    Renderer2D::Shutdown();
    glfwTerminate();
    return 0;
}
