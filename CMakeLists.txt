cmake_minimum_required(VERSION 3.10)

project(chip-8)

include(FetchContent)

set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
)
if (NOT glfw_POPULATED)
  FetchContent_MakeAvailable(glfw)
endif()

file(GLOB_RECURSE CPP_SRC_FILES src/*.cpp)
file(GLOB_RECURSE C_SRC_FILES src/*.c)

add_executable(chip-8 ${CPP_SRC_FILES} ${C_SRC_FILES})

target_link_libraries(chip-8
  glfw
)
