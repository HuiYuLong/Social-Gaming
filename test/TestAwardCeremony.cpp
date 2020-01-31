#include "Awards.h"
#include <list> 
#include <iterator>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class StubRankList : public awards::RankList {
private:
    std::list<std::string> names;
    int nameCount = 0;

public:
    StubRankList()
    {
        names.push_back("Carole");
        names.push_back("Travis");
        names.push_back("Mitchell");
        names.push_back("James");
        nameCount += 4;
    }

    std::string getNext() override {
        std::string name = names.front();
        names.pop_front();
        --nameCount;
        // Make sure that 
        if(nameCount < 1)
        {
            throw std::domain_error("Should be greater than 1");
        }
        return name;
    }

    int getCount()
    {
        return nameCount;
    }
}; // class StubRankList

// Mock for AwardCeremonyActions
class MockAwardCeremonyActions : public awards::AwardCeremonyActions {
public:
    MOCK_METHOD0(playAnthem, void());
    MOCK_METHOD0(turnOffTheLightsAndGoHome, void());
    MOCK_METHOD1(awardBronze, void(std::string));
    MOCK_METHOD1(awardSilver, void(std::string recipient));
    MOCK_METHOD1(awardGold, void(std::string recipient));
}; // class MockAwardCeremonyActions

// GTest
using ::testing::StrEq;         // Used to test string values after a function call
using ::testing::InSequence;    // Makes sure the function calls happen in sequence
TEST(AwardsTests, AwardsOrderTest){
    // Create mock and stubs
    MockAwardCeremonyActions awards;
    StubRankList stub;
    
    // Mock Testing
    InSequence seq;     // Function call happend in sequence
    EXPECT_CALL(awards, playAnthem()).Times(1);
    EXPECT_CALL(awards, awardBronze(StrEq("Carole"))).Times(1);
    EXPECT_CALL(awards, awardSilver(StrEq("Travis"))).Times(1);
    EXPECT_CALL(awards, awardGold(StrEq("Mitchell"))).Times(1);
    EXPECT_CALL(awards, turnOffTheLightsAndGoHome()).Times(1);

    // Run the awards ceremony
    performAwardCeremony(stub, awards);
    
    // Stub Testing
    EXPECT_EQ(stub.getCount(), 1);
    
}   // Test