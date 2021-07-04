cmake_minimum_required(VERSION 3.15)
project(ShallowTestApp)

FIND_LIBRARY(OPTICK_LIBRARY NAMES OptickCore.lib PATHS "Optick_1.3.1/lib/x64/Release" DOC "Optick library")
FIND_PATH(OPTICK_INCLUDE_DIR optick.h optick.config.h "Optick_1.3.1/include" DOC "Optick includes")


set(CMAKE_C_STANDARD 17) # Requires C17 standard
add_subdirectory(raylib)
add_subdirectory(source)

TARGET_INCLUDE_DIRECTORIES(ShallowTest PUBLIC ${OPTICK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(ShallowTest ${OPTICK_LIBRARY})