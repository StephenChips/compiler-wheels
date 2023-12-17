module;

#include <gtest/gtest.h>
#include <vector>

export module Tests;

import Grammar;
import ValidateGrammar;

TEST(Test_ValidateRegex, test_simple_correct_grammar) {
	auto g = Grammar{
		.nonTerminators = {
			{"Expr", {
				{ "a", "Expr"_id },
				{ "a" }
			}}
		}
	};

	EXPECT_TRUE(isGrammarValid(g));
}

TEST(Test_ValidateRegex, test_redefined_identifier) {
	Grammar g{
		.nonTerminators = {
			{"Expr", {
				{ "a", "Expr"_id },
				{ "a" }
			}},
			{"Expr", {
				{ "a" }
			}}
		}
	};

	Grammar h{
		.terminators = { { "ID", "\\w+" }, {"ID", "option-(\\d+)" }},
	};

	Grammar j{
		.terminators = { { "Value", "{\\d+}" }, { "ID", "(0|1|2|3|4|5)+"}},
		.nonTerminators = {
			{"Value", {
				{ "ID", "Value"},
				{ "ID" }
			}}
		}
	};

	EXPECT_FALSE(isGrammarValid(g));
	EXPECT_FALSE(isGrammarValid(h));
	EXPECT_FALSE(isGrammarValid(j));
}

TEST(Test_ValidateRegex, test_undefined_identifier) {
	auto g = Grammar{
	.nonTerminators = {
		{"Expr", {
				{ "a", "Number"_id },
				{ "a" }
			}}
		}
	};
}