#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Server.h"
#include "translator.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>

using ::testing::AtLeast;

using networking::Server;
using networking::Connection;
using networking::Message;
using networking::ConnectionHash;
using json = nlohmann::json;

using Name2Connection = std::unordered_map<std::string, Connection>;

class PseudoServer : public Server {
public:
	PseudoServer()  {};
	void send(const Message& message) {
		this->messages.push_back(message); // just check if the correct message was pushed to msg vector
	}
	int getMsgVectorSize() {
		return this->messages.size();
	}

private:
	std::vector<Message> messages;
};

class MockGlobalMessageRule : public GlobalMessageRule {
public:
	MOCK_METHOD2(run, void(Server& server, GameState& state));
};

class MockAddRule : public AddRule {
public:
	MOCK_METHOD2(run, void(Server& server, GameState& state));
};

TEST(RuleTests, GlobalMessageRuleTest) {

	// initialize environment
	// rule take in Server and GameState

	std::vector<Configuration> configurations;

	std::ifstream serverconfig{"../configs/server/ruleTestConfig.json"};
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

	PseudoServer pseudoServer;
	Server& server = pseudoServer;
	Name2Connection name2connection; // maps string to connection

	Connection player1 = Connection{1};
	Connection player2 = Connection{2};

	name2connection.insert({"player1", player1});
	name2connection.insert({"player2", player2});

	std::unique_ptr<GameState> game_state = std::make_unique<GameState>(configurations[0], name2connection, player1);

	configurations[0].launchGame(server, *game_state);

	std::cout << "\n" << configurations.size() << std::endl; 

	// EXPECT_FALSE(3*2 == 5);
	// ASSERT_TRUE(true);
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


