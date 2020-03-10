/////////////////////////////////////////////////////////////////////////////
//                         Single Threaded Networking
//
// This file is distributed under the MIT License. See the LICENSE file
// for details.
/////////////////////////////////////////////////////////////////////////////


#include "Server.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <nlohmann/json.hpp>

using networking::Server;
using networking::Connection;
using networking::Message;
using networking::GameSession;
using networking::ConnectionHash;

using json = nlohmann::json;

/**
 *  Publicly available collection of sessions
 */
std::unordered_map<Connection, GameSession*, ConnectionHash> sessionMap;

std::vector<std::unique_ptr<GameSession>> gameSessions;

void
onConnect(Connection c, std::string_view target) {
  auto gameSessionIter = std::find_if(gameSessions.begin(), gameSessions.end(), [target](std::unique_ptr<GameSession>& gameSession) {
    return target.compare(gameSession->invite_code) == 0;
  });
  if (gameSessionIter != gameSessions.end()) {
    auto& gameSession = *gameSessionIter;
    gameSession->players.push_back(c);
    sessionMap[c] = gameSession.get();
    std::cout << "Session " << gameSession->id << " joined by " << c.id << std::endl;
    return;
  }
  gameSessions.emplace_back(std::make_unique<GameSession>(c, target));
  sessionMap[c] = gameSessions.back().get();
  std::cout << "Session " << gameSessions.back()->id << " created by " << c.id << std::endl;
}


void
onDisconnect(Connection c) {
  // remove the connection from the session
  auto session = sessionMap.at(c);
  sessionMap.erase(c);
  std::cout << "Session " << session->id << " has lost connection " << c.id << std::endl; 
  session->players.erase(std::remove(std::begin(session->players), std::end(session->players), c), std::end(session->players));
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

  std::ifstream config{argv[1]};
  json j = json::parse(config);

  unsigned short port = j["port"];
  Server server{port, getHTTPMessage(j["indexhtml"]), onConnect, onDisconnect};
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
      for(auto it = session->players.begin(); it != session->players.end(); ) {
        Connection connection = *it;
        auto received = server.receive(connection);
        if (received.has_value()) {
          std::string message_text = std::move(received.value());
        
          if (message_text == "quit") {
            it = session->players.erase(it);
            sessionMap.erase(connection);
            server.disconnect(connection, false);
            std::cout << "Session " << session->id << " has lost connection " << connection.id << std::endl; 
            continue;
          }
          if (message_text == "shutdown" && session->gameOwner == connection) {
            for (Connection player : session->players) {
              sessionMap.erase(player);
              server.disconnect(player, false);
              std::cout << "Session " << session->id << " has lost connection " << connection.id << std::endl; 
            }
            session->players.clear();
            break;
          }
          buffer << connection.id << "> " << message_text << "\n";
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