#pragma once
// Person 4 (store, game flow and text).
// Block letters made of small squares (5 x 7 "pixels" per letter).
// no lab code in this file (only declarations; the code is in the .cpp file)
#include <string>
#include <glm/glm.hpp>

// Draws text centered on (x, y). `size` = size of one letter pixel.
void Text(const std::string& text, float x, float y, float size, glm::vec3 color);
