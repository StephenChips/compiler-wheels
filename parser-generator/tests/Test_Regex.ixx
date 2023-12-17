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

TEST(Test_ConvertRegexToNFA_BasicRegexSyntax, match_epsilon) {
	auto actual = convertRegexToNFA("");
	auto expect = Automata{
			.stateGraph = {
				{ to(1, accepts(EPSILON)) },
				{}
			},
			.stateTypes = { INITIAL_STATE, ACCEPTING_STATE }
	};

	EXPECT_EQ(actual, expect);

	actual = convertRegexToNFA("()");
	expect = Automata{
			.stateGraph = {
				{ to(1, accepts(EPSILON)) },
				{}
			},
			.stateTypes = { INITIAL_STATE, ACCEPTING_STATE }
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA_BasicRegexSyntax, match_epsilon_or_a_character) {
	auto actual = convertRegexToNFA("a|");
	auto expect = Automata{
		.stateGraph = {
			{ to(1, accepts('a'))},
			{ eps(3) },
			{ eps(0), eps(4) },
			{},
			{ eps(5) },
			{ eps(3) }
		},
		.stateTypes = {
			PLAIN_STATE,
			PLAIN_STATE,
			INITIAL_STATE,
			ACCEPTING_STATE,
			PLAIN_STATE,
			PLAIN_STATE
		}
	};

	EXPECT_EQ(actual, expect);

	actual = convertRegexToNFA("|a");
	expect = Automata{
		.stateGraph = {
			{ eps(1) },
			{ eps(3) },
			{ eps(0), eps(4) },
			{},
			{ to(5, accepts('a')) },
			{ eps(3) }
		},
		.stateTypes = {
			PLAIN_STATE,
			PLAIN_STATE,
			INITIAL_STATE,
			ACCEPTING_STATE,
			PLAIN_STATE,
			PLAIN_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA_BasicRegexSyntax, match_multiple_epsilon_OR_branches) {
	auto actual = convertRegexToNFA("a|||b|");
	auto expect = Automata{
		.stateGraph = {
			{ to(1, accepts('a'))},
			{ eps(3) },
			{ eps(0), eps(4), eps(6), eps(8), eps(10) },
			{},
			{ eps(5) },
			{ eps(3) },
			{ eps(7) },
			{ eps(3) },
			{ to(9, accepts('b')) },
			{ eps(3) },
			{ eps(11) },
			{ eps(3) }
		},
		.stateTypes = {
			PLAIN_STATE,
			PLAIN_STATE,
			INITIAL_STATE,
			ACCEPTING_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA, match_a_single_character) {
	auto actual = convertRegexToNFA("a");
	auto expect = Automata{
			.stateGraph = {
				{ to(1, accepts('a')) },
				{}
			},
			.stateTypes = { INITIAL_STATE, ACCEPTING_STATE }
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA, match_a_word) {
	auto actual = convertRegexToNFA("while");
	auto expect = Automata {
			.stateGraph = {
				{ to(1, accepts('w')) },
				{ to(2, accepts(EPSILON)) },
				{ to(3, accepts('h')) },
				{ to(4, accepts(EPSILON)) },
				{ to(5, accepts('i')) },
				{ to(6, accepts(EPSILON)) },
				{ to(7, accepts('l')) },
				{ to(8, accepts(EPSILON)) },
				{ to(9, accepts('e')) },
				{}
			},
			.stateTypes = {
				INITIAL_STATE,
				PLAIN_STATE,
				PLAIN_STATE,
				PLAIN_STATE,
				PLAIN_STATE,
				PLAIN_STATE,
				PLAIN_STATE,
				PLAIN_STATE,
				PLAIN_STATE,
				ACCEPTING_STATE
			}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA, match_any_of_two_words) {
	auto actual = convertRegexToNFA("cat|tea");
	auto expect = Automata{
		.stateGraph = {
			{ to(1, accepts('c'))},
		    { to(2, accepts(EPSILON)) },
		    { to(3, accepts('a')) },
		    { to(4, accepts(EPSILON)) },
		    { to(5, accepts('t')) },
			{ to(7, accepts(EPSILON)) },
			{ to(0, accepts(EPSILON)), to(8, accepts(EPSILON)) },
			{},
			{ to(9, accepts('t')) },
			{ to(10, accepts(EPSILON)) },
			{ to(11, accepts('e')) },
			{ to(12, accepts(EPSILON)) },
			{ to(13, accepts('a')) },
			{ to(7, accepts(EPSILON)) },
		},
		.stateTypes = {
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			INITIAL_STATE,
			ACCEPTING_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA, match_regex_in_a_pair_of_parentheses) {
	auto actual = convertRegexToNFA("c(a|b)");
	auto expect = Automata{
		.stateGraph = {
			{ to(1, accepts('c')) },
			{ to(4, accepts(EPSILON)) },
			{ to(3, accepts('a')) },
			{ to(5, accepts(EPSILON)) },
			{ to(2, accepts(EPSILON)), to(6, accepts(EPSILON)) },
			{},
			{ to(7, accepts('b')) },
			{ to(5, accepts(EPSILON)) }
		},
		.stateTypes = {
			INITIAL_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			ACCEPTING_STATE,
			PLAIN_STATE,
			PLAIN_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA, repeats_any_number_of_times) {
	auto actual = convertRegexToNFA("a*");
	auto expect = Automata{
		.stateGraph = {
			{ to(1, accepts('a')) },
			{ to(3, accepts(EPSILON)), to(0, accepts(EPSILON)) },
			{ to(3, accepts(EPSILON)), to(0, accepts(EPSILON)) },
			{},
		},
		.stateTypes = {
			PLAIN_STATE,
			PLAIN_STATE,
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA, matches_at_least_once) {
	auto actual = convertRegexToNFA("a+");
	Automata expect{
		.stateGraph = {
			{ to(1, accepts('a'))},
			{ to(4, accepts(EPSILON)) },
			{ to(3, accepts('a')) },
			{ to(5, accepts(EPSILON)), to(2, accepts(EPSILON)) },
			{ to(5, accepts(EPSILON)), to(2, accepts(EPSILON)) },
			{}
		},
		.stateTypes = {
			INITIAL_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA, optional) {
	auto actual = convertRegexToNFA("a?");
	Automata expect{
		.stateGraph = {
			{ to(1, accepts({'a', EPSILON})) },
			{}
		},
		.stateTypes = { INITIAL_STATE, ACCEPTING_STATE }
	};

	EXPECT_EQ(actual, expect);
}


TEST(Test_ConvertRegexToNFA, consecutive_qualiders_are_invalid) {
	const std::vector<std::string> listOfRegexes = {
		"a**", "a*+", "a*?", "a++", "a+*", "a+?", "a??", "a?*", "a?+"
	};

	for (const auto& regex : listOfRegexes) {
		try {
			convertRegexToNFA(regex);
			FAIL() << std::format("Invalid regex /{}/ should cause an exception but it's not thrown.", regex);
		}
		catch (std::runtime_error e) {
			EXPECT_STREQ(e.what(), "Consecutive qualifiers is not valid.");
		}
	}
}
TEST(Test_ConvertRegexToNFA, leading_qualiders_are_invalid) {
	const std::vector<std::string> listOfRegexes = {
		"*a", "{3}a", "?a", "+a", "(*abc)", "(?bc)"
	};

	for (const auto& regex : listOfRegexes) {
		try {
			convertRegexToNFA(regex);
			FAIL() << std::format("Invalid regex /{}/ should've caused an exception but haven't.", regex);
		}
		catch (std::runtime_error e) {
			EXPECT_STREQ(e.what(), 
				"A qualifier cannot be the first character of a regex, "
				"it must be placed after a character, a right parenthesis or a right bracket.\n"
				"For example: a*, (abc)* and [abc]*"
			);
		}
	}
}

TEST(Test_ConvertRegexToNFA, exactly_repeats_N_times) {
	auto actual = convertRegexToNFA("a{5}");
	auto expect = convertRegexToNFA("aaaaa");
	EXPECT_EQ(actual, expect);
}


TEST(Test_ConvertRegexToNFA, repeats_at_least_N_times) {
	EXPECT_EQ(
		convertRegexToNFA("a{5, }"),
		convertRegexToNFA("aaaaaa*")
	);
}

TEST(Test_ConvertRegexToNFA, repeats_N_to_M_times) {
	auto actual = convertRegexToNFA("a{1, 5}");
	Automata expect{
		.stateGraph = {
			{ to(1, accepts('a'))},
			{ eps(2) },
			{ to(3, accepts('a')), eps(9)},
			{ eps(4) },
			{ to(5, accepts('a')), eps(9)},
			{ eps(6) },
			{ to(7, accepts('a')), eps(9)},
			{ eps(8) },
			{ to(9, accepts({'a', EPSILON}))},
			{}
		},
		.stateTypes = {
			INITIAL_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			ACCEPTING_STATE
		}
	};
	EXPECT_EQ(actual, expect);
}