#pragma once
// Person 1 (window, drawing and text).
// Drawing with the lab shader: a square, a triangle and a circle, moved with
// glm::translate / rotate / scale. Everything in the game is made of these.
// no lab code in this file (only declarations; the code is in the .cpp file)

#include <glm/glm.hpp>

void InitShapes();

// x, y = center, w, h = size, angle in radians.
void Rect(float x, float y, float w, float h, glm::vec3 color, float angle = 0.0f, float highlight = 0.0f);
void Tri(float x, float y, float w, float h, glm::vec3 color, float angle = 0.0f);
void Circle(float x, float y, float rx, float ry, glm::vec3 color);
// Lines (GL_LINES) stored in the VBO from `start`, placed at (x, y) and scaled by size.
// Used for the letters (see Text.cpp).
void Lines(int start, int count, float x, float y, float size, glm::vec3 color);

// Effects for everything drawn after them (0 = off, 1 = full).
extern float redness;   // hurt characters turn red (sent to u_Red in the shader)
extern float grayness;  // locked cards are gray (sent to u_Gray in the shader)

// Moves / scales a whole character as one (like the transform in Lab 2).
// Normally glm::mat4(1.0f) = "no change"; DrawCandy sets it and resets it.
extern glm::mat4 model;
