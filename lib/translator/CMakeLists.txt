project(translator_test)
add_executable(translator_test translator.cpp)

target_include_directories(translator_test
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIR}
)

target_link_libraries(translator_test
  PRIVATE nlohmann_json::nlohmann_json
  PUBLIC common)