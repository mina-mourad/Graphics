#pragma once

#include <glm/glm.hpp>

void InitShapes();

void Rect(float x, float y, float w, float h, glm::vec3 color, float angle = 0.0f, float highlight = 0.0f);
void Tri(float x, float y, float w, float h, glm::vec3 color, float angle = 0.0f);
void Circle(float x, float y, float rx, float ry, glm::vec3 color);
void Triangles(int start, int count, float x, float y, float size, glm::vec3 color);

extern float redness;
extern float grayness;

extern glm::mat4 model;
