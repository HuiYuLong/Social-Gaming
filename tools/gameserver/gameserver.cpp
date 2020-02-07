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


using networking::Server;
using networking::Connection;
using networking::Message;
using networking::GameSession;
using networking::ConnectionHash;

/**
 *  Publicly available collection of sessions
 */
std::unordered_map<Connection, GameSession*, ConnectionHash> sessionMap;

std::vector<std::unique_ptr<GameSession>> gameSessions;

void
onConnect(Connection c, std::string_view target) {
  for (std::unique_ptr<GameSession>& gameSession : gameSessions)
  {
    if(target.compare(gameSession->invite_code) == 0)
    {
      gameSession->players.push_back(c);
      sessionMap[c] = gameSession.get();
      std::cout << "Session " << gameSession->id << " joined by " << c.id << std::endl;
      return;
    }
  }
  gameSessions.emplace_back(std::make_unique<GameSession>(c, target));
  sessionMap[c] = gameSessions.back().get();
  std::cout << "Session " << gameSessions.back()->id << " created by " << c.id << std::endl;
}


void
onDisconnect(Connection c) {
  // remove the connection from the session
  auto session = sessionMap.at(c);
  std::cout << "Session " << session->id << " has lost connection " << c.id << std::endl; 
  session->players.erase(std::remove(std::begin(session->players), std::end(session->players), c), std::end(session->players));
  if(session->players.empty())
  {
    std::cout << "No players left. Shutting down session " << session->id << std::endl;
    gameSessions.erase(std::remove_if(std::begin(gameSessions), std::end(gameSessions), [](std::unique_ptr<GameSession>& session) { return session->players.empty(); }), std::end(gameSessions));
  }
  sessionMap.erase(c);
}


std::deque<Message>
processMessages(Server& server, const std::deque<Message>& incoming) {
  for (auto& message : incoming) {
    if (message.text == "quit") {
      server.disconnect(message.connection);
    } else if (message.text == "shutdown") {
      GameSession* session = sessionMap[message.connection];
      if(session->gameOwner == message.connection)
      {
        for (Connection player : std::vector<Connection>(session->players))
          server.disconnect(player);
      }
    } else {
      sessionMap[message.connection]->communication << message.connection.id << "> " << message.text << "\n";
    }
  }

  std::deque<Message> outgoing;
  for (auto const [connection, gameSession] : sessionMap)
  {
    outgoing.push_back({connection, gameSession->communication.str()});
  }
  for (auto& session : gameSessions)
  {
    session->communication.str(""); // clear the stream
  }
  // std::vector<std::string> output(server.gameSessions.size());
  // std::transform(server.gameSessions.begin(), server.gameSessions.end(), output.begin(), [](GameSession& session) {
  //   std::string temp = session.communication.str();
  //   session.communication.str("");  // clear the stream
  //   return temp;
  // });
  return outgoing;
}


std::string
getHTTPMessage(const char* htmlLocation) {
  if (access(htmlLocation, R_OK ) != -1) {
    std::ifstream infile{htmlLocation};
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
  if (argc < 3) {
    std::cerr << "Usage:\n  " << argv[0] << " <port> <html response>\n"
              << "  e.g. " << argv[0] << " 4002 ./webchat.html\n";
    return 1;
  }

  unsigned short port = std::stoi(argv[1]);
  Server server{port, getHTTPMessage(argv[2]), onConnect, onDisconnect};

  while (true) {
    try {
      server.update();
    } catch (std::exception& e) {
      std::cerr << "Exception from Server update:\n"
                << " " << e.what() << "\n\n";
      break;
    }

    auto incoming = server.receive();
    auto outgoing = processMessages(server, incoming);
    server.send(outgoing);

    sleep(1);
  }

  return 0;
}