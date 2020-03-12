add_subdirectory(lib/googlemock)
enable_testing()

add_executable(runAllTests exampleText.cpp
  # Add all new test files to the list here.
  # Place each file name on a separate line.
)

target_link_libraries(runAllTests gmock gtest gtest_main)

add_test(NAME AllTests COMMAND runAllTests)

