add_library(translator translator.cpp variables.cpp)

target_include_directories(translator
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIR}
)

SET(CMAKE_CXX_FLAGS -pthread)

target_link_libraries(translator
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE networking)

target_link_libraries(translator
  PRIVATE nlohmann_json::nlohmann_json
  
  PRIVATE networking)