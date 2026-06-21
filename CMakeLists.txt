cmake_minimum_required(VERSION 3.15)
project(rasterizer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 — tries vcpkg config, then module mode, then a manual SDL2_ROOT path
find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
    find_package(SDL2 MODULE QUIET)
endif()
if(NOT SDL2_FOUND)
    if(NOT DEFINED SDL2_ROOT)
        message(FATAL_ERROR
            "SDL2 not found.\n"
            "Option A: install vcpkg and pass -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake\n"
            "Option B: download the SDL2 VC dev zip, extract it, and pass -DSDL2_ROOT=<path>")
    endif()
    find_path(SDL2_INCLUDE_DIRS SDL.h
        HINTS "${SDL2_ROOT}/include/SDL2" "${SDL2_ROOT}/include")
    find_library(SDL2_LIB     NAMES SDL2     HINTS "${SDL2_ROOT}/lib/x64" "${SDL2_ROOT}/lib")
    find_library(SDL2MAIN_LIB NAMES SDL2main HINTS "${SDL2_ROOT}/lib/x64" "${SDL2_ROOT}/lib")
    set(SDL2_LIBRARIES ${SDL2_LIB} ${SDL2MAIN_LIB})
    if(NOT SDL2_INCLUDE_DIRS OR NOT SDL2_LIB)
        message(FATAL_ERROR "SDL2 headers/libs not found under SDL2_ROOT=${SDL2_ROOT}")
    endif()
    set(SDL2_FOUND TRUE)
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
    if(TARGET SDL2::SDL2)
        add_custom_command(TARGET rasterizer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL2::SDL2>
                $<TARGET_FILE_DIR:rasterizer>)
    elseif(DEFINED SDL2_ROOT)
        # Manual SDL2: copy SDL2.dll from the x64 lib folder
        add_custom_command(TARGET rasterizer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SDL2_ROOT}/lib/x64/SDL2.dll"
                $<TARGET_FILE_DIR:rasterizer>)
    endif()
else()
    target_compile_options(rasterizer PRIVATE -Wall -Wextra -O2)
endif()
