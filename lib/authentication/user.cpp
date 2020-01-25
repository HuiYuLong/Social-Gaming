#include "test.h"
#include <assert.h>
#include <iostream>

using rolls::User;

User::User(std::string username)

    : userName{username},
     userType{UserType::GAMER}
{
    assert(!username.empty() && "Error, cannot create an empty username!");
}

User::~User() = default;

void
User::joinGame(std::string token)
{
    userType = UserType::GAMER;
    gamesPlayed.push_back("gameName");
    // Do stuff to join game

    // look for matches in the gameslist
    for (const auto& game: gameslist)
    {
        // if the Game exist, then join
        int count = token.compare(game);
        if(count == 0)
            // connect to the GameServer
        else
            cout << "Game does not exist" << endl;
    }
} 
}

void
User::createGame(/* WILL NEED TO PASS SOME FORM OF JSON OR STRING */)
{
    userType = UserType::OWNER;
    // Do stuff to create game
    
    // Assume passed a string called NameId
    for (const auto& game: gameslist)
    {
        // if the Game Id is unique, then create a new game
        int count = NameId.compare(game);
        if(count != 0)      
        {
            gamesCreated.push_back("gameName");
            joinGame(NameId);
        }
        else
            cout << "Game already exists" << endl;
    }


}

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



int main() {
    std::string username = "Sophia";
    // User User(UserType::GAMER);

    // newUser.createGame();
    // newUser.listGamesCreated();

    return 0;

}