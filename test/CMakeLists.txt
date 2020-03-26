add_subdirectory(lib/googlemock)
enable_testing()

<<<<<<< HEAD
add_executable(runAllTests 
  VariablesTest.cpp
  TranslatorTest.cpp
=======
add_executable(runAllTests
	exampleText.cpp
>>>>>>> 01e4118ea818078cade8cef14e19b00e9017a752
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

FIND_PACKAGE( Boost 1.68.0 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_test(NAME AllTests COMMAND runAllTests)

