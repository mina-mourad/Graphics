
#include "Renderer2D.h"
#include "GameConfig.h"

#include <vector>
#include <cmath>
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
        default:
            return { "00000", "00000", "00000", "00000", "00000", "00000", "00000" };
        }
    }

    int getGlyphWidth(char c)
    {
        return (c == ' ') ? 4 : 5;
    }
}

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

    // Fragment shader.
    const char* fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 u_Color;
uniform float u_Highlight;

void main()
{
    vec3 finalColor = mix(u_Color, vec3(1.0, 1.0, 0.0), u_Highlight);
    FragColor = vec4(finalColor, 1.0);
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

            std::cerr << "Renderer2D shader compile error:\n"
                      << log << "\n";
        }

        return shader;
    }

    GLuint shaderProgram = 0;

    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    // One reusable buffer for circles and polygons. Creating and deleting a
    // buffer for every shape is very slow when many shapes are drawn.
    GLuint shapeVAO = 0;
    GLuint shapeVBO = 0;

    GLint locModel = -1;
    GLint locProjection = -1;
    GLint locColor = -1;
    GLint locHighlight = -1;

    glm::mat4 projection(1.0f);


// Initialize the renderer.
void Renderer2D::Init(int screenWidth, int screenHeight)
{
    GLuint vertexShader =
        compileShader(GL_VERTEX_SHADER, vertexShaderSrc);

    GLuint fragmentShader =
        compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

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

        std::cerr << "Renderer2D shader link error:\n"
                  << log << "\n";
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create one reusable rectangle.
    float vertices[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,

         0.5f,  0.5f,
        -0.5f,  0.5f,
        -0.5f, -0.5f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create the reusable buffer for circles and polygons.
    glGenVertexArrays(1, &shapeVAO);
    glGenBuffers(1, &shapeVBO);

    glBindVertexArray(shapeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shapeVBO);

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Get shader uniform locations.
    locModel = glGetUniformLocation(shaderProgram, "u_Model");
    locProjection = glGetUniformLocation(shaderProgram, "u_Projection");
    locColor = glGetUniformLocation(shaderProgram, "u_Color");
    locHighlight = glGetUniformLocation(shaderProgram, "u_Highlight");

    OnResize(screenWidth, screenHeight);
}

// Update the projection when the window changes size.
void Renderer2D::OnResize(int screenWidth, int screenHeight)
{
    glViewport(0, 0, screenWidth, screenHeight);

    projection = glm::ortho(
        0.0f, WORLD_WIDTH,
        0.0f, WORLD_HEIGHT,
        -1.0f, 1.0f
    );
}

// Clear the screen.
void Renderer2D::BeginFrame()
{
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);
}

// Draw a rectangle.
void Renderer2D::DrawQuad(
    glm::vec2 pos,
    glm::vec2 size,
    glm::vec3 color,
    float highlight)
{
    glm::mat4 model(1.0f);

    model = glm::translate(
        model,
        glm::vec3(pos, 0.0f)
    );

    model = glm::scale(
        model,
        glm::vec3(size, 1.0f)
    );

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(
        locModel,
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );

    glUniformMatrix4fv(
        locProjection,
        1,
        GL_FALSE,
        glm::value_ptr(projection)
    );

    glUniform3f(
        locColor,
        color.r,
        color.g,
        color.b
    );

    glUniform1f(locHighlight, highlight);

    glBindVertexArray(quadVAO);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
}

void Renderer2D::DrawText(const std::string& text, glm::vec2 centerPos, float scale, glm::vec3 color, float highlight)
{
    const float cell = 0.18f * scale;
    const float gap = 0.12f * scale;

    float totalWidth = 0.0f;
    for (size_t i = 0; i < text.size(); ++i)
    {
        totalWidth += static_cast<float>(getGlyphWidth(text[i])) * cell + gap;
    }

    float cursorX = centerPos.x - totalWidth * 0.5f + 0.5f * cell;

    for (size_t i = 0; i < text.size(); ++i)
    {
        const char ch = text[i];
        const std::array<std::string, 7> pattern = getGlyphPattern(ch);

        if (ch == ' ')
        {
            cursorX += (5.0f * cell) + gap;
            continue;
        }

        for (int row = 0; row < 7; ++row)
        {
            for (int col = 0; col < 5; ++col)
            {
                if (pattern[row][col] == '1')
                {
                    glm::vec2 pixelPos(
                        cursorX + (col * cell) + (cell * 0.5f),
                        centerPos.y + (3.0f * cell) - (row * cell) - (cell * 0.5f)
                    );
                    DrawQuad(pixelPos, glm::vec2(cell * 0.8f, cell * 0.8f), color, highlight);
                }
            }
        }

        cursorX += (5.0f * cell) + gap;
    }
}

// Draw a circle.
void Renderer2D::DrawCircle(
    glm::vec2 pos,
    float radius,
    glm::vec3 color)
{
    const int segments = 40;

    std::vector<float> vertices;

    // Center of the circle.
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    // Points around the circle.
    for (int i = 0; i <= segments; i++)
    {
        float angle =
            2.0f * 3.14159265f * i / segments;

        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);

        vertices.push_back(x);
        vertices.push_back(y);
    }

    glBindVertexArray(shapeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shapeVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STREAM_DRAW
    );

    // Move the circle to its world position.
    glm::mat4 model(1.0f);

    model = glm::translate(
        model,
        glm::vec3(pos, 0.0f)
    );

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(
        locModel,
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );

    glUniformMatrix4fv(
        locProjection,
        1,
        GL_FALSE,
        glm::value_ptr(projection)
    );

    glUniform3f(
        locColor,
        color.r,
        color.g,
        color.b
    );

    glUniform1f(locHighlight, 0.0f);

    glDrawArrays(
        GL_TRIANGLE_FAN,
        0,
        segments + 2
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Draw a polygon.
void Renderer2D::DrawPolygon(
    const glm::vec2* points,
    int count,
    glm::vec3 color)
{
    if (count < 3)
        return;

    std::vector<float> vertices;

    // Copy the polygon points into the vertex array.
    for (int i = 0; i < count; i++)
    {
        vertices.push_back(points[i].x);
        vertices.push_back(points[i].y);
    }

    glBindVertexArray(shapeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shapeVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STREAM_DRAW
    );

    // Polygon points are already in world coordinates.
    glm::mat4 model(1.0f);

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(
        locModel,
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );

    glUniformMatrix4fv(
        locProjection,
        1,
        GL_FALSE,
        glm::value_ptr(projection)
    );

    glUniform3f(
        locColor,
        color.r,
        color.g,
        color.b
    );

    glUniform1f(locHighlight, 0.0f);

    glDrawArrays(
        GL_TRIANGLE_FAN,
        0,
        count
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Release renderer resources.
void Renderer2D::Shutdown()
{
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &shapeVAO);
    glDeleteBuffers(1, &shapeVBO);
    glDeleteProgram(shaderProgram);

    quadVAO = 0;
    quadVBO = 0;
    shapeVAO = 0;
    shapeVBO = 0;
    shaderProgram = 0;
}
