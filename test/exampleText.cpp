#include "gtest/gtest.h"
#include "gmock/gmock.h"



// Test Example
// "MyDumbTest" is the test group name
// "TestFalse" is the test name
// You can have multiple tests within a test group

// EXPECT_*() will continue to run the other tests even if that test fails
// ASSERT_*() will stop other tests from running
TEST(MyDumbTest, TestFalse) {
    EXPECT_FALSE(3*2 == 5);
    ASSERT_TRUE(true);
}