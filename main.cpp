// Soda vs Candy - DMET 502 Project 1 (simple, lab-level version)
// Person 1 (window, drawing and menus).
//
// [LAB] This file follows the Lab 2 template and Lab 3 code.
// Below every part that uses lab code there is a "LAB CODE" comment with the
// lab lines exactly as they are written in the lab files:
//   Lab 1 Notes.pdf, Lab 2 Template/main.cpp, Lab 3 Notes/main.cpp, Lab 3 Notes/Lab3ExtraNotes.cpp
// Parts with no lab code say "no lab code".
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "src/Game.h"
#include "src/Shapes.h"
#include "src/Sound.h"
// LAB CODE (Lab 2 Template/main.cpp):
//   #include <glad/glad.h>
//   #include <GLFW/glfw3.h>
//   #include <iostream>
// (our GLAD version names its header glad/gl.h)

// Keyboard: remember keys pressed this frame.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key >= 0 && key < 512)
        keyPressed[key] = true;
}
// LAB CODE (Lab 3 Notes/main.cpp):
//   void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
//   {
//       //GLFW_PRESS: Once per click, you need to reclick to do (do for things you don't want spammed)
//       if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//           glfwSetWindowShouldClose(window, true);

// Mouse button: a left click.
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        mouseClicked = true;
}
// LAB CODE (Lab 3 Notes/Lab3ExtraNotes.cpp):
//   void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
//   {
//       if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
//           clicked = true;

// Mouse position: pixels -> -1..1, then x times ASPECT (see Shapes.cpp).
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    mouseX = ((float)xpos / (WINDOW_W / 2.0f) - 1.0f) * ASPECT;
    mouseY = 1.0f - (float)ypos / (WINDOW_H / 2.0f);
}
// LAB CODE (Lab 3 Notes/Lab3ExtraNotes.cpp):
//   void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
//   {
//       mouseX = (float)xpos / 400.0f - 1.0f;
//       mouseY = 1.0f - (float)ypos / 300.0f;
//   }
// (400 and 300 are half of the lab's 800 x 600 window; ours is WINDOW_W / 2 and WINDOW_H / 2)

int main()
{
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // keeps the mouse math simple (no lab code)
    // LAB CODE (Lab 2 Template/main.cpp):
    //   if (!glfwInit()) return -1;
    //   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    //   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // (the project asks for OpenGL 3.3+, so we ask for 3.3)

    GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Soda vs Candy", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // LAB CODE (Lab 2 Template/main.cpp):
    //   GLFWwindow* window = glfwCreateWindow(g_screenWidth, g_screenHeight, "Triangle", nullptr, nullptr);
    //   if (!window) {
    //       std::cerr << "Failed to create window\n";
    //       glfwTerminate();
    //       return -1;
    //   }
    //   glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    // LAB CODE (Lab 3 Notes/Lab3ExtraNotes.cpp):
    //   glfwSetKeyCallback(window, key_callback);
    //   glfwSetMouseButtonCallback(window, mouseButtonCallback);
    //   glfwSetCursorPosCallback(window, cursor_position_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }
    // LAB CODE (Lab 2 Template/main.cpp):
    //   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    //       std::cerr << "Failed to init GLAD\n";
    //       return -1;
    //   }
    // (our GLAD version calls the same function gladLoadGL)

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);
    glfwSwapInterval(1);
    // LAB CODE (Lab 2 Template/main.cpp):
    //   glfwSwapInterval(1);
    //   glViewport(0, 0, g_screenWidth, g_screenHeight);
    // (glfwGetFramebufferSize: no lab code - it gives the real pixel size on high-DPI screens)

    InitShapes();   // the lab shader + VBO (see Shapes.cpp)
    InitSound();    // no lab code
    ResetGame();    // no lab code

    float lastFrame = (float)glfwGetTime();
    while (!glfwWindowShouldClose(window) && !GameWantsExit())
    {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // LAB CODE (Lab 2 Template/main.cpp):
        //   float deltaTime = 0.0f;
        //   float lastFrame = 0.0f;
        //   while (!glfwWindowShouldClose(window))
        //   {
        //       float currentFrame = (float)glfwGetTime();
        //       deltaTime = currentFrame - lastFrame;
        //       lastFrame = currentFrame;

        glfwPollEvents();
        GameUpdate(deltaTime);   // no lab code (Game.cpp)

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        GameDraw();              // no lab code (Game.cpp)
        glfwSwapBuffers(window);
        // LAB CODE (Lab 2 Template/main.cpp):
        //   glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        //   glClear(GL_COLOR_BUFFER_BIT);
        //   glfwSwapBuffers(window);
        //   glfwPollEvents();
        // LAB CODE (Lab 1 Notes.pdf):
        //   while (!glfwWindowShouldClose(window)) {
        //       glDrawArrays(GL_TRIANGLES, 0, 3);
        //       glfwSwapBuffers(window);
        //       glfwPollEvents();
        //   }
        // (GameDraw() does all the glDrawArrays / glDrawElements calls through Shapes.cpp)

        // the click / key presses of this frame are used up (no lab code)
        mouseClicked = false;
        for (int i = 0; i < 512; ++i) keyPressed[i] = false;
    }

    CloseSound();   // no lab code
    glfwTerminate();
    return 0;
    // LAB CODE (Lab 2 Template/main.cpp):
    //   glfwTerminate();
    //   return 0;
}
