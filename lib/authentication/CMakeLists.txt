set(HEADER_FILES ../authentication/include}/file1.h)

add_library(mylib user.cpp ${HEADER_FILES})
add_executable(myexec user.cpp ${HEADER_FILES})