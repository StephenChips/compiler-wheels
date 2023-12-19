module;

#include <gtest/gtest.h>
#include <string>
#include <format>;

export module Test_Regex;
import Regex;

TEST(Test_Regex, test_match_single_character) {
	FAIL();
	Regex pattern("abc");

	std::string str1("abcd");
	auto result = pattern.match(str1.cbegin());
	EXPECT_TRUE(result.succeeded);
	EXPECT_EQ(result.endIter, str1.cbegin() + 3);

	std::string str2("dcba");
	result = pattern.match(str2.cbegin());
	EXPECT_FALSE(result.succeeded);
	EXPECT_EQ(result.endIter, str2.cbegin());
}

