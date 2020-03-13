
add_executable(gameserver
  gameserver.cpp
)

set_target_properties(gameserver
                      PROPERTIES
                      LINKER_LANGUAGE CXX
                      CXX_STANDARD 17
                      PREFIX ""
)


FIND_PACKAGE( Boost 1.68.0 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

find_package(Threads REQUIRED)

target_link_libraries(gameserver
  PRIVATE networking
  PRIVATE translator
  PRIVATE Threads::Threads
  PRIVATE nlohmann_json::nlohmann_json
)

install(TARGETS gameserver
  RUNTIME DESTINATION bin
)

