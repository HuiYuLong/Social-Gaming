project(Gamespec)
add_executable(Gamespec Gamespec.cpp)
include_directories(../translator/include)
target_include_directories(Gamespec
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIR}
)