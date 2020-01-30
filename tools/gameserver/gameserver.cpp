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


std::vector<Connection> clients;


void
onConnect(Connection c) {
  std::cout << "New connection found: " << c.id << "\n";
  clients.push_back(c);
}


void
onDisconnect(Connection c) {
  std::cout << "Connection lost: " << c.id << "\n";
  auto eraseBegin = std::remove(std::begin(clients), std::end(clients), c);
  clients.erase(eraseBegin, std::end(clients));
}


std::deque<Message>
processMessages(Server& server, const std::deque<Message>& incoming) {
  for (auto& message : incoming) {
    if (message.text == "quit") {
      server.disconnect(message.connection);
    } else if (message.text == "shutdown") {
      GameSession* session = server.sessionMap[message.connection];
      if(session->gameOwner == message.connection)
      {
        std::cout << "Shutting down session " << session->id << std::endl;
        for (Connection player : session->players)
          server.disconnect(player);
      }
    } else {
      server.sessionMap[message.connection]->communication << message.connection.id << "> " << message.text << "\n";
    }
  }

  std::deque<Message> outgoing;
  for (auto client : clients)
  {
    outgoing.push_back({client, server.sessionMap[client]->communication.str()});
  }
  for (auto& session : server.gameSessions)
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