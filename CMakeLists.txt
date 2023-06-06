cmake_minimum_required(VERSION 3.23)
project(soasm)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-stdlib=libc++ -fexperimental-library)
add_link_options(-stdlib=libc++ -fexperimental-library)

include(cmake/CPM.cmake)
CPMAddPackage("gh:TheLartians/Ccache.cmake@1.2.4")
CPMAddPackage("gh:Neargye/magic_enum@0.8.2")

add_library(soisv1 src/types.cpp src/soisv1/model.cpp src/soisv1/instr_set.cpp)
target_include_directories(soisv1 PUBLIC "include" "${magic_enum_SOURCE_DIR}/include")

add_executable(soasm main.cpp)
target_link_libraries(soasm soisv1)