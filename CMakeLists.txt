cmake_minimum_required(VERSION 3.15)
project(rasterizer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 — works with vcpkg toolchain or a manual SDL2_DIR
find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
    find_package(SDL2 REQUIRED)
endif()

add_executable(rasterizer
    src/main.cpp
    src/mesh.cpp
    src/renderer.cpp
)

target_include_directories(rasterizer PRIVATE src)

if(TARGET SDL2::SDL2)
    target_link_libraries(rasterizer PRIVATE SDL2::SDL2 SDL2::SDL2main)
else()
    target_include_directories(rasterizer PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(rasterizer PRIVATE ${SDL2_LIBRARIES})
endif()

if(MSVC)
    target_compile_options(rasterizer PRIVATE /W4 /O2)
    # Copy SDL2.dll next to the executable on Windows
    if(TARGET SDL2::SDL2)
        add_custom_command(TARGET rasterizer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL2::SDL2>
                $<TARGET_FILE_DIR:rasterizer>)
    endif()
else()
    target_compile_options(rasterizer PRIVATE -Wall -Wextra -O2)
endif()
