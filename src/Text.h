#pragma once
#include <glm/glm.hpp>
#include <string>

// Tiny bitmap-font text renderer built on Renderer2D quads (5x7 pixel glyphs).
// Supports A-Z (lower case is drawn as upper case), 0-9 and common punctuation.
//
// `px` is the size of ONE font pixel in world units, so a glyph is 5*px wide
// and 7*px tall, and the advance between letters is 6*px.
namespace Text
{
    enum class Align { Left, Center, Right };

    float Width(const std::string& s, float px);

    // `pos` is the bottom edge of the text; its x depends on `align`.
    void Draw(const std::string& s, glm::vec2 pos, float px, glm::vec3 color,
              Align align = Align::Left, float alpha = 1.0f);

    // Draw with a dark drop shadow so it stays readable on any background.
    void DrawShadow(const std::string& s, glm::vec2 pos, float px, glm::vec3 color,
                    Align align = Align::Left, float alpha = 1.0f);
}
