#include "Input.h"
#include "GameConfig.h"
#include "Renderer2D.h"

namespace
{
    GLFWwindow* g_window = nullptr;

    bool mouseDown[GLFW_MOUSE_BUTTON_LAST + 1] = {};
    bool mousePressed[GLFW_MOUSE_BUTTON_LAST + 1] = {};

    int screenWidth = 1280;
    int screenHeight = 720;

    void mouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int /*mods*/)
    {
        if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;
        if (action == GLFW_PRESS) {
            mouseDown[button] = true;
            // Adapt Lab 3's press/release state: retain a short click even if
            // its release arrives during the same glfwPollEvents call.
            mousePressed[button] = true;
        }
        else if (action == GLFW_RELEASE) mouseDown[button] = false;
    }

    void framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
    {
        screenWidth = width;
        screenHeight = height;
        Renderer2D::OnResize(width, height);
    }
}

void Input::Init(GLFWwindow* window)
{
    g_window = window;
    glfwGetFramebufferSize(g_window, &screenWidth, &screenHeight);
    Renderer2D::OnResize(screenWidth, screenHeight);
    glfwSetMouseButtonCallback(g_window, mouseButtonCallback);
    glfwSetFramebufferSizeCallback(g_window, framebufferSizeCallback);
}

void Input::Update()
{
    for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; ++i)
        mousePressed[i] = false;
}

glm::vec2 Input::GetMouseWorldPos()
{
    if (!g_window) return glm::vec2(-1.0f);

    // Lab 3 main.cpp:256-261 normalizes cursor coordinates and flips Y.
    // Project adaptation: normalize in window coordinates, independently of
    // framebuffer pixel scaling. The existing callback still sizes the viewport.
    int windowWidth, windowHeight;
    glfwGetWindowSize(g_window, &windowWidth, &windowHeight);
    if (windowWidth <= 0 || windowHeight <= 0 || screenWidth <= 0 || screenHeight <= 0)
        return glm::vec2(-1.0f);

    double x, y;
    glfwGetCursorPos(g_window, &x, &y);

    // Pixels -> [0,1] -> world units. GLFW's Y grows downward; the world's Y
    // (matching Renderer2D's ortho projection, which puts (0,0) bottom-left)
    // grows upward, so it has to be flipped here.
    float u = static_cast<float>(x) / static_cast<float>(windowWidth);
    float v = static_cast<float>(y) / static_cast<float>(windowHeight);
    return glm::vec2(u * WORLD_WIDTH, (1.0f - v) * WORLD_HEIGHT);
}

bool Input::IsMouseButtonDown(int button)
{
    return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && mouseDown[button];
}

bool Input::IsMouseButtonPressed(int button)
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return mousePressed[button];
}

bool Input::IsKeyDown(int key)
{
    return g_window && glfwGetKey(g_window, key) == GLFW_PRESS;
}
