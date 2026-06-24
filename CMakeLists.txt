cmake_minimum_required(VERSION 3.15)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point this to wherever you extracted the Raylib prebuilt binaries
set(RAYLIB_PATH "C:/raylib/raylib")

include_directories(${RAYLIB_PATH}/include)
link_directories(${RAYLIB_PATH}/lib)

file(GLOB SOURCES "src/*.cpp")

add_executable(ChessEngine ${SOURCES})

target_link_libraries(ChessEngine raylib winmm)

# Copy assets folder next to the executable after build
add_custom_command(TARGET ChessEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:ChessEngine>/assets
)