add_library(interpreter interpreter.cpp)

target_include_directories(interpreter
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIR}
)

target_link_libraries(interpreter PRIVATE nlohmann_json::nlohmann_json)

#include_directories(include)
#add_executable(rule interpreter.cpp include/interpreter.h)