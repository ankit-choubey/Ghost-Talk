cmake_minimum_required(VERSION 3.11)
project(Ghost-Talk)

set(CMAKE_CXX_STANDARD 17)

# Define local Raylib
set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/vendor/raylib")
include_directories("${RAYLIB_DIR}/include")
link_directories("${RAYLIB_DIR}/lib")

add_executable(Ghost-Talk src/main.cpp)

# Link against local library. 
# We link against the static (.a) library explicitly.
target_link_libraries(Ghost-Talk "${RAYLIB_DIR}/lib/libraylib.a")

if(APPLE)
    target_link_libraries(Ghost-Talk "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()
