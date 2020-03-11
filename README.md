
# Social Game Engine

Work in progress...

## Dependencies

This project requires:

1. C++17 or newer
2. Boost >= 1.66
3. CMake >= 3.12

## Building with CMake

1. Clone the repository.

        git clone https://csil-git1.cs.surrey.sfu.ca/373-20-1-elvis/social-gaming.git

2. Change into the project directory.

        cd social-gaming

3. Create a directory for building and change into the new directory.

        mkdir build && cd build

4. Run CMake with the path to the source.

        cmake ..

5. Run make inside the build directory:

        make -j<N>

N stands for the number of available cores. This produces the `gameserver` tool which can be started with command `bin/gameserver <configuration file>`.

## Running the Example Game Server

The server can be started with command bin/gameserver ../configs/server/config1.json. After that, you can open the game engine website at localhost:4000, and create a new game with the Create Game button. The displayed invite code can be shared with other players for them to join the lobby. In the lobby, each user can create their name with the /username command or quit with the /quit command. The game owner, who created the game, can /shutdown the session and remove everyone from the lobby, or /select a game from the displayed list and /start it when everyone is ready. When the game ends, everyone should be returned back to their lobby.

