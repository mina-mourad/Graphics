#pragma once
// Block letters made of small squares (5 x 7 "pixels" per letter).
#include <string>
#include <glm/glm.hpp>

// Draws text centered on (x, y). `size` = size of one letter pixel.
void Text(const std::string& text, float x, float y, float size, glm::vec3 color);
