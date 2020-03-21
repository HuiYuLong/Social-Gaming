#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Server.h"
#include "translator.h"
#include <nlohmann/json.hpp>
#include <vector>

using ::testing::AtLeast;

using networking::Server;
using networking::Connection;
using networking::Message;
using networking::ConnectionHash;
using json = nlohmann::json;

class PseudoServer : public Server {
public:
	PseudoServer()  {};
	void send(const Message& message) {
		this->messages.push_back(message); // just check if the correct message was pushed to Server
	}

private:
	std::vector<Message> messages;
};


// void GlobalMessageRule::run(Server& server, GameState& state)
// {
// 	List& players = boost::get<List>(boost::get<Map>(state.getVariables())["players"]);
// 	for (Variable& player : players) {
// 		const std::string& name = boost::get<std::string>(boost::get<Map>(player)["name"]);
// 		server.send({state.getConnectionByName(name), value.fill_with(state.getVariables()) });
// 	}
// }

class MockGlobalMessageRule : public GlobalMessageRule {
public:
	MOCK_METHOD2(run, void(Server& server, GameState& state));
};

class MockAddRule : public AddRule {
public:
	MOCK_METHOD2(run, void(Server& server, GameState& state));
};

// TEST(MyDumbTest, TestFalse) {
//     EXPECT_FALSE(3*2 == 5);
//     ASSERT_TRUE(true);
// }

TEST(RuleTest, GlobalMessageRuleTest) {

	// initialize environment

	std::vector<Configuration> configurations;

	std::ifstream serverconfig{"../configs/server/testing.json"};
 	if (serverconfig.fail()) {
 		std::cout << "Could not open the rule testing configuration file" << std::endl;
  }

	json serverspec = json::parse(serverconfig);
	configurations.reserve(serverspec["games"].size());

	for ([[maybe_unused]] const auto& [key, gamespecfile]: serverspec["games"].items()) {
		std::ifstream gamespecstream{std::string(gamespecfile)};
		if (gamespecstream.fail()) {
			std::cout << "Could not open the game configuration file " << gamespecfile << std::endl;
		}
    
		json gamespec = json::parse(gamespecstream);
		configurations.emplace_back(gamespec);
		std::cout << "\nTranslated game " << gamespecfile << "\n\n";
	}

  //Server server{port, getHTTPMessage(serverspec["indexhtml"]), onConnect, onDisconnect};
  //PseudoServer pseudoServer{port, getHTTPMessage(serverspec["indexhtml"]), onConnect, onDisconnect};
  //PseudoServer pseudoServer{randPortNum, randStr, onConnect, onDisconnect};
	PseudoServer pseudoServer;
	Server& server = pseudoServer;


	EXPECT_FALSE(3*2 == 5);
	ASSERT_TRUE(true);
}



// Test Example
// "MyDumbTest" is the test group name
// "TestFalse" is the test name
// You can have multiple tests within a test group

// EXPECT_*() will continue to run the other tests even if that test fails
// ASSERT_*() will stop other tests from running
// ... should we instantiate a server here?
// nick sumner's recommendation is that we should move the Server out of run() method
// TODO: research the way that we could move the server out of run()


