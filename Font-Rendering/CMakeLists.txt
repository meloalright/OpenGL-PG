cmake_minimum_required(VERSION 3.19)
project(OpenGL-PG)

# C++ version
set(CMAKE_CXX_STANDARD 11)

# 检查环境变量
if (NOT DEFINED ENV{GLFW_HOME})
message(FATAL_ERROR "found no env named GLFW_HOME")
endif()
if (NOT DEFINED ENV{GLAD_HOME})
message(FATAL_ERROR "found no env named GLAD_HOME")
endif()

# 暂存环境变量
set(GLFW_HOME $ENV{GLFW_HOME})
set(GLAD_HOME $ENV{GLAD_HOME})
set(GLM_HOME $ENV{GLM_HOME})
set(FREETYPE_HOME $ENV{FREETYPE_HOME})


# 设置头文件目录
include_directories("${GLFW_HOME}/include")
include_directories("${GLAD_HOME}/include")
include_directories("${GLM_HOME}/include")
include_directories("${FREETYPE_HOME}/include/freetype2")

# 添加 GLFW3 预编译库
add_library(glfw SHARED IMPORTED)
SET_TARGET_PROPERTIES(glfw PROPERTIES IMPORTED_LOCATION "${GLFW_HOME}/lib/libglfw.3.dylib")

add_library(freetype SHARED IMPORTED)
SET_TARGET_PROPERTIES(freetype PROPERTIES IMPORTED_LOCATION "${FREETYPE_HOME}/lib/libfreetype.dylib")

# 创建可执行文件
add_executable(OpenGL-PG main.cpp "lib/glad/src/glad.c")

# 链接 GLFW GLAD OpenGL
target_link_libraries(OpenGL-PG glfw freetype "-framework OpenGL")
