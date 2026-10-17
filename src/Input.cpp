#include "Input.h"
#include "GameConfig.h"
#include "Renderer2D.h"

namespace
{
    GLFWwindow* g_window = nullptr;

    bool mouseDown[GLFW_MOUSE_BUTTON_LAST + 1] = {};
    bool mouseDownLastFrame[GLFW_MOUSE_BUTTON_LAST + 1] = {};

    int screenWidth = 1280;
    int screenHeight = 720;

    void mouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int /*mods*/)
    {
        if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;
        if (action == GLFW_PRESS) mouseDown[button] = true;
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
    glfwSetMouseButtonCallback(g_window, mouseButtonCallback);
    glfwSetFramebufferSizeCallback(g_window, framebufferSizeCallback);
}

void Input::Update()
{
    for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; ++i)
        mouseDownLastFrame[i] = mouseDown[i];
}

glm::vec2 Input::GetMouseWorldPos()
{
    double x, y;
    glfwGetCursorPos(g_window, &x, &y);

    // Pixels -> [0,1] -> world units. GLFW's Y grows downward; the world's Y
    // (matching Renderer2D's ortho projection, which puts (0,0) bottom-left)
    // grows upward, so it has to be flipped here.
    float u = static_cast<float>(x) / static_cast<float>(screenWidth);
    float v = static_cast<float>(y) / static_cast<float>(screenHeight);
    return glm::vec2(u * WORLD_WIDTH, (1.0f - v) * WORLD_HEIGHT);
}

bool Input::IsMouseButtonDown(int button)
{
    return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && mouseDown[button];
}

bool Input::IsMouseButtonPressed(int button)
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return mouseDown[button] && !mouseDownLastFrame[button];
}

bool Input::IsKeyDown(int key)
{
    return g_window && glfwGetKey(g_window, key) == GLFW_PRESS;
}
