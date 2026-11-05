// Person 1 (window, drawing and text).
// The labs do not draw text. The spec needs a coin counter and menu labels, so:
//   - Letters are line drawings: a few straight lines each, drawn with GL_LINES
//     (a primitive type from the Lab 3 notes, used for the grid / axes in Lab 4).
//     All the lines are stored once in the shared VBO (see InitShapes in Shapes.cpp).
//   - Digits look like a digital clock: 7 rectangles (segments), some on, some off.
// Below every part that uses lab code there is a "LAB CODE" comment with the
// lab lines exactly as they are written in the lab files.
// Parts with no lab code say "no lab code".

#include "Text.h"
#include "Shapes.h"

// Every letter sits on a small grid: x from 0 to 4, y from 0 (bottom) to 6 (top).
// Line(0,0, 0,6) is one line from point (0,0) to point (0,6).
// The letters are kept blocky (straight lines along the edges and the middle),
// so they match the digital-clock digits and need only 1 to 6 lines each.

static std::vector<float>* points;              // the vertex list Line() adds to
static int lineStart[128], lineCount[128];      // where each character's lines are in the VBO

// One line of a letter = 2 points (position + white color, 6 floats each).
static void Line(float x1, float y1, float x2, float y2)
{
    points->insert(points->end(), { x1, y1, 0.0f,  1.0f, 1.0f, 1.0f,
                                    x2, y2, 0.0f,  1.0f, 1.0f, 1.0f });
    // LAB CODE (Lab 4 Code/Lab4Camera.cpp):
    //   const float AXES_VERTICES[] = {
    //       // X axis - red
    //       0.0f, 0.01f, 0.0f,     1.0f, 0.0f, 0.0f,
    //       5.0f, 0.01f, 0.0f,     1.0f, 0.0f, 0.0f,
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.0f,  0.0f, 0.8f, 1.0f }); // center and cyan color
    // (2 points with position + color make one line, like the lab's axes)
}

// The lines of one character (characters that are not here, like a space, have none).
static void LetterLines(char ch)
{
    if (ch == 'A') { Line(0,0, 0,6); Line(0,6, 4,6); Line(4,6, 4,0); Line(0,3, 4,3); }
    if (ch == 'B') { Line(0,0, 0,6); Line(0,6, 3,6); Line(3,6, 3,3); Line(0,3, 4,3); Line(4,3, 4,0); Line(4,0, 0,0); }
    if (ch == 'C') { Line(4,6, 0,6); Line(0,6, 0,0); Line(0,0, 4,0); }
    if (ch == 'D') { Line(0,0, 0,6); Line(0,6, 2,6); Line(2,6, 4,3); Line(4,3, 2,0); Line(2,0, 0,0); }
    if (ch == 'E') { Line(4,6, 0,6); Line(0,6, 0,0); Line(0,0, 4,0); Line(0,3, 3,3); }
    if (ch == 'F') { Line(4,6, 0,6); Line(0,6, 0,0); Line(0,3, 3,3); }
    if (ch == 'G') { Line(4,6, 0,6); Line(0,6, 0,0); Line(0,0, 4,0); Line(4,0, 4,3); Line(4,3, 2,3); }
    if (ch == 'H') { Line(0,0, 0,6); Line(4,0, 4,6); Line(0,3, 4,3); }
    if (ch == 'I') { Line(0,6, 4,6); Line(0,0, 4,0); Line(2,0, 2,6); }
    if (ch == 'J') { Line(4,6, 4,0); Line(4,0, 0,0); Line(0,0, 0,2); }
    if (ch == 'K') { Line(0,0, 0,6); Line(0,3, 4,6); Line(0,3, 4,0); }
    if (ch == 'L') { Line(0,6, 0,0); Line(0,0, 4,0); }
    if (ch == 'M') { Line(0,0, 0,6); Line(0,6, 2,3); Line(2,3, 4,6); Line(4,6, 4,0); }
    if (ch == 'N') { Line(0,0, 0,6); Line(0,6, 4,0); Line(4,0, 4,6); }
    if (ch == 'O') { Line(0,0, 0,6); Line(0,6, 4,6); Line(4,6, 4,0); Line(4,0, 0,0); }
    if (ch == 'P') { Line(0,0, 0,6); Line(0,6, 4,6); Line(4,6, 4,3); Line(4,3, 0,3); }
    if (ch == 'Q') { Line(0,0, 0,6); Line(0,6, 4,6); Line(4,6, 4,0); Line(4,0, 0,0); Line(2,2, 4,0); }
    if (ch == 'R') { Line(0,0, 0,6); Line(0,6, 4,6); Line(4,6, 4,3); Line(4,3, 0,3); Line(2,3, 4,0); }
    if (ch == 'S') { Line(4,6, 0,6); Line(0,6, 0,3); Line(0,3, 4,3); Line(4,3, 4,0); Line(4,0, 0,0); }
    if (ch == 'T') { Line(0,6, 4,6); Line(2,6, 2,0); }
    if (ch == 'U') { Line(0,6, 0,0); Line(0,0, 4,0); Line(4,0, 4,6); }
    if (ch == 'V') { Line(0,6, 2,0); Line(2,0, 4,6); }
    if (ch == 'W') { Line(0,6, 0,0); Line(0,0, 4,0); Line(4,0, 4,6); Line(2,0, 2,3); }
    if (ch == 'X') { Line(0,0, 4,6); Line(0,6, 4,0); }
    if (ch == 'Y') { Line(0,6, 2,3); Line(4,6, 2,3); Line(2,3, 2,0); }
    if (ch == 'Z') { Line(0,6, 4,6); Line(4,6, 0,0); Line(0,0, 4,0); }
    if (ch == '!') { Line(2,6, 2,2); Line(2,0, 2,1); }
    if (ch == '?') { Line(0,6, 4,6); Line(4,6, 4,3); Line(4,3, 2,3); Line(2,3, 2,2); Line(2,0, 2,1); }
    if (ch == '.') { Line(2,0, 2,1); }
    if (ch == ',') { Line(2,1, 1,0); }
    if (ch == ':') { Line(2,4, 2,5); Line(2,1, 2,2); }
    if (ch == '\'') { Line(2,6, 2,4); }
    if (ch == '-') { Line(1,3, 3,3); }
    if (ch == '+') { Line(0,3, 4,3); Line(2,1, 2,5); }
    if (ch == '/') { Line(0,0, 4,6); }
}
// no lab code (the shapes of the letters are ours)

// Called by InitShapes() before the VBO is filled: adds the lines of every character
// to the vertex list, and remembers where each character starts and how many points it has.
void AddLetterLines(std::vector<float>& vertices)
{
    points = &vertices;
    for (int c = 0; c < 128; ++c)
    {
        lineStart[c] = (int)vertices.size() / 6;           // 6 floats per point
        LetterLines((char)c);
        lineCount[c] = (int)vertices.size() / 6 - lineStart[c];
    }
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   int circleVertexStart = 0; // filled in once the circle data is appended for flexibility
    //   circleVertexStart = TRIANGLE_VERTEX_COUNT; // = 6
    // (like the lab remembers where the circle starts in the VBO, we remember where each letter starts)
}

// Digits like a digital clock: 7 rectangles (segments), each one on or off.
// (x0, y0) = bottom-left corner, on the same 4 x 6 grid as the letters.
static void Digit(int d, float x0, float y0, float size, glm::vec3 color)
{
    if (d != 1 && d != 4)                      Rect(x0 + 2 * size, y0 + 6 * size, 5 * size, size, color);     // top
    if (d != 5 && d != 6)                      Rect(x0 + 4 * size, y0 + 4.5f * size, size, 4 * size, color);  // top right
    if (d != 2)                                Rect(x0 + 4 * size, y0 + 1.5f * size, size, 4 * size, color);  // bottom right
    if (d != 1 && d != 4 && d != 7)            Rect(x0 + 2 * size, y0, 5 * size, size, color);                // bottom
    if (d == 0 || d == 2 || d == 6 || d == 8)  Rect(x0, y0 + 1.5f * size, size, 4 * size, color);             // bottom left
    if (d != 1 && d != 2 && d != 3 && d != 7)  Rect(x0, y0 + 4.5f * size, size, 4 * size, color);             // top left
    if (d != 0 && d != 1 && d != 7)            Rect(x0 + 2 * size, y0 + 3 * size, 5 * size, size, color);     // middle
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
        int c = (unsigned char)ch;
        if (c < 128 && lineCount[c] > 0)                    // a space has no lines: nothing drawn
            Lines(lineStart[c], lineCount[c], x0, y0, size, color);
    }
}
// no lab code (choosing the letter / digit and its place; the drawing is in Lines() and Rect())
