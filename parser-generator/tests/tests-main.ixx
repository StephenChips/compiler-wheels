module;

#include <gtest/gtest.h>

export module Tests;

import ParserGenerator;

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
	EXPECT_EQ(true, true);
}
