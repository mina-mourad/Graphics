#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include "Shapes.h"
#include "Game.h"
#include "Text.h"

float redness = 0.0f;
float grayness = 0.0f;
glm::mat4 model = glm::mat4(1.0f);

static const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 vColor;
uniform mat4 u_Transform;
void main()
{
    gl_Position = u_Transform * vec4(aPos, 1.0);
    vColor = aColor;
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
uniform vec4 u_Tint;
uniform float u_Highlight;
uniform float u_Red;
uniform float u_Gray;
void main()
{
    vec3 color = vColor * u_Tint.rgb;
    color = mix(color, vec3(1.0, 1.0, 0.0), u_Highlight);
    color = mix(color, vec3(0.9, 0.05, 0.05), u_Red);
    float gray = (color.r + color.g + color.b) / 3.0;
    color = mix(color, vec3(gray), u_Gray);
    FragColor = vec4(color, 1.0);
}
)";

static GLuint Vao, Vbo, Ebo, shaderProgram;
static GLint locTransform, locTint, locHighlight, locRed, locGray;

const int CircleSegments = 40;
const int SquareStart = 0, TriangleStart = 4, CircleStart = 7;

static GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compile error:\n" << infoLog << std::endl;
    }
    return shader;
}

void InitShapes()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    std::vector<float> vertices = {
        -0.5f, -0.5f, 0.0f,  1, 1, 1,
         0.5f, -0.5f, 0.0f,  1, 1, 1,
         0.5f,  0.5f, 0.0f,  1, 1, 1,
        -0.5f,  0.5f, 0.0f,  1, 1, 1,
        -0.5f, -0.5f, 0.0f,  1, 1, 1,
         0.5f, -0.5f, 0.0f,  1, 1, 1,
         0.0f,  0.5f, 0.0f,  1, 1, 1,
    };

    vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.0f, 1, 1, 1 });
    for (int i = 0; i <= CircleSegments; ++i) {
        float angle = 2.0f * 3.14159f * (float)i / (float)CircleSegments;
        vertices.insert(vertices.end(), { cosf(angle), sinf(angle), 0.0f, 1, 1, 1 });
    }

    AddLetters(vertices);

    GLuint indices[] = { 0, 1, 2,  2, 3, 0 };

    glGenVertexArrays(1, &Vao);
    glGenBuffers(1, &Vbo);
    glGenBuffers(1, &Ebo);
    glBindVertexArray(Vao);
    glBindBuffer(GL_ARRAY_BUFFER, Vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    locTransform = glGetUniformLocation(shaderProgram, "u_Transform");
    locTint = glGetUniformLocation(shaderProgram, "u_Tint");
    locHighlight = glGetUniformLocation(shaderProgram, "u_Highlight");
    locRed = glGetUniformLocation(shaderProgram, "u_Red");
    locGray = glGetUniformLocation(shaderProgram, "u_Gray");
}

static void Prepare(float x, float y, float w, float h, float angle, glm::vec3 color, float highlight)
{
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::scale(transform, glm::vec3(1.0f / Aspect, 1.0f, 1.0f));
    transform = transform * model;
    transform = glm::translate(transform, glm::vec3(x, y, 0.0f));
    transform = glm::rotate(transform, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    transform = glm::scale(transform, glm::vec3(w, h, 1.0f));

    glUseProgram(shaderProgram);
    glBindVertexArray(Vao);
    glUniformMatrix4fv(locTransform, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform4f(locTint, color.r, color.g, color.b, 1.0f);
    glUniform1f(locHighlight, highlight);
    glUniform1f(locRed, redness);
    glUniform1f(locGray, grayness);
}

void Rect(float x, float y, float w, float h, glm::vec3 color, float angle, float highlight)
{
    Prepare(x, y, w, h, angle, color, highlight);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Tri(float x, float y, float w, float h, glm::vec3 color, float angle)
{
    Prepare(x, y, w, h, angle, color, 0.0f);
    glDrawArrays(GL_TRIANGLES, TriangleStart, 3);
}

void Circle(float x, float y, float rx, float ry, glm::vec3 color)
{
    Prepare(x, y, rx, ry, 0.0f, color, 0.0f);
    glDrawArrays(GL_TRIANGLE_FAN, CircleStart, CircleSegments + 2);
}

void Triangles(int start, int count, float x, float y, float size, glm::vec3 color)
{
    Prepare(x, y, size, size, 0.0f, color, 0.0f);
    glDrawArrays(GL_TRIANGLES, start, count);
}
