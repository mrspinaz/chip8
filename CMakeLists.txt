cmake_minimum_required(VERSION 3.15)

project(chip8 CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# raylib commands 
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.0 
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

#add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
#add_subdirectory(vendored/SDL_mixer EXCLUDE_FROM_ALL)

add_executable(chip8
    src/main.cpp
    src/Chip8.cpp
    src/Device.cpp
    src/Seg16.cpp
)

#target_link_libraries(chip8 PRIVATE SDL3_mixer::SDL3_mixer SDL3::SDL3)
target_link_libraries(chip8 PRIVATE raylib)