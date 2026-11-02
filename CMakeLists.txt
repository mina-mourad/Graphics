cmake_minimum_required(VERSION 3.10)
project(OpenGLStarter)

add_subdirectory(external/glfw-3.5.1)
add_subdirectory(external/glm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)   # needed by miniaudio (sound)

add_executable(OpenGLStarter
    main.cpp
    src/Shapes.cpp
    src/Text.cpp
    src/Shelves.cpp
    src/Candies.cpp
    src/Sodas.cpp
    src/Hud.cpp
    src/Menus.cpp
    src/Cutscene.cpp
    src/Game.cpp
    src/Sound.cpp
)

target_include_directories(OpenGLStarter PRIVATE external/glad/include external/miniaudio)
target_compile_definitions(OpenGLStarter PRIVATE GLFW_INCLUDE_NONE SOUND_DIR="${CMAKE_SOURCE_DIR}/sounds/")
target_link_libraries(OpenGLStarter PRIVATE glfw OpenGL::GL glm::glm Threads::Threads ${CMAKE_DL_LIBS})

# Avoid incompatible MinGW runtime DLLs being loaded from Windows system directories.
if(MINGW)
    target_link_options(OpenGLStarter PRIVATE -static -static-libgcc -static-libstdc++)
endif()
