cmake_minimum_required(VERSION 3.20)

project(ply2image)

set(DEFAULT_BUILD_TYPE "Release")

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Werror)
endif()

find_package(fmt REQUIRED)
find_package(PNG REQUIRED)
find_package(argparse REQUIRED)

add_executable(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/src/main.cpp")

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_link_libraries(${PROJECT_NAME} fmt::fmt)
target_link_libraries(${PROJECT_NAME} PNG::PNG)
target_link_libraries(${PROJECT_NAME} argparse::argparse)
