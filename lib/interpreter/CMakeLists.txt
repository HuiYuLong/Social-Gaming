project(interpreter_test)
add_executable(interpreter_test interpreter.cpp)

target_include_directories(interpreter_test
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIR}
)

target_link_libraries(interpreter_test PRIVATE nlohmann_json::nlohmann_json)