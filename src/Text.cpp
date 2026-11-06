#include "Text.h"
#include "Shapes.h"
#include "stb_easy_font.h"

static int letterStart[128], letterCount[128], letterWidth[128];

void AddLetters(std::vector<float>& vertices)
{
    for (int c = 0; c < 128; ++c)
    {
        letterStart[c] = (int)vertices.size() / 6;
        letterCount[c] = 0;
        letterWidth[c] = 0;
        if (c < 32 || c > 126) continue;

        char letter[2] = { (char)c, 0 };
        float corners[400];
        int rectangles = stb_easy_font_print(0, 0, letter, nullptr, corners, sizeof(corners));
        letterWidth[c] = stb_easy_font_width(letter);

        int order[6] = { 0, 1, 2,  2, 3, 0 };
        for (int r = 0; r < rectangles; ++r)
            for (int k = 0; k < 6; ++k)
            {
                float* corner = &corners[(r * 4 + order[k]) * 4];
                vertices.insert(vertices.end(), { corner[0], -corner[1], 0.0f,  1.0f, 1.0f, 1.0f });
            }
        letterCount[c] = (int)vertices.size() / 6 - letterStart[c];
    }
}

void Text(const std::string& text, float x, float y, float size, glm::vec3 color)
{
    int width = 0;
    for (char ch : text)
    {
        int c = (unsigned char)ch;
        if (c < 128) width += letterWidth[c];
    }
    float left = x - width * size / 2.0f;
    float top = y + 3.5f * size;

    for (char ch : text)
    {
        int c = (unsigned char)ch;
        if (c >= 128) continue;
        if (letterCount[c] > 0)
            Triangles(letterStart[c], letterCount[c], left, top, size, color);
        left += letterWidth[c] * size;
    }
}
