#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

void Text(const std::string& text, float x, float y, float size, glm::vec3 color);

void AddLetters(std::vector<float>& vertices);
