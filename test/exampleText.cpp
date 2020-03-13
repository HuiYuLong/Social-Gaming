#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Server.h"
#include "translator.h"
#include <nlohmann/json.hpp>

using ::testing::AtLeast;

// Test Example
// "MyDumbTest" is the test group name
// "TestFalse" is the test name
// You can have multiple tests within a test group

// EXPECT_*() will continue to run the other tests even if that test fails
// ASSERT_*() will stop other tests from running

// ... should we instantiate a server here?
// nick sumner's recommendation is that we should move the Server out of run() method
// TODO: research the way that we could move the server out of run()


class MockAddRule : public AddRule {
public:
	MOCK_METHOD2(run, void(Server& server, GameState& state));
};

TEST(MyDumbTest, TestFalse) {
    EXPECT_FALSE(3*2 == 5);
    ASSERT_TRUE(true);
}

// TEST(JunhosDumbRuleTest, Test1) {
// 	MockAddRule mockAddRule = std::make_unique<MockAddRule>(server, state);
// 	mockAddRule 
// }

