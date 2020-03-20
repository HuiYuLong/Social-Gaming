#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Server.h"
#include "translator.h"

#include <nlohmann/json.hpp>
#include <vector>

using networking::Server;
using networking::Connection;
using networking::Message;
using networking::ConnectionHash;

using json = nlohmann::json;

void onConnect();
void onDisconnect();

class PseudoServer : public Server {
public:
	void send(const Message& message) {
		this->messages.push_back(message);
	}
private:
	std::vector<Message> messages;
};

class MockAddRule : public AddRule {
public:
	MOCK_METHOD2(run, void(Server& server, GameState& state));
};


TEST(MyDumbTest, TestFalse) {
    EXPECT_FALSE(3*2 == 5);
    ASSERT_TRUE(true);
}


// Test Example
// "MyDumbTest" is the test group name
// "TestFalse" is the test name
// You can have multiple tests within a test group

// EXPECT_*() will continue to run the other tests even if that test fails
// ASSERT_*() will stop other tests from running

// we'll need a test.json
// build a GameState and a Server

// pseudoserver with a vector of messages 