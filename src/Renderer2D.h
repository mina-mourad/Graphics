#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>

// Minimal 2D renderer.
//
// Everything is drawn in WORLD coordinates (see GameConfig.h) with one shader
// program, the same way the labs do it:
//   * one VAO/VBO per reusable shape (unit quad, unit circle built once with a
//     GL_TRIANGLE_FAN like Lab 3's circle, and one buffer for free polygons),
//   * a model matrix built with glm::translate / glm::rotate / glm::scale
//     (Lab 2 / Lab 3 u_Transform),
//   * glm::ortho for the projection (Lab 4 camera),
//   * the Lab 3 u_Highlight idea -- mix() the color toward another color --
//     reused for damage (red), locked cards (gray) and fades (black).
namespace Renderer2D
{
    // Call once, after the GL context exists (after gladLoadGL succeeds).
    void Init(int screenWidth, int screenHeight);

    // Call once per frame, before any draw calls, to clear the screen.
    void BeginFrame();

    // Rectangle centered at `pos`, `size` wide/tall. `highlight` mixes the
    // color toward yellow (0 = base color, 1 = fully yellow), as in Lab 3.
    void DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec3 color, float highlight = 0.0f);

    // Rectangle rotated by `angle` radians around its center (glm::rotate).
    void DrawQuadRotated(glm::vec2 pos, glm::vec2 size, float angle, glm::vec3 color);

    // A thick straight line from a to b (a rotated quad).
    void DrawLine(glm::vec2 a, glm::vec2 b, float width, glm::vec3 color);

    // Circle / ellipse: the unit circle scaled by the radius.
    void DrawCircle(glm::vec2 pos, float radius, glm::vec3 color);
    void DrawEllipse(glm::vec2 pos, glm::vec2 radii, glm::vec3 color);

    // Filled polygon (convex or star-shaped around its first point), drawn as
    // a GL_TRIANGLE_FAN. Points are in world coordinates.
    void DrawPolygon(const glm::vec2* vertices, int count, glm::vec3 color);
    void DrawTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec3 color);

    // Text made of small quads (5x7 pixel font). `scale` 0.16 is a normal
    // button label. DrawText centers the text on `centerPos`.
    void DrawText(const std::string& text, glm::vec2 centerPos, float scale, glm::vec3 color, float highlight = 0.0f);
    enum class Align { Left, Center, Right };
    void DrawTextAligned(const std::string& text, glm::vec2 pos, float scale, glm::vec3 color, Align align);
    // Text with a dark copy behind it, so it stays readable on any background.
    void DrawTextShadow(const std::string& text, glm::vec2 pos, float scale, glm::vec3 color, Align align = Align::Center);
    float TextWidth(const std::string& text, float scale);

    // Effects applied to everything drawn until they are changed again.
    // They all use the same mix() as u_Highlight in Lab 3.
    void SetHighlight(float amount);   // toward yellow (whole lane highlight)
    void SetDamage(float amount);      // toward red (characters getting hurt)
    void SetGray(float amount);        // toward grayscale (locked cards)
    void SetFade(float amount);        // toward black (fades, dim behind menus)
    void ResetEffects();

    // Call if the window is resized, so the viewport keeps matching it.
    void OnResize(int screenWidth, int screenHeight);

    // Call once at shutdown to free the shader/VAOs/VBOs.
    void Shutdown();
}
