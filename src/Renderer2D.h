#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>

// Minimal 2D quad renderer.
//
// Draws axis-aligned colored rectangles in WORLD coordinates (see
// GameConfig.h), using one reusable unit quad and a per-draw model matrix --
// the same create/draw/destroy shape as Lab 4's Mesh struct, and the same
// highlight-uniform shader pattern as the Lab 2/3 hover-highlight demos.
//
// Nobody outside this file should need to call raw OpenGL to put a shape on
// screen. Textured sprites, text, and anything beyond flat-colored rectangles
// are not in here yet -- this is the minimum to unblock everyone else.
namespace Renderer2D
{
    // Call once, after the GL context exists (after gladLoadGL succeeds).
    // Compiles the shader, builds the reusable quad VAO/VBO, and sets up the
    // ortho projection to match GameConfig's WORLD_WIDTH / WORLD_HEIGHT.
    void Init(int screenWidth, int screenHeight);

    // Call once per frame, before any DrawQuad calls, to clear the screen.
    void BeginFrame();

    // Draws a rectangle centered at `pos` (world units), `size` wide/tall
    // (world units), tinted `color`. `highlight` mixes the color toward
    // yellow: 0 = base color, 1 = fully highlighted. Use it for hover /
    // selection feedback, or drive it from a short timer for a damage flash.
    void DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec3 color, float highlight = 0.0f);

    // Call if the window is resized, so the projection keeps matching the
    // screen (Input::Init registers the GLFW resize callback that should
    // call this).
    void OnResize(int screenWidth, int screenHeight);

    // Call once at shutdown to free the shader/VAO/VBO.
    void Shutdown();
}
