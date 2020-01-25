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

void
User::joinGame(std::string token)
{
    userType = UserType::GAMER;
    gamesPlayed.push_back("gameName");
    // Do stuff to join game
    
//     if(userType.compare())
//     void compareOperation(string s1, string s2) 
// { 
//     // Compares 5 characters from index number 3 of s2 with s1 
//     if((s2.compare(3, 5, s1)) == 0) 
//         cout << "Here, "<< s1 << " are " << s2; 
  
//     else
//         cout << "Strings didn't match "; 
// } 
}

void
User::createGame(/* WILL NEED TO PASS SOME FORM OF JSON OR STRING */)
{
    userType = UserType::OWNER;
    gamesCreated.push_back("gameName");
    // Do stuff to create game

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
    User User(UserType::GAMER);

    // newUser.createGame();
    // newUser.listGamesCreated();

    return 0;

}