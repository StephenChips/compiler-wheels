module;

#include <gtest/gtest.h>
#include <string>
#include <format>;

export module Test_ConvertRegexToNFA;
import ConvertRegexToNFA;


/*
 * Following section of test cases test basic regexes, those contains
 * only "three fundamentals": concatenation, alternative and kleene
 * closure.
 */

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

TEST(Test_ConvertRegexToNFA_BasicRegexSyntax, match_a_single_character) {
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

TEST(Test_ConvertRegexToNFA_BasicRegexSyntax, match_a_word) {
	auto actual = convertRegexToNFA("while");
	auto expect = Automata{
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

TEST(Test_ConvertRegexToNFA_BasicRegexSyntax, match_any_of_two_words) {
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

TEST(Test_ConvertRegexToNFA_BasicRegexSyntax, match_regex_in_a_pair_of_parentheses) {
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

TEST(Test_ConvertRegexToNFA_BasicRegexSyntax, repeats_any_number_of_times) {
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

/*
* Following section of test cases test non-basic regexes, those contains features
* that are not concatenation, alternative or kleene closure. For example:
* /a+/, /a{1, 5}/
*
* The "expected" automata may be generated from a equivalent basic regexes.
* As long as we have sufficient tests on the basic regexes, which garentee
* the function can return a correct automata when parsing a basic regex,
* we are confident to use it for testing any non-basic regexes.
*/

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, matches_any_character) {
	auto actual = convertRegexToNFA(".");
	Automata expect{
		.stateGraph = {
			{ to(1, acceptsAnyCharacter()) },
			{}
		},
		.stateTypes = { INITIAL_STATE, ACCEPTING_STATE }
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, matches_at_least_once) {
	auto actual = convertRegexToNFA("a+");
	Automata expect{
		.stateGraph = {
			{ to(1, accepts('a'))},
			{ to(4, accepts(EPSILON)) },
			{ to(3, accepts('a')) },
			{ to(5, accepts(EPSILON)),
			to(2, accepts(EPSILON)) },
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

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, optional) {
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


TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, consecutive_qualiders_are_invalid) {
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
TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, leading_qualiders_are_invalid) {
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
				"For example: /a*/, /(abc)+/ and /[abc]{3}/"
			);
		}
	}
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, exactly_repeats_N_times) {
	auto actual = convertRegexToNFA("a{5}");
	auto expect = convertRegexToNFA("aaaaa");
	EXPECT_EQ(actual, expect);
}


TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, repeats_at_least_N_times) {
	EXPECT_EQ(
		convertRegexToNFA("a{5, }"),
		convertRegexToNFA("aaaaaa*")
	);
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, repeats_N_to_M_times) {
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

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, incomplete_repetition_should_be_treat_as_concatenation) {
	auto actual = convertRegexToNFA("{");
	auto expect = Automata{
		.stateGraph {
			{ to(1, accepts('{')) },
			{}
		},

		.stateTypes {
			INITIAL_STATE,
			ACCEPTING_STATE,
		}
	};

	EXPECT_EQ(actual, expect);

	actual = convertRegexToNFA("{1");
	expect = Automata{
		.stateGraph {
			{ to(1, accepts('{')) },
			{ eps(2) },
			{ to(3, accepts('1')) },
			{}
		},

		.stateTypes {
			INITIAL_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			ACCEPTING_STATE,
		}
	};

	EXPECT_EQ(actual, expect);

	actual = convertRegexToNFA("{1, ");
	expect = Automata{
		.stateGraph {
			{ to(1, accepts('{')) },
			{ eps(2) },
			{ to(3, accepts('1')) },
			{ eps(4) },
			{ to(5, accepts(',')) },
			{ eps(6) },
			{ to(7, accepts(' ')) },
			{}
		},

		.stateTypes {
			INITIAL_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			ACCEPTING_STATE,
		}
	};

	EXPECT_EQ(actual, expect);

	actual = convertRegexToNFA("{1, 3");
	expect = {
		.stateGraph {
			{ to(1, accepts('{')) },
			{ eps(2) },
			{ to(3, accepts('1')) },
			{ eps(4) },
			{ to(5, accepts(',')) },
			{ eps(6) },
			{ to(7, accepts(' ')) },
			{ eps(8) },
			{ to(9, accepts('3')) },
			{},
		},

		.stateTypes {
			INITIAL_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			ACCEPTING_STATE,
		}
	};

	EXPECT_EQ(actual, expect);


	actual = convertRegexToNFA("{1, 3a");
	expect = {
		.stateGraph {
			{ to(1, accepts('{')) },
			{ eps(2) },
			{ to(3, accepts('1')) },
			{ eps(4) },
			{ to(5, accepts(',')) },
			{ eps(6) },
			{ to(7, accepts(' ')) },
			{ eps(8) },
			{ to(9, accepts('3')) },
			{ eps(10) },
			{ to(11, accepts('a')) },
			{}
		},

		.stateTypes {
			INITIAL_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			ACCEPTING_STATE,
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, match_a_set_of_characters) {
	auto actual = convertRegexToNFA("[abc]");
	Automata expect = {
		.stateGraph = {
			{ to(1, accepts({ 'a', 'b', 'c' }))},
			{}
		},
		.stateTypes = {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, match_a_range_of_characters) {
	auto actual = convertRegexToNFA("[a-c]");
	Automata expect = {
		.stateGraph = {
			{ to(1, accepts(Transition::Range{'a', 'c'}))},
			{}
		},
		.stateTypes = {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, match_a_set_and_a_range_of_characters) {
	std::vector<std::string> patterns{ "[abcA-C]", "[aA-Cbc]", "[A-Cabc]" };

	for (const auto& pattern : patterns) {
		auto actual = convertRegexToNFA(pattern);
		Automata expect = {
			.stateGraph = {
				{ to(1, accepts({ Transition::Range{'A', 'C'}, 'a', 'b', 'c' }))},
				{}
			},
			.stateTypes = {
				INITIAL_STATE,
				ACCEPTING_STATE
			}
		};

		EXPECT_EQ(actual, expect);
	}
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, match_any_except_a_set_of_characters) {
	auto actual = convertRegexToNFA("[^abc]");
	Automata expect = {
		.stateGraph = {
			{ to(1, acceptsAnyExcept({ 'a', 'b', 'c' }))},
			{}
		},
		.stateTypes = {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, match_any_except_a_range_of_characters) {
	auto actual = convertRegexToNFA("[^a-c]");
	Automata expect = {
		.stateGraph = {
			{ to(1, acceptsAnyExcept(Transition::Range{'a', 'c'}))},
			{}
		},
		.stateTypes = {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, match_any_except_a_set_and_a_range_of_characters) {
	auto actual = convertRegexToNFA("[^abcA-C]");
	Automata expect = {
		.stateGraph = {
			{ to(1, acceptsAnyExcept({ Transition::Range{'A', 'C'}, 'a', 'b', 'c' }))},
			{}
		},
		.stateTypes = {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, empty_square_brackets) {
	auto actual = convertRegexToNFA("[]");
	Automata expect = {
		.stateGraph = {
			{ eps(1) },
			{}
		},
		.stateTypes = {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, square_bracket_match_a_space_or_a) {
	auto actual = convertRegexToNFA("[ a]");
	Automata expect = {
		.stateGraph = {
			{ to(1, accepts({' ', 'a'}))},
			{}
		},
		.stateTypes = {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}


TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, character_class_that_is_out_of_order) {
	std::vector<std::string> patterns{ "[c-a]", "[Z-A]", "[9-0]" };
	for (const auto& pattern : patterns) {
		try {
			convertRegexToNFA(pattern);
			FAIL() << std::format("Invalid regex {} should causes an exception but there isn't one.", pattern);

		}
		catch (std::runtime_error(e)) {
			EXPECT_STREQ(e.what(), "Invalid regex expression: range out of order in the character class.");
		}
		catch (...) {
			FAIL() << "It should throw a std::runtime_error";
		}
	}
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, character_classes_that_has_no_ending_bracket) {
	std::vector<std::string> patterns{ "[", "[12", "[12,", "[12,33", "[a" };

	for (const auto& pattern : patterns) {
		try {
			convertRegexToNFA(pattern);
			FAIL() << std::format("Invalid regex {} should causes an exception but there isn't one.", pattern);

		}
		catch (std::runtime_error(e)) {
			EXPECT_STREQ(e.what(), "Invalid regex: missing ending bracket ']'.");
		}
		catch (...) {
			FAIL() << "It should throw a std::runtime_error";
		}
	}
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, quirky_character_classes) {
	auto actual = convertRegexToNFA("[[-]");
	Automata expect = {
		.stateGraph {
			{ to(1, accepts({'-', '[' }))},
			{}
		},
		.stateTypes {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);

	actual = convertRegexToNFA("[---]");
	expect = {
		.stateGraph {
			{ to(1, accepts('-'))},
			{}
		},
		.stateTypes {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);

	try {
		actual = convertRegexToNFA("[[--]");
		FAIL("Expects the regex /[[--]/ to throw a std::runtime_error, but it doesn't throw.");
	}
	catch (std::runtime_error e) {
		EXPECT_STREQ(e.what(), "Invalid regex expression: range out of order in the character class.");
	}
	catch (...) {
		FAIL("Expects the regex /[[--]/ to throw a std::runtime_error, but it throws an other type of exception.");
	}

	actual = convertRegexToNFA("[--[]");
	expect = {
		.stateGraph {
			{ to(1, accepts(Transition::Range('-', '[')))},
			{}
		},
		.stateTypes {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);

	actual = convertRegexToNFA("[-]]");
	expect = {
		.stateGraph {
			{ to(1, accepts('-')) },
			{ eps(2) },
			{ to(3, accepts(']')) },
			{}
		},
		.stateTypes {
			INITIAL_STATE,
			PLAIN_STATE,
			PLAIN_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);

	actual = convertRegexToNFA("[a-u-z]");
	expect = {
		.stateGraph {
			{ to(1, accepts({'-', Transition::Range('a', 'u'), 'z'}))},
			{}
		},
		.stateTypes {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, a_lonely_right_bracket_is_just_a_right_bracket) {
	auto actual = convertRegexToNFA("]");
	Automata expect = {
		.stateGraph {
			{ to(1, accepts(']')) },
			{}
		},
		.stateTypes {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);
}

TEST(Test_ConvertRegexToNFA_EnhancedRegexSyntax, character_class_overlapping_ranges) {
	auto actual = convertRegexToNFA("[A-uc-z]");
	Automata expect = {
		.stateGraph {
			{ to(1, accepts(Transition::Range('A', 'z')))},
			{}
		},
		.stateTypes {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);

	actual = convertRegexToNFA("[a-za]");
	expect = {
		.stateGraph {
			{ to(1, accepts(Transition::Range('a', 'z')))},
			{}
		},
		.stateTypes {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);

	actual = convertRegexToNFA("[aba]");
	expect = {
		.stateGraph {
			{ to(1, accepts({'a', 'b'}))},
			{}
		},
		.stateTypes {
			INITIAL_STATE,
			ACCEPTING_STATE
		}
	};

	EXPECT_EQ(actual, expect);

}