add_subdirectory(lib/googlemock)
enable_testing()

add_executable(runAllTests
  # Add all new test files to the list here.
  # Place each file name on a separate line.
  TriangleTest.cpp # Part 1: Triangle Class tests
  HailstoneTest.cpp # Part 2: Hailstone function tests
  TestAwardCeremony.cpp # Part 3: Awards function tests
)

target_link_libraries(runAllTests gmock gtest gtest_main)
target_link_libraries(runAllTests simpleLibrary)

add_test(NAME AllTests COMMAND runAllTests)

