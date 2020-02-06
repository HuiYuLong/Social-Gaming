project(authentication)

include_directories(include)
add_executable(authentication user.cpp)


set(HEADER_FILES ../authentication/include/user.h)

add_library(mylib user.cpp ${HEADER_FILES})