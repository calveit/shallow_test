cmake_minimum_required(VERSION 3.15)
project(ShallowTest)

set(CMAKE_C_STANDARD 17) # Requires C17 standard

set(SOURCE
   ${SOURCE}
   drawing.cpp
   game_state.cpp
   main.cpp
   parallel_for.cpp
   raylib_extensions.cpp
)

set(HEADERS
   ${HEADERS}
   constants.h
   drawing.h
   free_list.h
   game_state.h
   parallel_for.h
   raylib_extensions.h
   systems.h
   vector2.h
)

add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})
target_link_libraries(ShallowTest raylib)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)