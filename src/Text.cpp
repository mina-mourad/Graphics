// Person 1 (window, drawing and text).
// The labs do not draw text. The spec needs a coin counter and menu labels, so the
// letters come from the one-header library stb_easy_font (external/stb/stb_easy_font.h,
// by Sean Barrett, public domain) - the same way the sound uses the library miniaudio.
//   - stb_easy_font gives every letter as a few small rectangles (no textures, no pictures).
//   - OpenGL 3.3 cannot draw 4-corner shapes, so each rectangle becomes 2 triangles.
//   - All the triangles are stored once in the shared VBO (see InitShapes in Shapes.cpp).
// Below every part that uses lab code there is a "LAB CODE" comment with the
// lab lines exactly as they are written in the lab files.
// Parts with no lab code say "no lab code".

#include "Text.h"
#include "Shapes.h"
#include "stb_easy_font.h"

// For every character: where its triangles are in the VBO, how many points they have,
// and how far to move right after it (stb_easy_font's letters are 4 steps wide, plus a gap).
static int letterStart[128], letterCount[128], letterWidth[128];

// Called by InitShapes() before the VBO is filled: adds the triangles of every character
// to the vertex list, and remembers where each character starts and how many points it has.
void AddLetters(std::vector<float>& vertices)
{
    for (int c = 0; c < 128; ++c)
    {
        letterStart[c] = (int)vertices.size() / 6;      // 6 floats per point
        letterCount[c] = 0;
        letterWidth[c] = 0;
        if (c < 32 || c > 126) continue;                // stb_easy_font has only the characters 32 to 126

        // stb_easy_font fills `corners` with the rectangles of this character.
        // Every corner is 4 numbers: x, y, z and the color (we only use x and y).
        char letter[2] = { (char)c, 0 };                // the character as a 1-letter string
        float corners[400];                             // room for 25 rectangles (the biggest letter, X, has 11)
        int rectangles = stb_easy_font_print(0, 0, letter, nullptr, corners, sizeof(corners));
        letterWidth[c] = stb_easy_font_width(letter);
        // no lab code (stb_easy_font is a library)

        // A rectangle has 4 corners (0 1 2 3) going around it; OpenGL 3.3 draws it as
        // 2 triangles: corners 0 1 2, then corners 2 3 0 (like the square in Shapes.cpp).
        int order[6] = { 0, 1, 2,  2, 3, 0 };
        for (int r = 0; r < rectangles; ++r)
            for (int k = 0; k < 6; ++k)
            {
                float* corner = &corners[(r * 4 + order[k]) * 4];
                // stb_easy_font's y goes down the screen and ours goes up, so y becomes -y
                vertices.insert(vertices.end(), { corner[0], -corner[1], 0.0f,  1.0f, 1.0f, 1.0f });
            }
        letterCount[c] = (int)vertices.size() / 6 - letterStart[c];
    }
    // LAB CODE (Lab 3 Notes/Lab 3 notes.pdf):
    //   GL_TRIANGLES: Draws a triangle with every 3 vertices
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.0f,  0.0f, 0.8f, 1.0f }); // center and cyan color
    //   int circleVertexStart = 0; // filled in once the circle data is appended for flexibility
    //   circleVertexStart = TRIANGLE_VERTEX_COUNT; // = 6
    // (every point is position + white color; like the lab remembers where the circle
    //  starts in the VBO, we remember where each letter starts)
}

void Text(const std::string& text, float x, float y, float size, glm::vec3 color)
{
    // the width of the whole text first, so it can be centered on x
    int width = 0;
    for (char ch : text)
    {
        int c = (unsigned char)ch;
        if (c < 128) width += letterWidth[c];
    }
    float left = x - width * size / 2.0f;               // centered on x
    float top = y + 3.5f * size;                        // a capital letter is 7 steps tall: centered on y

    for (char ch : text)
    {
        int c = (unsigned char)ch;
        if (c >= 128) continue;                         // not a character stb_easy_font has
        if (letterCount[c] > 0)                         // a space has no triangles: nothing drawn
            Triangles(letterStart[c], letterCount[c], left, top, size, color);
        left += letterWidth[c] * size;                  // move right to the next letter
    }
}
// no lab code (choosing the place of each letter; the drawing is in Triangles())
