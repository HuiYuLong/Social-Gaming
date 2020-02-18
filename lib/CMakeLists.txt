add_subdirectory(networking)
add_subdirectory(json)
add_subdirectory(translator)
add_subdirectory(authentication)
add_subdirectory(antlr)

add_library(common INTERFACE)

target_include_directories(common INTERFACE .)