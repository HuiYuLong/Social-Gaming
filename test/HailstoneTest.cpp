#include "Hailstone.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"


// Test 1: Input = 0
TEST(HailstoneTests, TestZero) {
    EXPECT_FALSE(sequence::satisfiesHailstone(0));
}

// Test 2: Input = 5
TEST(HailstoneTests, TestOtherThanZero) {
    EXPECT_TRUE(sequence::satisfiesHailstone(5));
}