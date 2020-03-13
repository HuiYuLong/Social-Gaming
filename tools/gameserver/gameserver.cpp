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

using networking::Server;
using networking::Connection;
using networking::Message;
using networking::ConnectionHash;

using json = nlohmann::json;

//Connection DISCONNECTED{reinterpret_cast<uintptr_t>(nullptr)};

/**
 * Since the server should be able to handle multiple games,
 * some identifier is needed to distinguish different connections.
 * Each game corresponds to a session, and each connection of the game belongs to that session.
 */
struct GameSession {
  uintptr_t id;
  Connection game_owner;
  std::string invite_code;
  std::vector<Connection> players;
  std::unique_ptr<GameState> game_state;
  Name2Connection name2connection;
  Configuration* configuration;
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

  std::string
  register_username(const std::string& name, Connection connection)
  {
    if(name.size() == 0) {
      return "Invalid command\n\n";
    }
    if (name2connection.find(name) != name2connection.end()) {
      return "This name is already used\n\n";
    }
    remove_username_of(connection);
    name2connection[name] = connection;
    return "Changed the username to " + name + "\n\n";
  }

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

  // runs in parallel
  void
  operator()(Server& server)
  {
    detached = true;
    std::cout << "Session " << id << " is set free" << std::endl;
    game_state = std::make_unique<GameState>(*configuration, this->name2connection);
    try {
      configuration->launchGame(server, *game_state);
    }
    catch (std::out_of_range& e) {
      std::cout << "One of the players has disconnected while the game was on" << std::endl;
    }
    detached = false;
    std::cout << "Session's " << id << " thread is finished" << std::endl;
  }
};

/**
 *  Publicly available collection of sessions
 */
std::unordered_map<Connection, GameSession*, ConnectionHash> sessionMap;

std::vector<std::unique_ptr<GameSession>> gameSessions;

std::vector<Configuration> configurations;

const std::string welcoming_message = "Welcome to the Social Game Engine!\n\n\
/username &lt;name&gt; - choose yourself an in-game name\n\
/select &lt;game&gt; - choose a game to play from the list below\n\
/start - when you are ready\n\
/quit - if you need to leave\n\
/shutdown - close the game\n\n\n";

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

  std::ostringstream ostream;
  ostream << "Available games:\n\n";
  for (size_t i = 0u, end = configurations.size(); i < end; i++) {
    ostream << '\t' << i << ". " << configurations[i].getName() << "\n";
  }
  ostream << "\n\n";
  server.send({c, welcoming_message});
  server.send({c, ostream.str()});
}


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

  for ([[maybe_unused]] const auto& [key, gamespecfile]: serverspec["games"].items())
	{
		std::ifstream gamespecstream{std::string(gamespecfile)};
		if (gamespecstream.fail()) {
			std::cout << "Could not open the game configuration file " << gamespecfile << std::endl;
			return 1;
		}
    
		json gamespec = json::parse(gamespecstream);
		configurations.emplace_back(gamespec);
		std::cout << "\nTranslated game " << gamespecfile << "\n\n";
  }

  unsigned short port = serverspec["port"];
  Server server{port, getHTTPMessage(serverspec["indexhtml"]), onConnect, onDisconnect};

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
      if(session->detached) {
        continue;
      }
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
                auto game = std::find_if(configurations.begin(), configurations.end(),
                  [game_name = std::move(message_text.substr(8))](const Configuration& conf) {
                  return game_name == conf.getName();
                });
                if (game != configurations.end()) {
                  session->configuration = &(*game);
                  server.send({connection, "Successfully changed the game to " + game->getName() + "\n\n"});
                }
                else {
                  server.send({connection, "Could not find a game with the given name\n\n"});
                }
              }
              else if (message_text == "/start") {
                auto [notice, good_to_go] = session->validate();
                buffer << notice;
                if(good_to_go) {
                  std::thread t(std::ref(*session), std::ref(server));
                  t.detach();
                }
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

      if(session->players.empty())
      {
        std::cout << "No players left. Shutting down session " << session->id << std::endl;
        gameSessions.erase(std::remove_if(std::begin(gameSessions), std::end(gameSessions), [](const std::unique_ptr<GameSession>& session) { return session->players.empty(); }), std::end(gameSessions));
        continue;
      }

      for(Connection connection: session->players) {
        server.send({connection, buffer.str()});
      }
      buffer.str(""); // clear buffer
    }
    sleep(1);
  }

  return 0;
}