
cmake_minimum_required(VERSION 3.10)
project(OpenGLStarter)

add_subdirectory(external/glfw-3.5.1)
add_subdirectory(external/glm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

add_executable(OpenGLStarter
    main.cpp
    src/Renderer2D.cpp
    src/Input.cpp
    src/Text.cpp
    src/Draw.cpp
    src/Characters.cpp
    src/Environment.cpp
    src/Cutscene.cpp
    src/Game.cpp
    src/Audio.cpp
)

target_include_directories(OpenGLStarter PRIVATE external/glad/include external/miniaudio)
# GLAD provides OpenGL declarations; prevent GLFW from including system GL headers.
target_compile_definitions(OpenGLStarter PRIVATE GLFW_INCLUDE_NONE)
find_package(Threads REQUIRED)
target_link_libraries(OpenGLStarter PRIVATE glfw OpenGL::GL glm::glm Threads::Threads ${CMAKE_DL_LIBS})
