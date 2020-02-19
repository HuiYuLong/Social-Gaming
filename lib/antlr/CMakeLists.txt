
project(Temp)

cmake_minimum_required(VERSION 3.16)

file(GLOB libantlrcpp_SRC
  "generated/*.cpp"
  "antlr4-runtime/*.cpp"
  "antlr4-runtime/atn/*.cpp"
  "antlr4-runtime/dfa/*.cpp"
  "antlr4-runtime/misc/*.cpp"
  "antlr4-runtime/support/*.cpp"
  "antlr4-runtime/tree/*.cpp"
  "antlr4-runtime/tree/pattern/*.cpp"
  "antlr4-runtime/tree/xpath/*.cpp"
)

add_library(antlr4_static STATIC ${libantlrcpp_SRC})

target_include_directories(antlr4_static
  PUBLIC generated
  PUBLIC antlr4-runtime/
  PUBLIC antlr4-runtime/atn
  PUBLIC antlr4-runtime/dfa
  PUBLIC antlr4-runtime/misc
  PUBLIC antlr4-runtime/support
  PUBLIC antlr4-runtime/tree
  PUBLIC antlr4-runtime/tree/pattern
  PUBLIC antlr4-runtime/tree/xpath
)

add_executable(evaluator evaluator.cpp)

target_link_libraries(evaluator
    PRIVATE antlr4_static
    PUBLIC variables
    PUBLIC nlohmann_json::nlohmann_json)



