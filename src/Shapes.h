#pragma once
// Person 1 (window, drawing and the store).
// Drawing with the lab shader: a square, a triangle and a circle, moved with
// glm::translate / rotate / scale. Everything in the game is made of these.
// no lab code in this file (only declarations; the code is in the .cpp file)

#include <glm/glm.hpp>

void InitShapes();

// x, y = center, w, h = size, angle in radians.
void Rect(float x, float y, float w, float h, glm::vec3 color, float angle = 0.0f, float highlight = 0.0f);
void Tri(float x, float y, float w, float h, glm::vec3 color, float angle = 0.0f);
void Circle(float x, float y, float rx, float ry, glm::vec3 color);

// Effects for everything drawn after them (0 = off, 1 = full).
extern float redness;   // hurt characters turn red
extern float grayness;  // locked cards are gray
