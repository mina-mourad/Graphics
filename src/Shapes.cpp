// [LAB] Almost all of this file comes from the labs:
//   - VBO, VAO, glVertexAttribPointer, glDrawArrays  -> Lab 1
//   - shader sources and compileShader()         -> Lab 2 template / Lab 3
//   - aPos + aColor vertex layout, u_Transform     -> Lab 3
//   - u_Tint (color multiply)                      -> Lab 2 template
//   - u_Highlight mix toward yellow                -> Lab 3
//   - u_Red mix toward red (hurt characters)       -> Lab 3 (the same code as u_Highlight, but red)
//   - u_Gray mix toward gray (locked cards)        -> Lab 3 (the same code as u_Highlight, but gray)
//   - one VBO holding several shapes + EBO         -> Lab 3 ("global VBO", EBO notes)
//   - circle as a GL_TRIANGLE_FAN                  -> Lab 3
//   - letters as GL_LINES                          -> Lab 3 notes (primitive types), Lab 4 (grid / axes)
//   - glm::translate / rotate / scale              -> Lab 2 template
//   - scale(1/aspect) so circles stay round        -> Lab 2 template (commented lines)
// [NOT LAB] one line in the shader: gray = (r + g + b) / 3, the gray version of a color.
// Below every part that uses lab code there is a "LAB CODE" comment with the
// lab lines exactly as they are written in the lab files.

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include "Shapes.h"
#include "Game.h"
#include "Text.h"
// LAB CODE (Lab 3 Notes/main.cpp):
//   #include <glad/glad.h>
//   #include <iostream>
//   #include <vector>
//   #include <cmath>
//   #include <glm/gtc/matrix_transform.hpp>
//   #include <glm/gtc/type_ptr.hpp>

float redness = 0.0f;
float grayness = 0.0f;
glm::mat4 model = glm::mat4(1.0f);   // the transform of a whole character (set by DrawCandy)
// no lab code

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
// LAB CODE (Lab 2 Template/main.cpp):
//   const char* vertexShaderSource = R"(
//   #version 430 core
//   layout (location = 0) in vec3 aPos;
//   layout (location = 1) in vec3 aColor;
//   out vec3 vColor;
//   uniform mat4 u_Transform;
//   void main()
//   {
//       gl_Position = u_Transform * vec4(aPos, 1.0);
//       vColor = aColor;
//   }
//   )";
// (we use "#version 330 core" because the project asks for OpenGL 3.3+)

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
    vec3 color = vColor * u_Tint.rgb;                     // Lab 2: tint
    color = mix(color, vec3(1.0, 1.0, 0.0), u_Highlight); // Lab 3: highlight (yellow)
    color = mix(color, vec3(0.9, 0.05, 0.05), u_Red);     // like Lab 3 highlight, but red
    float gray = (color.r + color.g + color.b) / 3.0;     // the gray version of this color (not lab)
    color = mix(color, vec3(gray), u_Gray);               // like Lab 3 highlight, but gray
    FragColor = vec4(color, 1.0);
}
)";
// LAB CODE (Lab 2 Template/main.cpp):
//   in vec3 vColor;
//   out vec4 FragColor;
//   uniform vec4 u_Tint;
//   FragColor = vec4(vColor, 1.0) * u_Tint;
// LAB CODE (Lab 3 Notes/main.cpp):
//   uniform float u_Highlight; //Whether or not we want it yellow! 0 for no, 1 for yes
//   vec3 finalColor = mix(vColor, vec3(1.0, 1.0, 0.0), u_Highlight); //Calculating the color we want at last, uses interpolation (WOW)
//   FragColor = vec4(finalColor, 1.0);
// (u_Red and u_Gray are copies of u_Highlight: the same mix, toward red / gray instead of yellow)

static GLuint VAO, VBO, EBO, shaderProgram;
static GLint locTransform, locTint, locHighlight, locRed, locGray;
// LAB CODE (Lab 3 Notes/main.cpp):
//   //Global VBO, VAO AND EBO
//   GLuint VAO, VBO, EBO;

const int CIRCLE_SEGMENTS = 40;
const int SQUARE_START = 0, TRIANGLE_START = 4, CIRCLE_START = 7;
// LAB CODE (Lab 3 Notes/main.cpp):
//   const int TRIANGLE_VERTEX_COUNT = 6; // vertices 0 -> 5 belong to the triangle
//   int circleVertexStart = 0; // filled in once the circle data is appended for flexibility
//   const int segments = 50; //increase to make smoother looking circle

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
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   GLuint compileShader(GLenum type, const char* source) {
    //       GLuint shader = glCreateShader(type);
    //       glShaderSource(shader, 1, &source, nullptr);
    //       glCompileShader(shader);
    //       int success;
    //       glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    //       if (!success) {
    //           char infoLog[512];
    //           glGetShaderInfoLog(shader, 512, nullptr, infoLog);
    //           std::cerr << "Shader compile error:\n" << infoLog << std::endl;
    //       }
    //       return shader;
    //   }
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
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    //   GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    //   GLuint shaderProgram = glCreateProgram();
    //   glAttachShader(shaderProgram, vertexShader);
    //   glAttachShader(shaderProgram, fragmentShader);
    //   glLinkProgram(shaderProgram);
    //   glDeleteShader(vertexShader);
    //   glDeleteShader(fragmentShader);

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
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   std::vector<float> vertices = {
    //       //Positions             //Colors
    //       -0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f, //0
    //        0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f, //3
    //        0.0f, 0.5f, 0.0f,      0.0f, 0.0f, 1.0f  //5
    //   };

    // circle: center, then the points around it
    vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.0f, 1, 1, 1 });
    for (int i = 0; i <= CIRCLE_SEGMENTS; ++i) {
        float angle = 2.0f * 3.14159f * (float)i / (float)CIRCLE_SEGMENTS;
        vertices.insert(vertices.end(), { cosf(angle), sinf(angle), 0.0f, 1, 1, 1 });
    }
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.0f,  0.0f, 0.8f, 1.0f }); // center and cyan color
    //   for (int i = 0; i <= segments; ++i) {
    //       float angle = 2.0f * 3.14f * (float)i / (float)segments;
    //       vertices.insert(vertices.end(), {
    //           cosf(angle), sinf(angle), 0.0f,
    //           0.0f, 0.8f, 1.0f
    //           });
    //   }

    AddLetterLines(vertices);   // the lines of every letter, after the circle (see Text.cpp)

    GLuint indices[] = { 0, 1, 2,  2, 3, 0 };   // the square = 2 triangles
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   GLuint indices[] = {
    //       0, 2, 1,
    //       2, 3, 4,
    //       4, 5, 1
    //   };

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
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   glGenVertexArrays(1, &VAO);
    //   glGenBuffers(1, &VBO);
    //   glGenBuffers(1, &EBO);
    //   glBindVertexArray(VAO);
    //   glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    //   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    //   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    //   glEnableVertexAttribArray(0);
    //   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    //   glEnableVertexAttribArray(1);
    // LAB CODE (Lab 1 Notes.pdf):
    //   unsigned int buffer_id;
    //   glGenBuffers(1, &buffer_id);
    //   glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    //   glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
    //   unsigned int VAO;
    //   glGenVertexArrays(1, &VAO);
    //   glBindVertexArray(VAO);
    //   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    //   glEnableVertexAttribArray(0);
    // (Lab 1 has only positions = 3 floats per vertex; Lab 2 and 3 add the color = 6 floats)

    locTransform = glGetUniformLocation(shaderProgram, "u_Transform");
    locTint = glGetUniformLocation(shaderProgram, "u_Tint");
    locHighlight = glGetUniformLocation(shaderProgram, "u_Highlight");
    locRed = glGetUniformLocation(shaderProgram, "u_Red");
    locGray = glGetUniformLocation(shaderProgram, "u_Gray");
    // LAB CODE (Lab 2 Template/main.cpp):
    //   GLint locTransform = glGetUniformLocation(shaderProgram, "u_Transform");
    //   GLint locTint = glGetUniformLocation(shaderProgram, "u_Tint");
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   GLint locHighlight = glGetUniformLocation(shaderProgram, "u_Highlight");
    // (locRed and locGray are found the same way as locHighlight)
}

// Sends the transform and color, ready for one draw call.
static void Prepare(float x, float y, float w, float h, float angle, glm::vec3 color, float highlight)
{
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::scale(transform, glm::vec3(1.0f / ASPECT, 1.0f, 1.0f));   // keep shapes round on a wide window
    transform = transform * model;   // the whole character (moved / scaled as one; normally "no change")
    transform = glm::translate(transform, glm::vec3(x, y, 0.0f));
    transform = glm::rotate(transform, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    transform = glm::scale(transform, glm::vec3(w, h, 1.0f));
    // LAB CODE (Lab 2 Template/main.cpp):
    //   glm::mat4 transform = glm::mat4(1.0f);
    //   /*float aspect = (float)g_screenWidth / (float)g_screenHeight;
    //   transform = glm::scale(transform, glm::vec3(1.0f / aspect, 1.0f, 1.0f));*/
    //   //transform = glm::translate(transform, glm::vec3(offsetX, 0.0f, 0.0f));
    //   //transform = glm::rotate(transform, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //   //transform = glm::scale(transform, glm::vec3(pulse, pulse, 1.0f));
    // (transform * model: not lab code - it puts the whole character's transform in front of this shape's own)

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glUniformMatrix4fv(locTransform, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform4f(locTint, color.r, color.g, color.b, 1.0f);
    glUniform1f(locHighlight, highlight);
    glUniform1f(locRed, redness);
    glUniform1f(locGray, grayness);
    // LAB CODE (Lab 2 Template/main.cpp):
    //   glUseProgram(shaderProgram);
    //   glUniformMatrix4fv(locTransform, 1, GL_FALSE, glm::value_ptr(transform));
    //   glUniform4f(locTint, r, g, b, 1.0f);
    //   glBindVertexArray(VAO);
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   glUniform1f(locHighlight, triangleHighlight ? 1.0f : 0.0f);
    // (redness is sent to u_Red and grayness to u_Gray the same way)
}

void Rect(float x, float y, float w, float h, glm::vec3 color, float angle, float highlight)
{
    Prepare(x, y, w, h, angle, color, highlight);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   // draw the triangle: indexed, uses the EBO, 9 indices for the three small triangles
    //   glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
    // (our square needs 6 indices = 2 triangles)
}

void Tri(float x, float y, float w, float h, glm::vec3 color, float angle)
{
    Prepare(x, y, w, h, angle, color, 0.0f);
    glDrawArrays(GL_TRIANGLES, TRIANGLE_START, 3);
    // LAB CODE (Lab 2 Template/main.cpp):
    //   glDrawArrays(GL_TRIANGLES, 0, 3);
    // LAB CODE (Lab 1 Notes.pdf):
    //   glDrawArrays(GL_TRIANGLES, 0, 3);
    // (our triangle starts at vertex 4 of the VBO, after the 4 corners of the square)
}

void Circle(float x, float y, float rx, float ry, glm::vec3 color)
{
    Prepare(x, y, rx, ry, 0.0f, color, 0.0f);
    glDrawArrays(GL_TRIANGLE_FAN, CIRCLE_START, CIRCLE_SEGMENTS + 2);
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   glDrawArrays(GL_TRIANGLE_FAN, circleVertexStart, circleVertexCount);
    //   circleVertexCount = segments + 2; // center + segments and because triangle fan... +1 for end :3
}

void Lines(int start, int count, float x, float y, float size, glm::vec3 color)
{
    Prepare(x, y, size, size, 0.0f, color, 0.0f);
    float width = size * WINDOW_H / 2.0f;           // line thickness in pixels: one step of the letter grid
    if (width < 1.0f) width = 1.0f;
    glLineWidth(width);
    glDrawArrays(GL_LINES, start, count);
    // LAB CODE (Lab 3 Notes/Lab 3 notes.pdf):
    //   GL_LINES: only 2 points make a line so discontinuous lines
    // LAB CODE (Lab 4 Code/Lab4Camera.cpp):
    //   GLenum  drawMode = GL_TRIANGLES;   // GL_TRIANGLES for solids, GL_LINES for grid/axes
    //   glDrawArrays(mesh.drawMode, 0, mesh.vertexCount);
    // LAB CODE (Lab 3 Notes/Lab3ExtraNotes.cpp):
    //   //glLineWidth(4);
}
