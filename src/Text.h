#pragma once
// Person 1 (window, drawing and text).
// Letters are line drawings (GL_LINES), digits look like a digital clock (7 rectangles).
// no lab code in this file (only declarations; the code is in the .cpp file)
#include <string>
#include <vector>
#include <glm/glm.hpp>

// Draws text centered on (x, y). `size` = one step of the letter grid
// (a letter is 4 x 6 steps, plus a gap).
void Text(const std::string& text, float x, float y, float size, glm::vec3 color);

// Adds the lines of every letter to the shared vertex list (called once by InitShapes).
void AddLetterLines(std::vector<float>& vertices);
