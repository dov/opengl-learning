cmake_minimum_required(VERSION 3.16)

project(million-points VERSION 1.0.0 LANGUAGES C CXX)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

link_libraries(
  glfw
  OpenGL::GL
  m)

include_directories(
  deps/glad/include/)

add_executable(million-points
  million-points.cpp
  deps/glad/src/glad.c
)

