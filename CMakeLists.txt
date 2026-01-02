cmake_minimum_required(VERSION 3.22)
project(MyRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_TOOLCHAIN_FILE "C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(renderer
    "src/main.cpp"
    "src/scene.cpp"
    "src/PathMutator.cpp"
    "src/GeomUtil.cpp"
    "src/Renderer.cpp")

target_link_libraries(renderer PRIVATE assimp::assimp glm::glm)

target_include_directories(renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
