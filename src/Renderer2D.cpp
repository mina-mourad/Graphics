#include "Renderer2D.h"
#include "GameConfig.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace
{
    // Same shader pattern as the Lab 2/3 "highlight" demos (u_Highlight mixed
    // toward yellow in the fragment shader), adapted to GLSL 330 (matching
    // this project's template, not the labs' #version 430) and to a
    // per-instance model matrix instead of a single global transform.
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

uniform vec3 u_Color;
uniform float u_Highlight;   // 0 = base color, 1 = fully mixed toward yellow

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
        if (!success) {
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::cerr << "Renderer2D shader compile error:\n" << log << "\n";
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
    if (!success) {
        char log[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, log);
        std::cerr << "Renderer2D shader link error:\n" << log << "\n";
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // A single unit quad (1x1, centered on the origin), two triangles.
    // DrawQuad scales/translates this per-call through u_Model instead of
    // rebuilding geometry for every rectangle -- one VAO/VBO for the whole
    // game, same spirit as Lab 4's createMesh()/drawMesh() pattern.
    float vertices[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f,
        -0.5f, -0.5f,
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    locModel = glGetUniformLocation(shaderProgram, "u_Model");
    locProjection = glGetUniformLocation(shaderProgram, "u_Projection");
    locColor = glGetUniformLocation(shaderProgram, "u_Color");
    locHighlight = glGetUniformLocation(shaderProgram, "u_Highlight");

    OnResize(screenWidth, screenHeight);
}

void Renderer2D::OnResize(int screenWidth, int screenHeight)
{
    glViewport(0, 0, screenWidth, screenHeight);

    // Orthographic projection sized to the game's WORLD_WIDTH/WORLD_HEIGHT
    // (GameConfig.h), NOT the window's pixel size -- so world coordinates
    // (lane positions, tile positions) mean the same thing no matter what
    // the actual window resolution is. (0,0) is the bottom-left of the world,
    // matching Input::GetMouseWorldPos()'s Y-flip.
    projection = glm::ortho(0.0f, WORLD_WIDTH, 0.0f, WORLD_HEIGHT, -1.0f, 1.0f);
}

void Renderer2D::BeginFrame()
{
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer2D::DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec3 color, float highlight)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(pos, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(locColor, color.r, color.g, color.b);
    glUniform1f(locHighlight, highlight);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer2D::Shutdown()
{
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteProgram(shaderProgram);
}
