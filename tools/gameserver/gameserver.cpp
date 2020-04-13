/////////////////////////////////////////////////////////////////////////////
//                         Single Threaded Networking
//
// This file is distributed under the MIT License. See the LICENSE file
// for details.
/////////////////////////////////////////////////////////////////////////////


#include "Server.h"
#include "translator.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <thread>
#include  <random>

using networking::Server;
using networking::Connection;
using networking::Message;
using networking::ConnectionHash;

using json = nlohmann::json;

//Connection DISCONNECTED{reinterpret_cast<uintptr_t>(nullptr)};

// All game configurations available on the server
std::vector<Configuration> configurations;

std::string select_random_animal() {
    static std::vector<std::string> random_animals = {"Alligator", "Anteater", "Armadillo", "Auroch", "Axolotl", "Badger", "Bat", "Bear", "Beaver", "Buffalo", "Camel", "Capybara", "Chameleon", "Cheetah", "Chinchilla", "Chipmunk", "Chupacabra", "Cormorant", "Coyote", "Crow", "Dingo", "Dinosaur", "Dog", "Dolphin", "Duck", "Elephant", "Ferret", "Fox", "Frog", "Giraffe", "Gopher", "Grizzly", "Hedgehog", "Hippo", "Hyena", "Ibex", "Ifrit", "Iguana", "Jackal", "Kangaroo", "Koala", "Kraken", "Lemur", "Leopard", "Liger", "Lion", "Llama", "Loris", "Manatee", "Mink", "Monkey", "Moose", "Narwhal", "Nyan Cat", "Orangutan", "Otter", "Panda", "Penguin", "Platypus", "Pumpkin", "Python", "Quagga", "Rabbit", "Raccoon", "Rhino", "Sheep", "Shrew", "Skunk", "Squirrel", "Tiger", "Turtle", "Walrus", "Wolf", "Wolverine", "Wombat"};
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, random_animals.size() - 1);
    return "Anonymous " + random_animals[dis(gen)];
}

/**
 * Since the server should be able to handle multiple games,
 * some identifier is needed to distinguish different connections.
 * Each game corresponds to a session, and each connection of the game belongs to that session.
 */
struct GameSession {
  uintptr_t id;
  // The person who created the game
  Connection game_owner;
  // Created on the client, passed as the url target and accepted in the onConnect function 
  std::string invite_code;
  // // Connections of players in this game session
  // std::vector<Connection> players;
  // Part of the job of the game session is to set up the game state that is used by the interpreter
  std::unique_ptr<GameState> game_state;
  // Mapping of in-game player names to their connections created with the /username command
  // Will be passed to the interpreter through the game state so the interpreter can send and receive messages via the server
  std::unordered_map<Connection, std::string, ConnectionHash> players;
  // Currently /selected game configuration
  Configuration* configuration;
  // When the session is detached, the gameserver stops handling the messages of the players
  // and the control of this session is passed to a separate thread that runs the game
  bool detached;

  GameSession(Connection game_owner, std::string_view invite_code):
    id(reinterpret_cast<uintptr_t>(this)),
    game_owner(game_owner),
    invite_code(invite_code),
    configuration(nullptr),
    detached(false)
    { }

  bool
  operator==(const GameSession& other) const
  {
    return id == other.id;
  }

  // Adds the username to the name2connection mapping
  std::string
  change_username(const std::string& name, Connection connection)
  {
    if(name.size() == 0) {
      return "Don't be shy, enter a real name\n\n";
    }
    auto found = std::find_if(players.begin(), players.end(), [&name](const std::pair<Connection, std::string> iter) {
      return name == iter.second;
    });
    if (found != players.end()) {
      return "This name is already used\n\n";
    }
    players.at(connection) = name;
    return "Changed the username to " + name + "\n\n";
  }

  // // Removes the username of a given connectionfrom name2connection 
  // bool
  // remove_username(Connection connection)
  // {
  //   auto found = std::find_if(name2connection.begin(), name2connection.end(), [connection](const std::pair<std::string, Connection>& iter) {
  //     return connection == iter.second;
  //   });
  //   if(found != name2connection.end()) {
  //     name2connection.erase(found);
  //     return true;
  //   }
  //   return false;
  // }

  // Indicates whether all the necessary fields of the game session are set up properly
  // and the game session is ready to start the game
  std::pair<std::string, bool>
  validate(const std::string& game_name)
  {
    try {
      // Assume that the user provided the index from the list
      size_t config_index = std::stoul(game_name) - 1u; // user-visible list starts with one
      configuration = &configurations.at(config_index);
    }
    catch (std::exception& e) {
      // Otherwise, try to find it by it's name
      auto game = std::find_if(configurations.begin(), configurations.end(),
        [&game_name](const Configuration& conf) {
        return game_name == conf.getName();
      });
      if (game != configurations.end()) {
        configuration = &(*game);
      }
      else {
        return {"Could not find a game with the given name\n\n", false};
      }
    }
    if (players.size() > configuration->getPlayerCountMax()) {
      std::ostringstream ostream;
      ostream << "Too many players for this game. You need to evict "
        << players.size() - configuration->getPlayerCountMax() << " player(s)\n\n";
      return {ostream.str(), false};
    }
    if (players.size() < configuration->getPlayerCountMin()) {
      std::ostringstream ostream;
      ostream << "This game requires at least " << configuration->getPlayerCountMin() << " people to play, but only "
        << players.size() << " are present in the lobby\n\n";
      return {ostream.str(), false};
    }
    return {"Starting the game...\n\n", true};
  }

  std::string
  start(Server& server, const std::string& game_name)
  {
    auto [notice, good_to_go] = validate(game_name);
    if(good_to_go) {
      Name2Connection name2connection;
      name2connection.reserve(players.size());
      for (const auto& [connection, name] : players) {
        name2connection.emplace(name, connection);
      }
      game_state = std::make_unique<GameState>(*configuration, std::move(name2connection), game_owner);
      std::thread t([this, &server]() {
        detached = true;
        std::cout << "Session " << id << " is set free" << std::endl;
        try {
          configuration->launchGame(server, *game_state);
        }
        catch (std::out_of_range& e) {  // out of range exception should be caused by the channels' at() method in the server's receive() or send() methods if the user has disconnected
          std::cout << "One of the players has disconnected while the game was on" << std::endl;
        }
        catch (std::exception& e) {
          std::cout << "Warning: a game thread has exited with an exception " << e.what() << std::endl;
        }
        for(const auto& [connection, name] : this->players) {
          server.send({connection, "The game is over!\n\n"});
        }
        detached = false;
        std::cout << "Session's " << id << " thread is finished" << std::endl;
      });
      t.detach();
    }
    return notice;
  }

};

// Map that allows to find each player's session based on the connection
std::unordered_map<Connection, GameSession*, ConnectionHash> sessionMap;

// Publicly available collection of sessions
std::vector<std::unique_ptr<GameSession>> gameSessions;

std::string welcoming_message;

// Called by the server when a user connects
// Creates a new game session if the player connects to a target that has not been registered,
// otherwise adds the player to the existing game session
// Also, sends the welcoming message with the instructions on using the game engine
void
onConnect(Connection c, std::string_view target, Server& server) {
  auto gameSessionIter = std::find_if(gameSessions.begin(), gameSessions.end(), [target](std::unique_ptr<GameSession>& gameSession) {
    return target.compare(gameSession->invite_code) == 0;
  });
  if (gameSessionIter != gameSessions.end()) {
    auto& gameSession = *gameSessionIter;
    std::string new_name = select_random_animal();
    gameSession->players.emplace(c, new_name);
    sessionMap[c] = gameSession.get();
    std::cout << "Session " << gameSession->id << " joined by " << new_name << std::endl;
    for(const auto& [connection, _] : gameSession->players) {
      server.send({connection, new_name + " has joined the lobby\n\n"});
    }
  }
  else {
    GameSession* gameSession = gameSessions.emplace_back(std::make_unique<GameSession>(c, target)).get();
    gameSession->players.emplace(c, "Game Owner");
    sessionMap[c] = gameSession;
    std::cout << "Session " << gameSessions.back()->id << " created by Game Owner" << std::endl;
  }

  server.send({c, welcoming_message});
}

// Called by the server when the user disconnects
// Remove the player from the sessionMap and the session's players list and name2connection mapping
// If no players are left in the session, it will be removed
void
onDisconnect(Connection c, Server& server) {
  // remove the connection from the session
  auto session = sessionMap.at(c);
  sessionMap.erase(c);
  std::cout << "Session " << session->id << " has lost connection with " << session->players.at(c) << std::endl;
  for(const auto& [connection, name] : session->players) {
    server.send({connection, session->players.at(c) + " has left\n\n"});
  }
  session->players.erase(c);
  if(session->players.empty())
  {
    std::cout << "No players left. Shutting down session " << session->id << std::endl;
    while (session->detached) { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
    gameSessions.erase(std::remove_if(std::begin(gameSessions), std::end(gameSessions), [](std::unique_ptr<GameSession>& session) { return session->players.empty(); }), std::end(gameSessions));
  }
}


// Prepares the default HTTP response of the server
std::string
getHTTPMessage(const std::string& htmlLocation) {
  std::ifstream infile{htmlLocation};
  if (infile) {
    return std::string{std::istreambuf_iterator<char>(infile),
                       std::istreambuf_iterator<char>()};
  } else {
    std::cerr << "Unable to open HTML index file:\n"
              << htmlLocation << "\n";
    std::exit(-1);
  }
}


// Creates the server and the game configuratons based on the server configuration file
// Implements the pre-game lobby where the players can select a game to play, provide
// their usernames and just chat
int
main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage:\n  " << argv[0] << "<server config>\n"
              << "  e.g. " << argv[0] << " ../configs/server/congfig1.json\n";
    return 1;
  }

  std::ifstream serverconfig{argv[1]};
  if (serverconfig.fail()) {
      std::cout << "Could not open the server configuration file" << std::endl;
      return 1;
  }
  json serverspec = json::parse(serverconfig);

	configurations.reserve(serverspec["games"].size());

  std::ostringstream ostream;
  ostream << "Welcome to the Social Game Engine!\n\n\
/username &lt;name&gt; - to change your name\n\
/start &lt;game&gt; - to start the game\n\
/quit - if you need to leave\n\
/shutdown - to close the game\n\n\nAvailable games:\n\n";
  for ([[maybe_unused]] const auto& [key, gamespecfile]: serverspec["games"].items())
	{
		std::ifstream gamespecstream{std::string(gamespecfile)};
		if (gamespecstream.fail()) {
			std::cout << "Could not open the game configuration file " << gamespecfile << std::endl;
			return 1;
		}
    
		json gamespec = json::parse(gamespecstream);
		configurations.emplace_back(gamespec);
    ostream << '\t' << std::stoi(key) + 1 << ". " << configurations.back().getName() << "\n";
		std::cout << "\nTranslated game " << gamespecfile << "\n\n";
  }
  ostream << "\n\n";
  welcoming_message = ostream.str();

  unsigned short port = serverspec["port"];
  Server server{port, getHTTPMessage(serverspec["indexhtml"]), onConnect, onDisconnect};


  // The game server main loop
  std::ostringstream buffer;
  while (true) {
    try {
      server.update();
    } catch (std::exception& e) {
      std::cerr << "Exception from Server update:\n"
                << " " << e.what() << "\n\n";
      break;
    }

    for (const auto& session: gameSessions) {
      // If the session runs a game in a separate thread, don't handle it's massages
      if(session->detached) {
        continue;
      }
      // Go over each player's messages, send them to other players in the session
      // or handle their /-commands
      for(auto it = session->players.begin(); it != session->players.end(); ) {
        Connection connection = it->first;
        const std::string& player_name = it->second;
        auto received = server.receive(connection);
        if (received.has_value()) {
          std::string message_text = std::move(received.value());

          if(message_text.size() > 0 && message_text[0] == '/') {
            if (message_text == "/quit") {
              buffer << "Player " << player_name << " has left the lobby\n\n";
              std::cout << "Session " << session->id << " has lost connection with " << player_name << std::endl;
              it = session->players.erase(it);
              sessionMap.erase(connection);
              server.disconnect(connection, false); 
              continue;
            }
            if (message_text.compare(0, 10, "/username ") == 0) {
              std::string name = message_text.substr(10);
              std::string response = session->change_username(name, connection);
              server.send({connection, response});
            }
            else if(connection == session->game_owner) {
              if (message_text == "/shutdown") {
                for (const auto& [connection, name] : session->players) {
                  server.send({connection, "This session has been shut down by the game owner\n\n"});
                  std::cout << "Session " << session->id << " has lost connection with " << name << std::endl;
                  sessionMap.erase(connection);
                  server.disconnect(connection, false); 
                }
                session->players.clear();
                break;
              }
              if (message_text.compare(0, 7, "/start ") == 0) {
                buffer << session->start(server, message_text.substr(7));
              }
              else {
                server.send({connection, "Invalid command\n\n"});
              }
            }
            else {
              server.send({connection, "Invalid command or insufficient permissions\n\n"});
            }
          }
          buffer << player_name << "> " << message_text << "\n\n";
        }
        ++it;
      }

      // Sessions withput players are considered shut down
      if(session->players.empty())
      {
        std::cout << "No players left. Shutting down session " << session->id << std::endl;
        while (session->detached) { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
        gameSessions.erase(std::remove_if(std::begin(gameSessions), std::end(gameSessions), [](const std::unique_ptr<GameSession>& session) { return session->players.empty(); }), std::end(gameSessions));
        continue;
      }

      // Send the collected messages to everyone in the session
      for(const auto& [connection, _] : session->players) {
        server.send({connection, buffer.str()});
      }
      buffer.str(""); // clear buffer
    }
    sleep(1);
  }

  return 0;
}