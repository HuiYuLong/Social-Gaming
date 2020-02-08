#include "test.h"
#include <assert.h>
#include <iostream>

using rolls::User;

User::User(std::string username)
    // : userName{username},
    //  userType{UserType::GAMER}
{
    assert(!username.empty() && "Error, cannot create an empty username!");
}

void
User::joinGame(std::string token)
{
    userType = UserType::GAMER;
    gamesPlayed.push_back("gameName");
    // Do stuff to join game
}

void
User::createGame(std::vector<std::string>& gamesCreated/* WILL NEED TO PASS SOME FORM OF JSON OR STRING */)
{
    userType = UserType::OWNER;
    // Do stuff to create game
    
    // Assume passed a string called NameId
    // if(!inList(NameId, gameslist)) {
        // Create a new game (need a Game class)
        gamesCreated.push_back("gameName");
    // }


}

// bool
// User::destroyGame(std::string gameName)
// {
//     if (this->getUserType() == OWNER)
//     {
//         // Kick gamers out
//         // Destroy game "gameName"
//         return true;
//     }
//     else
//     {
//         return false;
//     }
    
// }

// bool
// User::kickPlayer(std::string username)
// {
//     if (this->getUserType() == OWNER)
//     {
//         // Kick gamers with "userName" out
//         return true;
//     }
//     else
//     {
//         return false;
//     }
// }

void
User::leaveGame(/* WILL NEED TO PASS SOME FORM OF JSON OR STRING */)
{
    userType = UserType::GAMER;
    // Do stuff to leave game
}

void
User::listGamesPlayed(const std::vector<std::string>& gameslist)
{
    for (const auto& game: gameslist)
    {
        std::cout << " " << game;
    }
    std::cout << std::endl;
}

void
User::listGamesCreated(const std::vector<std::string>& gamesCreated)
{
    for (const auto& game: gamesCreated)
    {
        std::cout << " " << game;
    }
    std::cout << std::endl;
}

bool 
inList(std::string GameId, const std::vector<std::string>& gamesCreated) {
    for(auto& game:gamesCreated) {
        if(GameId.std::string::compare(game) == 0) {
            std::cout << "game already exists" << std::endl;
            return 1;
        }
    }
    return 0;
}

int main() {
    std::string username = "Sophia";
    User User(username);

    std::vector<std::string> gamesCreated;
    User.createGame(gamesCreated);
    std::cout << "Games that are created: ";
    User.listGamesCreated(gamesCreated);

    return 0;

}