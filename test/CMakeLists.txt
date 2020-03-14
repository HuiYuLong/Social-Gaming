add_subdirectory(lib/googlemock)
enable_testing()

add_executable(runAllTests 
  VariablesTest.cpp
  TranslatorTest.cpp
  # Add all new test files to the list here.
  # Place each file name on a separate line.
)

target_link_libraries(runAllTests gmock gtest gtest_main)

FIND_PACKAGE( Boost 1.68.0 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_test(NAME AllTests COMMAND runAllTests)

