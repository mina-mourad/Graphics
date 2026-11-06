#pragma once
// Person 1 (window, drawing and text).
// The letters come from the library stb_easy_font, drawn as triangles (see Text.cpp).
// no lab code in this file (only declarations; the code is in the .cpp file)
#include <string>
#include <vector>
#include <glm/glm.hpp>

// Draws text centered on (x, y). `size` = one step of a letter
// (a capital letter is 4 x 7 steps, plus a gap).
void Text(const std::string& text, float x, float y, float size, glm::vec3 color);

// Adds the triangles of every letter to the shared vertex list (called once by InitShapes).
void AddLetters(std::vector<float>& vertices);
