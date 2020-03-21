add_subdirectory(lib/googlemock)
enable_testing()

add_executable(runAllTests
	exampleText.cpp
  # Add all new test files to the list here.
  # Place each file name on a separate line.
)
find_package(Threads REQUIRED)

target_link_libraries(runAllTests
 PRIVATE gmock
 PRIVATE gtest
 PRIVATE gtest_main
 PRIVATE networking
 PRIVATE translator
 PRIVATE Threads::Threads
 PRIVATE nlohmann_json::nlohmann_json
)

add_test(NAME AllTests COMMAND runAllTests)

