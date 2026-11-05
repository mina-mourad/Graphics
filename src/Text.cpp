// Person 1 (window, drawing and text).
// The labs do not draw text. The spec needs a coin counter and menu labels, so:
//   - LETTERS are line drawings: a few straight lines each, drawn with GL_LINES
//     (a primitive type from the Lab 3 notes, used for the grid / axes in Lab 4).
//     All the lines are stored once in the shared VBO (see InitShapes in Shapes.cpp).
//   - DIGITS look like a digital clock: 7 rectangles (segments), some on, some off.
// Below every part that uses lab code there is a "LAB CODE" comment with the
// lab lines exactly as they are written in the lab files.
// Parts with no lab code say "no lab code".

#include "Text.h"
#include "Shapes.h"

// Every letter sits on a small grid: x from 0 to 4, y from 0 (bottom) to 6 (top).
// "00-06" is one line from point (0,0) to point (0,6).
// The letters are kept blocky (mostly straight lines along the edges and the middle),
// so they match the digital-clock digits and need only 1 to 6 lines each.
static const char LETTERS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ!?.,:'-+/";
static const char* LINES[] = {
    "00-06 06-46 46-40 03-43",              // A
    "00-06 06-36 36-33 03-43 43-40 40-00",  // B
    "46-06 06-00 00-40",                    // C
    "00-06 06-26 26-43 43-20 20-00",        // D
    "46-06 06-00 00-40 03-33",              // E
    "46-06 06-00 03-33",                    // F
    "46-06 06-00 00-40 40-43 43-23",        // G
    "00-06 40-46 03-43",                    // H
    "06-46 00-40 20-26",                    // I
    "46-40 40-00 00-02",                    // J
    "00-06 03-46 03-40",                    // K
    "06-00 00-40",                          // L
    "00-06 06-23 23-46 46-40",              // M
    "00-06 06-40 40-46",                    // N
    "00-06 06-46 46-40 40-00",              // O
    "00-06 06-46 46-43 43-03",              // P
    "00-06 06-46 46-40 40-00 22-40",        // Q
    "00-06 06-46 46-43 43-03 23-40",        // R
    "46-06 06-03 03-43 43-40 40-00",        // S
    "06-46 26-20",                          // T
    "06-00 00-40 40-46",                    // U
    "06-20 20-46",                          // V
    "06-00 00-40 40-46 20-23",              // W
    "00-46 06-40",                          // X
    "06-23 46-23 23-20",                    // Y
    "06-46 46-00 00-40",                    // Z
    "26-22 20-21",                          // !
    "06-46 46-43 43-23 23-22 20-21",        // ?
    "20-21",                                // .
    "21-10",                                // ,
    "24-25 21-22",                          // :
    "26-24",                                // '
    "13-33",                                // -
    "03-43 21-25",                          // +
    "00-46",                                // /
};
static int lineStart[64], lineCount[64];   // where each letter's lines are in the VBO
// no lab code (the shapes of the letters are ours)

// Called by InitShapes() before the VBO is filled: adds every letter's lines
// (2 points = 1 line) to the vertex list, and remembers where each letter starts.
void AddLetterLines(std::vector<float>& vertices)
{
    for (int k = 0; LETTERS[k] != 0; ++k)
    {
        lineStart[k] = (int)vertices.size() / 6;           // 6 floats per vertex (position + color)
        const char* s = LINES[k];
        for (int j = 0; s[j] != 0; ++j)
            if (s[j] >= '0' && s[j] <= '9' && s[j + 1] >= '0' && s[j + 1] <= '9')
            {
                float x = (float)(s[j] - '0'), y = (float)(s[j + 1] - '0');
                vertices.insert(vertices.end(), { x, y, 0.0f,  1.0f, 1.0f, 1.0f });
                ++j;                                        // the y digit is used up too
            }
        lineCount[k] = (int)vertices.size() / 6 - lineStart[k];
    }
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   int circleVertexStart = 0; // filled in once the circle data is appended for flexibility
    //   circleVertexStart = TRIANGLE_VERTEX_COUNT; // = 6
    //   vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.0f,  0.0f, 0.8f, 1.0f }); // center and cyan color
    // LAB CODE (Lab 4 Code/Lab4Camera.cpp):
    //   const float AXES_VERTICES[] = {
    //       // X axis - red
    //       0.0f, 0.01f, 0.0f,     1.0f, 0.0f, 0.0f,
    //       5.0f, 0.01f, 0.0f,     1.0f, 0.0f, 0.0f,
    // (each line = 2 points with position + color, like the lab's axes; reading the
    //  "00-06" strings is our own code)
}

// Digits like a digital clock. Segments: a = top, b = top right, c = bottom right,
// d = bottom, e = bottom left, f = top left, g = middle.
static const char* DIGITS[10] = {
    "abcdef", "bc", "abdeg", "abcdg", "bcfg", "acdfg", "acdefg", "abc", "abcdefg", "abcdfg"
};

// One digit with its bottom-left corner at (x0, y0), on the same 4 x 6 grid as the letters.
static void Digit(int d, float x0, float y0, float size, glm::vec3 color)
{
    for (const char* seg = DIGITS[d]; *seg != 0; ++seg)
    {
        if (*seg == 'a') Rect(x0 + 2 * size, y0 + 6 * size, 5 * size, size, color);    // top
        if (*seg == 'b') Rect(x0 + 4 * size, y0 + 4.5f * size, size, 4 * size, color); // top right
        if (*seg == 'c') Rect(x0 + 4 * size, y0 + 1.5f * size, size, 4 * size, color); // bottom right
        if (*seg == 'd') Rect(x0 + 2 * size, y0, 5 * size, size, color);               // bottom
        if (*seg == 'e') Rect(x0, y0 + 1.5f * size, size, 4 * size, color);            // bottom left
        if (*seg == 'f') Rect(x0, y0 + 4.5f * size, size, 4 * size, color);            // top left
        if (*seg == 'g') Rect(x0 + 2 * size, y0 + 3 * size, 5 * size, size, color);    // middle
    }
}
// no lab code (each segment is a Rect, whose lab code is in Shapes.cpp)

void Text(const std::string& text, float x, float y, float size, glm::vec3 color)
{
    float width = text.size() * 6.0f * size;                // 5 wide + 1 gap per character
    float left = x - width / 2.0f;                          // centered on x
    for (size_t i = 0; i < text.size(); ++i)
    {
        float x0 = left + (i * 6 + 0.5f) * size;            // bottom-left of this character's grid
        float y0 = y - 3.0f * size;
        char ch = text[i];
        if (ch >= '0' && ch <= '9') { Digit(ch - '0', x0, y0, size, color); continue; }
        for (int k = 0; LETTERS[k] != 0; ++k)               // a space is not in the list: nothing drawn
            if (LETTERS[k] == ch) Lines(lineStart[k], lineCount[k], x0, y0, size, color);
    }
}
// no lab code (choosing the letter / digit and its place; the drawing is in Lines() and Rect())
