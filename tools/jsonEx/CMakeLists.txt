# Set the jsonRet poject
project(jsonRet)
add_executable(jsonRet jsonRet.cpp)


set(JSON_BuildTests OFF CACHE INTERNAL "")

# Link the directories, notice how "sortIntegers" is found automatically
target_link_libraries(jsonRet PRIVATE nlohmann_json::nlohmann_json)

# Install - Secified in the root CMakeLists.txt