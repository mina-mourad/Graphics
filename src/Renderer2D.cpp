
#include "Renderer2D.h"
#include "GameConfig.h"

#include <vector>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace
{
    // Vertex shader.
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

    GLint locModel = -1;
    GLint locProjection = -1;
    GLint locColor = -1;
    GLint locHighlight = -1;

    glm::mat4 projection(1.0f);
}

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
        0.0f,
        WORLD_WIDTH,
        0.0f,
        WORLD_HEIGHT,
        -1.0f,
        1.0f
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

    GLuint VAO;
    GLuint VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
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

    // Release temporary resources.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
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

    GLuint VAO;
    GLuint VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
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

    // Release temporary resources.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

// Release renderer resources.
void Renderer2D::Shutdown()
{
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteProgram(shaderProgram);

    quadVAO = 0;
    quadVBO = 0;
    shaderProgram = 0;
}
