
cmake_minimum_required(VERSION 3.10)

project(OpenGLStarter)

add_subdirectory(external/glfw-3.5.1)
add_subdirectory(external/glm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

add_executable(OpenGLStarter
    main.cpp
    external/glad/glad.c
    src/Renderer2D.cpp
    src/Input.cpp
    src/EnemySystem.cpp
    src/MainMenu.cpp
    src/WaveManager.cpp
    src/CharacterRenderer.cpp
)

target_include_directories(OpenGLStarter PRIVATE
    external/glad/include
)

# Prevent GLFW from including system OpenGL headers.
target_compile_definitions(OpenGLStarter PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(OpenGLStarter PRIVATE
    glfw
    OpenGL::GL
    glm::glm
)

# Avoid incompatible MinGW runtime DLLs.
if(MINGW)
    target_link_options(OpenGLStarter PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
    )
endif()
