#include "Renderer2D.h"
#include "GameConfig.h"

#include <vector>
#include <cmath>
#include <cctype>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <iostream>
#include <string>

namespace
{
    std::array<std::string, 7> getGlyphPattern(char c)
    {
        switch (c)
        {
        case ' ':
            return { "00000", "00000", "00000", "00000", "00000", "00000", "00000" };
        case 'A':
            return { "01110", "10001", "10001", "11111", "10001", "10001", "10001" };
        case 'B':
            return { "11110", "10001", "10001", "11110", "10001", "10001", "11110" };
        case 'C':
            return { "01110", "10001", "10000", "10000", "10000", "10001", "01110" };
        case 'D':
            return { "11110", "10001", "10001", "10001", "10001", "10001", "11110" };
        case 'E':
            return { "11111", "10000", "10000", "11110", "10000", "10000", "11111" };
        case 'F':
            return { "11111", "10000", "10000", "11110", "10000", "10000", "10000" };
        case 'G':
            return { "01110", "10001", "10000", "10111", "10001", "10001", "01110" };
        case 'H':
            return { "10001", "10001", "10001", "11111", "10001", "10001", "10001" };
        case 'I':
            return { "11111", "00100", "00100", "00100", "00100", "00100", "11111" };
        case 'J':
            return { "00111", "00010", "00010", "00010", "10010", "10010", "01100" };
        case 'K':
            return { "10001", "10010", "10100", "11000", "10100", "10010", "10001" };
        case 'L':
            return { "10000", "10000", "10000", "10000", "10000", "10000", "11111" };
        case 'M':
            return { "10001", "11011", "10101", "10001", "10001", "10001", "10001" };
        case 'N':
            return { "10001", "11001", "10101", "10011", "10001", "10001", "10001" };
        case 'O':
            return { "01110", "10001", "10001", "10001", "10001", "10001", "01110" };
        case 'P':
            return { "11110", "10001", "10001", "11110", "10000", "10000", "10000" };
        case 'Q':
            return { "01110", "10001", "10001", "10001", "10101", "10010", "01101" };
        case 'R':
            return { "11110", "10001", "10001", "11110", "10100", "10010", "10001" };
        case 'S':
            return { "01111", "10000", "10000", "01110", "00001", "00001", "11110" };
        case 'T':
            return { "11111", "00100", "00100", "00100", "00100", "00100", "00100" };
        case 'U':
            return { "10001", "10001", "10001", "10001", "10001", "10001", "01110" };
        case 'V':
            return { "10001", "10001", "10001", "10001", "01010", "01010", "00100" };
        case 'W':
            return { "10001", "10001", "10001", "10101", "10101", "01010", "01010" };
        case 'X':
            return { "10001", "01010", "00100", "00100", "01010", "10001", "10001" };
        case 'Y':
            return { "10001", "01010", "00100", "00100", "00100", "00100", "00100" };
        case 'Z':
            return { "11111", "00001", "00010", "00100", "01000", "10000", "11111" };
        case '0': return { "01110", "10001", "10011", "10101", "11001", "10001", "01110" };
        case '1': return { "00100", "01100", "00100", "00100", "00100", "00100", "01110" };
        case '2': return { "01110", "10001", "00001", "00010", "00100", "01000", "11111" };
        case '3': return { "11110", "00001", "00001", "01110", "00001", "00001", "11110" };
        case '4': return { "00010", "00110", "01010", "10010", "11111", "00010", "00010" };
        case '5': return { "11111", "10000", "11110", "00001", "00001", "10001", "01110" };
        case '6': return { "00110", "01000", "10000", "11110", "10001", "10001", "01110" };
        case '7': return { "11111", "00001", "00010", "00100", "01000", "01000", "01000" };
        case '8': return { "01110", "10001", "10001", "01110", "10001", "10001", "01110" };
        case '9': return { "01110", "10001", "10001", "01111", "00001", "00010", "01100" };
        case '!': return { "00100", "00100", "00100", "00100", "00100", "00000", "00100" };
        case '?': return { "01110", "10001", "00001", "00010", "00100", "00000", "00100" };
        case '.': return { "00000", "00000", "00000", "00000", "00000", "01100", "01100" };
        case ',': return { "00000", "00000", "00000", "00000", "01100", "00100", "01000" };
        case ':': return { "00000", "01100", "01100", "00000", "01100", "01100", "00000" };
        case '\'': return { "00100", "00100", "01000", "00000", "00000", "00000", "00000" };
        case '-': return { "00000", "00000", "00000", "11111", "00000", "00000", "00000" };
        case '+': return { "00000", "00100", "00100", "11111", "00100", "00100", "00000" };
        case '/': return { "00001", "00010", "00010", "00100", "01000", "01000", "10000" };
        case '%': return { "11001", "11010", "00010", "00100", "01000", "01011", "10011" };
        case '(': return { "00010", "00100", "01000", "01000", "01000", "00100", "00010" };
        case ')': return { "01000", "00100", "00010", "00010", "00010", "00100", "01000" };
        case '<': return { "00010", "00100", "01000", "10000", "01000", "00100", "00010" };
        case '>': return { "01000", "00100", "00010", "00001", "00010", "00100", "01000" };
        case '"': return { "01010", "01010", "00000", "00000", "00000", "00000", "00000" };
        default:
            return { "00000", "00000", "00000", "00000", "00000", "00000", "00000" };
        }
    }

    int getGlyphWidth(char c)
    {
        return (c == ' ') ? 4 : 5;
    }

    // ------------------------------------------------------------------
    // Shaders (Lab 3 style: transform in the vertex shader, mix() in the
    // fragment shader).
    // ------------------------------------------------------------------
    const char* vertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 u_Model;
uniform mat4 u_Projection;

void main()
{
    gl_Position = u_Projection * u_Model * vec4(aPos, 0.0, 1.0);
}
)";

    const char* fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;

uniform vec3  u_Color;
uniform float u_Highlight;   // Lab 3: mix toward yellow
uniform float u_Damage;      // same idea: mix toward red
uniform float u_Gray;        // mix toward the gray value of the color
uniform float u_Fade;        // mix toward black

void main()
{
    vec3 c = mix(u_Color, vec3(1.0, 1.0, 0.0), u_Highlight);
    c = mix(c, vec3(0.85, 0.05, 0.05), u_Damage);
    float gray = dot(c, vec3(0.299, 0.587, 0.114));
    c = mix(c, vec3(gray), u_Gray);
    c = mix(c, vec3(0.0), u_Fade);
    FragColor = vec4(c, 1.0);
}
)";

    GLuint compileShader(GLenum type, const char* src)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::cerr << "Renderer2D shader compile error:\n" << log << "\n";
        }
        return shader;
    }

    GLuint shaderProgram = 0;

    // Unit quad (-0.5..0.5).
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    // Unit circle, built once as a triangle fan (Lab 3 circle).
    const int CIRCLE_SEGMENTS = 40;
    GLuint circleVAO = 0;
    GLuint circleVBO = 0;

    // One reusable buffer for free polygons.
    GLuint shapeVAO = 0;
    GLuint shapeVBO = 0;

    GLint locModel = -1;
    GLint locProjection = -1;
    GLint locColor = -1;
    GLint locHighlight = -1;
    GLint locDamage = -1;
    GLint locGray = -1;
    GLint locFade = -1;

    glm::mat4 projection(1.0f);
    float pixelsPerUnit = 80.0f;   // screen pixels per world unit (for crisp text)

    // Current effect state (see SetHighlight / SetDamage / ...).
    float effectHighlight = 0.0f;
    float effectDamage = 0.0f;
    float effectGray = 0.0f;
    float effectFade = 0.0f;

    // Sends the uniforms for one draw call.
    void SetUniforms(const glm::mat4& model, glm::vec3 color, float highlight)
    {
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(locColor, color.r, color.g, color.b);
        glUniform1f(locHighlight, std::fmax(highlight, effectHighlight));
        glUniform1f(locDamage, effectDamage);
        glUniform1f(locGray, effectGray);
        glUniform1f(locFade, effectFade);
    }

    // Creates a VAO + VBO holding 2D positions (attribute 0).
    void MakeBuffer(GLuint& vao, GLuint& vbo, const std::vector<float>& data, GLenum usage)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        if (!data.empty())
            glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), usage);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

// Initialize the renderer.
void Renderer2D::Init(int screenWidth, int screenHeight)
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, log);
        std::cerr << "Renderer2D shader link error:\n" << log << "\n";
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Unit rectangle made of two triangles.
    MakeBuffer(quadVAO, quadVBO, {
        -0.5f, -0.5f,   0.5f, -0.5f,   0.5f,  0.5f,
         0.5f,  0.5f,  -0.5f,  0.5f,  -0.5f, -0.5f }, GL_STATIC_DRAW);

    // Unit circle: center first, then the points around it (Lab 3).
    std::vector<float> circle = { 0.0f, 0.0f };
    for (int i = 0; i <= CIRCLE_SEGMENTS; ++i)
    {
        float angle = 2.0f * 3.14159265f * (float)i / (float)CIRCLE_SEGMENTS;
        circle.push_back(std::cos(angle));
        circle.push_back(std::sin(angle));
    }
    MakeBuffer(circleVAO, circleVBO, circle, GL_STATIC_DRAW);

    // Empty buffer that DrawPolygon refills every call.
    MakeBuffer(shapeVAO, shapeVBO, {}, GL_STREAM_DRAW);

    locModel = glGetUniformLocation(shaderProgram, "u_Model");
    locProjection = glGetUniformLocation(shaderProgram, "u_Projection");
    locColor = glGetUniformLocation(shaderProgram, "u_Color");
    locHighlight = glGetUniformLocation(shaderProgram, "u_Highlight");
    locDamage = glGetUniformLocation(shaderProgram, "u_Damage");
    locGray = glGetUniformLocation(shaderProgram, "u_Gray");
    locFade = glGetUniformLocation(shaderProgram, "u_Fade");

    OnResize(screenWidth, screenHeight);
}

// Update the projection when the window changes size.
void Renderer2D::OnResize(int screenWidth, int screenHeight)
{
    glViewport(0, 0, screenWidth, screenHeight);
    if (screenWidth > 0)
        pixelsPerUnit = screenWidth / WORLD_WIDTH;

    // Lab 4: orthographic projection. The world is WORLD_WIDTH x WORLD_HEIGHT
    // with (0,0) in the bottom-left corner.
    projection = glm::ortho(0.0f, WORLD_WIDTH, 0.0f, WORLD_HEIGHT, -1.0f, 1.0f);
}

// Clear the screen.
void Renderer2D::BeginFrame()
{
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ResetEffects();
}

void Renderer2D::SetHighlight(float amount) { effectHighlight = amount; }
void Renderer2D::SetDamage(float amount)    { effectDamage = amount; }
void Renderer2D::SetGray(float amount)      { effectGray = amount; }
void Renderer2D::SetFade(float amount)      { effectFade = amount; }

void Renderer2D::ResetEffects()
{
    effectHighlight = 0.0f;
    effectDamage = 0.0f;
    effectGray = 0.0f;
    effectFade = 0.0f;
}

// Draw a rectangle.
void Renderer2D::DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec3 color, float highlight)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(pos, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    SetUniforms(model, color, highlight);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// Draw a rotated rectangle: translate, then rotate, then scale (Lab 2).
void Renderer2D::DrawQuadRotated(glm::vec2 pos, glm::vec2 size, float angle, glm::vec3 color)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(pos, 0.0f));
    model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    SetUniforms(model, color, 0.0f);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Renderer2D::DrawLine(glm::vec2 a, glm::vec2 b, float width, glm::vec3 color)
{
    glm::vec2 d = b - a;
    float length = glm::length(d);
    if (length < 0.0001f)
        return;
    DrawQuadRotated((a + b) * 0.5f, glm::vec2(length, width), std::atan2(d.y, d.x), color);
}

// Draw an ellipse: the unit circle, scaled differently in x and y.
void Renderer2D::DrawEllipse(glm::vec2 pos, glm::vec2 radii, glm::vec3 color)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(pos, 0.0f));
    model = glm::scale(model, glm::vec3(radii, 1.0f));

    SetUniforms(model, color, 0.0f);
    glBindVertexArray(circleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2);
    glBindVertexArray(0);
}

void Renderer2D::DrawCircle(glm::vec2 pos, float radius, glm::vec3 color)
{
    DrawEllipse(pos, glm::vec2(radius, radius), color);
}

// Draw a polygon (points already in world coordinates).
void Renderer2D::DrawPolygon(const glm::vec2* points, int count, glm::vec3 color)
{
    if (count < 3)
        return;

    glBindVertexArray(shapeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shapeVBO);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec2), points, GL_STREAM_DRAW);

    SetUniforms(glm::mat4(1.0f), color, 0.0f);
    glDrawArrays(GL_TRIANGLE_FAN, 0, count);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer2D::DrawTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec3 color)
{
    glm::vec2 pts[3] = { a, b, c };
    DrawPolygon(pts, 3, color);
}

// ---------------------------------------------------------------------------
// Text
// ---------------------------------------------------------------------------

// The font is drawn on whole screen pixels: every font "pixel" becomes an
// exact number of screen pixels, so small text stays sharp and readable.
namespace
{
    float FontCell(float scale)
    {
        float px = std::round(0.18f * scale * pixelsPerUnit);
        return std::fmax(1.0f, px) / pixelsPerUnit;
    }
    float FontGap(float scale)
    {
        float px = std::round(0.12f * scale * pixelsPerUnit);
        return std::fmax(1.0f, px) / pixelsPerUnit;
    }
    float Snap(float v) { return std::round(v * pixelsPerUnit) / pixelsPerUnit; }
}

float Renderer2D::TextWidth(const std::string& text, float scale)
{
    if (text.empty()) return 0.0f;
    return text.size() * (5.0f * FontCell(scale) + FontGap(scale)) - FontGap(scale);
}

void Renderer2D::DrawTextAligned(const std::string& text, glm::vec2 pos, float scale, glm::vec3 color, Align align)
{
    const float cell = FontCell(scale);
    const float gap = FontGap(scale);
    const bool tiny = cell * pixelsPerUnit < 2.5f;
    const float dot = tiny ? cell : cell * 0.86f;   // small text: solid pixels, big text: a small gap between them

    float width = TextWidth(text, scale);
    float cursorX = pos.x;
    if (align == Align::Center) cursorX -= width * 0.5f;
    if (align == Align::Right)  cursorX -= width;
    cursorX = Snap(cursorX);
    const float top = Snap(pos.y + 3.5f * cell);

    for (char raw : text)
    {
        const char ch = static_cast<char>(std::toupper(static_cast<unsigned char>(raw)));
        if (ch != ' ')
        {
            const std::array<std::string, 7> pattern = getGlyphPattern(ch);
            for (int row = 0; row < 7; ++row)
                for (int col = 0; col < 5; ++col)
                    if (pattern[row][col] == '1')
                        DrawQuad(glm::vec2(cursorX + (col + 0.5f) * cell, top - (row + 0.5f) * cell), glm::vec2(dot, dot), color);
        }
        cursorX += 5.0f * cell + gap;
    }
}

void Renderer2D::DrawText(const std::string& text, glm::vec2 centerPos, float scale, glm::vec3 color, float highlight)
{
    float saved = effectHighlight;
    effectHighlight = std::fmax(effectHighlight, highlight);
    DrawTextAligned(text, centerPos, scale, color, Align::Center);
    effectHighlight = saved;
}

void Renderer2D::DrawTextShadow(const std::string& text, glm::vec2 pos, float scale, glm::vec3 color, Align align)
{
    const float o = std::fmax(1.0f, std::round(0.18f * scale * 0.7f * pixelsPerUnit)) / pixelsPerUnit;
    DrawTextAligned(text, pos + glm::vec2(o, -o), scale, glm::vec3(0.10f, 0.06f, 0.04f), align);
    DrawTextAligned(text, pos, scale, color, align);
}

// Release renderer resources.
void Renderer2D::Shutdown()
{
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &circleVAO);
    glDeleteBuffers(1, &circleVBO);
    glDeleteVertexArrays(1, &shapeVAO);
    glDeleteBuffers(1, &shapeVBO);
    glDeleteProgram(shaderProgram);

    quadVAO = quadVBO = circleVAO = circleVBO = shapeVAO = shapeVBO = 0;
    shaderProgram = 0;
}
