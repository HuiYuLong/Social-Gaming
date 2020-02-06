// A temp copy of user.h

#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
namespace rolls
{

// Class that defines user rolls for the game engine
// Rolls can change overtime
class User {
public:
    // Define User Types. One of 'GAMER', 'OWNER', or 'MAINTAINER'
    enum class UserType : uint8_t{
        GAMER,      // Someone who joins a game --> DEFAULT for now
        OWNER,      // Someone who creates a game
        MAINTAINER  // Server maintainer --> Not used yet
    };
private:
    std::string userName;
    std::vector<std::string> gamesPlayed;
    // std::vector<std::string> gamesCreated;
    UserType userType;

public:
    std::vector<std::string> gamesCreated;
    // Constructor
    User(std::string username);

    // Join a game
    void joinGame(std::string token);

    // Create a game
    void createGame(std::vector<std::string>& gameslist/* WILL NEED TO PASS SOME FORM OF JSON OR STRING */);

    // Leave a game
    void leaveGame();

    // List the game a user has played
    void listGamesPlayed(const std::vector<std::string>& gamelist);

    // List the game a user has created
    void listGamesCreated(const std::vector<std::string>& gamesCreated);

    UserType getUserType() { return userType; };
    
};  // class Users
}   // namespace rolls
#endif