// [LAB] Almost all of this file comes from the labs:
//   - shader sources and compileShader()         -> Lab 2 template / Lab 3
//   - aPos + aColor vertex layout, u_Transform     -> Lab 3
//   - u_Tint (color multiply)                      -> Lab 2 template
//   - u_Highlight mix toward yellow                -> Lab 3
//   - one VBO holding several shapes + EBO         -> Lab 3 ("global VBO", EBO notes)
//   - circle as a GL_TRIANGLE_FAN                  -> Lab 3
//   - glm::translate / rotate / scale              -> Lab 2 template
//   - scale(1/aspect) so circles stay round        -> Lab 2 template (commented lines)
// [NOT LAB] redness / grayness: just glm::mix on the color before drawing.

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include "Shapes.h"
#include "Game.h"

float redness = 0.0f;
float grayness = 0.0f;

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
void main()
{
    vec3 color = vColor * u_Tint.rgb;                   // Lab 2: tint
    color = mix(color, vec3(1.0, 1.0, 0.0), u_Highlight); // Lab 3: highlight
    FragColor = vec4(color, 1.0);
}
)";

static GLuint VAO, VBO, EBO, shaderProgram;
static GLint locTransform, locTint, locHighlight;

const int CIRCLE_SEGMENTS = 40;
const int SQUARE_START = 0, TRIANGLE_START = 4, CIRCLE_START = 7;

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

    // positions + colors (all white: the real color comes from u_Tint)
    std::vector<float> vertices = {
        // square (4 corners, drawn with the EBO)
        -0.5f, -0.5f, 0.0f,  1, 1, 1,
         0.5f, -0.5f, 0.0f,  1, 1, 1,
         0.5f,  0.5f, 0.0f,  1, 1, 1,
        -0.5f,  0.5f, 0.0f,  1, 1, 1,
        // triangle
        -0.5f, -0.5f, 0.0f,  1, 1, 1,
         0.5f, -0.5f, 0.0f,  1, 1, 1,
         0.0f,  0.5f, 0.0f,  1, 1, 1,
    };
    // circle: center, then the points around it (Lab 3)
    vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.0f, 1, 1, 1 });
    for (int i = 0; i <= CIRCLE_SEGMENTS; ++i) {
        float angle = 2.0f * 3.14159f * (float)i / (float)CIRCLE_SEGMENTS;
        vertices.insert(vertices.end(), { cosf(angle), sinf(angle), 0.0f, 1, 1, 1 });
    }
    GLuint indices[] = { 0, 1, 2,  2, 3, 0 };   // the square = 2 triangles

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    locTransform = glGetUniformLocation(shaderProgram, "u_Transform");
    locTint = glGetUniformLocation(shaderProgram, "u_Tint");
    locHighlight = glGetUniformLocation(shaderProgram, "u_Highlight");
}

// Sends the transform and color, ready for one draw call.
static void Prepare(float x, float y, float w, float h, float angle, glm::vec3 color, float highlight)
{
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::scale(transform, glm::vec3(1.0f / ASPECT, 1.0f, 1.0f));   // keep shapes round on a wide window
    transform = glm::translate(transform, glm::vec3(x, y, 0.0f));
    transform = glm::rotate(transform, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    transform = glm::scale(transform, glm::vec3(w, h, 1.0f));

    color = glm::mix(color, glm::vec3(0.9f, 0.05f, 0.05f), redness);
    float gray = (color.r + color.g + color.b) / 3.0f;
    color = glm::mix(color, glm::vec3(gray), grayness);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glUniformMatrix4fv(locTransform, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform4f(locTint, color.r, color.g, color.b, 1.0f);
    glUniform1f(locHighlight, highlight);
}

void Rect(float x, float y, float w, float h, glm::vec3 color, float angle, float highlight)
{
    Prepare(x, y, w, h, angle, color, highlight);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Tri(float x, float y, float w, float h, glm::vec3 color, float angle)
{
    Prepare(x, y, w, h, angle, color, 0.0f);
    glDrawArrays(GL_TRIANGLES, TRIANGLE_START, 3);
}

void Circle(float x, float y, float rx, float ry, glm::vec3 color)
{
    Prepare(x, y, rx, ry, 0.0f, color, 0.0f);
    glDrawArrays(GL_TRIANGLE_FAN, CIRCLE_START, CIRCLE_SEGMENTS + 2);
}
