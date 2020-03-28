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

template<typename Iter>
std::string select_random_animal() {
    static std::vector<std::string> random_animals = {"Alligator", "Anteater", "Armadillo", "Auroch", "Axolotl", "Badger", "Bat", "Bear", "Beaver", "Buffalo", "Camel", "Capybara", "Chameleon", "Cheetah", "Chinchilla", "Chipmunk", "Chupacabra", "Cormorant", "Coyote", "Crow", "Dingo", "Dinosaur", "Dog", "Dolphin", "Duck", "Elephant", "Ferret", "Fox", "Frog", "Giraffe", "Gopher", "Grizzly", "Hedgehog", "Hippo", "Hyena", "Ibex", "Ifrit", "Iguana", "Jackal", "Kangaroo", "Koala", "Kraken", "Lemur", "Leopard", "Liger", "Lion", "Llama", "Loris", "Manatee", "Mink", "Monkey", "Moose", "Narwhal", "Nyan Cat", "Orangutan", "Otter", "Panda", "Penguin", "Platypus", "Pumpkin", "Python", "Quagga", "Rabbit", "Raccoon", "Rhino", "Sheep", "Shrew", "Skunk", "Squirrel", "Tiger", "Turtle", "Walrus", "Wolf", "Wolverine", "Wombat"};
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, random_animals.size() - 1);
    return random_animals[dis(gen)];
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
  // Connections of players in this game session
  std::vector<Connection> players;
  // Part of the job of the game session is to set up the game state that is used by the interpreter
  std::unique_ptr<GameState> game_state;
  // Mapping of in-game player names to their connections created with the /username command
  // Will be passed to the interpreter through the game state so the interpreter can send and receive messages via the server
  Name2Connection name2connection;
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
    { players.push_back(game_owner); }

  bool
  operator==(const GameSession& other) const
  {
    return id == other.id;
  }

  // Adds the username to the name2connection mapping
  std::string
  register_username(const std::string& name, Connection connection)
  {
    if(name.size() == 0) {
      return "Don't be shy, enter a real name\n\n";
    }
    if (name2connection.find(name) != name2connection.end()) {
      return "This name is already used\n\n";
    }
    remove_username_of(connection);
    name2connection[name] = connection;
    return "Changed the username to " + name + "\n\n";
  }

  std::string
  register_configuration(const std::string& game_name)
  {
    auto game = std::find_if(configurations.begin(), configurations.end(),
      [&game_name](const Configuration& conf) {
      return game_name == conf.getName();
    });
    if (game != configurations.end()) {
      configuration = &(*game);
      return "Successfully changed the game to " + game->getName() + "\n\n";
    }
    else {
      return "Could not find a game with the given name\n\n";
    }
  }

  // Removes the username of a given connectionfrom name2connection 
  bool
  remove_username_of(Connection connection)
  {
    auto found = std::find_if(name2connection.begin(), name2connection.end(), [connection](const std::pair<std::string, Connection>& iter) {
      return connection == iter.second;
    });
    if(found != name2connection.end()) {
      name2connection.erase(found);
      return true;
    }
    return false;
  }

  // Indicates whether all the necessary fields of the game session are set up properly
  // and the game session is ready to start the game
  std::pair<std::string, bool>
  validate()
  {
    if (configuration == nullptr) {
      return {"Please /select a game from the list\n\n", false};
    }
    if (players.size() > configuration->getPlayerCountMax()) {
      std::ostringstream ostream;
      ostream << "Too many players for this game. You need to evict "
        << configuration->getPlayerCountMax() - players.size() << " player(s)\n\n";
      return {ostream.str(), false};
    }
    if (players.size() < configuration->getPlayerCountMin()) {
      std::ostringstream ostream;
      ostream << "This game requires at least " << configuration->getPlayerCountMin() << "people to play, but only "
        << players.size() << " are present in the lobby\n\n";
      return {ostream.str(), false};
    }
    if (players.size() != name2connection.size()) {
      return {"Some of the players haven't named themselves yet\n\n", false};
    }
    return {"Starting the game...\n\n", true};
  }

  std::string
  start(Server& server)
  {
    auto [notice, good_to_go] = validate();
    if(good_to_go) {
      game_state = std::make_unique<GameState>(*configuration, name2connection);
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
          std::cout << "Warning: a game thread has exited with an exception" << std::endl;
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
    gameSession->players.push_back(c);
    sessionMap[c] = gameSession.get();
    std::cout << "Session " << gameSession->id << " joined by " << c.id << std::endl;
    for(Connection connection : gameSession->players) {
      server.send({connection, "Player " + std::to_string(connection.id) + " has joined the lobby\n\n"});
    }
  }
  else {
    gameSessions.emplace_back(std::make_unique<GameSession>(c, target));
    sessionMap[c] = gameSessions.back().get();
    std::cout << "Session " << gameSessions.back()->id << " created by " << c.id << std::endl;
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
  std::cout << "Session " << session->id << " has lost connection " << c.id << std::endl;
  for(Connection connection : session->players) {
    server.send({connection, "Player " + std::to_string(connection.id) + " has left\n\n"});
  }
  session->players.erase(std::remove(std::begin(session->players), std::end(session->players), c), std::end(session->players));
  session->remove_username_of(c);
  if(session->players.empty())
  {
    std::cout << "No players left. Shutting down session " << session->id << std::endl;
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
/username &lt;name&gt; - choose yourself an in-game name\n\
/select &lt;game&gt; - choose a game to play from the list below\n\
/start - when you are ready\n\
/quit - if you need to leave\n\
/shutdown - close the game\n\n\nAvailable games:\n\n";
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
        Connection connection = *it;
        auto received = server.receive(connection);
        if (received.has_value()) {
          std::string message_text = std::move(received.value());

          if(message_text.size() > 0 && message_text[0] == '/') {
            if (message_text == "/quit") {
              buffer << "Player " << connection.id << " has left the lobby\n\n";
              it = session->players.erase(it);
              session->remove_username_of(connection);
              sessionMap.erase(connection);
              server.disconnect(connection, false);
              std::cout << "Session " << session->id << " has lost connection " << connection.id << std::endl; 
              continue;
            }
            if (message_text.compare(0, 10, "/username ") == 0) {
              std::string name = message_text.substr(10);
              std::string response = session->register_username(name, connection);
              server.send({connection, response});
            }
            else if(connection == session->game_owner) {
              if (message_text == "/shutdown") {
                for (Connection connection : session->players) {
                  server.send({connection, "This session has been shut down by the game owner\n\n"});
                  sessionMap.erase(connection);
                  server.disconnect(connection, false);
                  std::cout << "Session " << session->id << " has lost connection " << connection.id << std::endl; 
                }
                session->players.clear();
                break;
              }
              if (message_text.compare(0, 8, "/select ") == 0) {
                buffer << session->register_configuration(message_text.substr(8));
              }
              else if (message_text == "/start") {
                buffer << session->start(server);
              }
              else {
                server.send({connection, "Invalid command\n\n"});
              }
            }
            else {
              server.send({connection, "Invalid command or insufficient permissions\n\n"});
            }
          }
          buffer << connection.id << "> " << message_text << "\n\n";
        }
        ++it;
      }

      // Sessions withput players are considered shut down
      if(session->players.empty())
      {
        std::cout << "No players left. Shutting down session " << session->id << std::endl;
        gameSessions.erase(std::remove_if(std::begin(gameSessions), std::end(gameSessions), [](const std::unique_ptr<GameSession>& session) { return session->players.empty(); }), std::end(gameSessions));
        continue;
      }

      // Send the collected messages to everyone in the session
      for(Connection connection: session->players) {
        server.send({connection, buffer.str()});
      }
      buffer.str(""); // clear buffer
    }
    sleep(1);
  }

  return 0;
}