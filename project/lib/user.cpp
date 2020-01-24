#include "user.h"
#include <assert.h>
#include <iostream>

using rolls::User;

User::User(std::string username)
    : userName{username},
     userType{UserType::GAMER}
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
User::createGame(/* WILL NEED TO PASS SOME FORM OF JSON OR STRING */)
{
    userType = UserType::OWNER;
    gamesCreated.push_back("gameName");
    // Do stuff to create game
}

bool
User::destroyGame(std::string gameName)
{
    if (User::userType ==  User::UserType::OWNER)
    {
        // Kick gamers out
        // Destroy game "gameName"
        return true;
    }
    else
    {
        return false;
    }
    
}

bool
User::kickPlayer(std::string username)
{
    if (User::userType == User::UserType::OWNER)
    {
        // Kick gamers with "userName" out
        return true;
    }
    else
    {
        return false;
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