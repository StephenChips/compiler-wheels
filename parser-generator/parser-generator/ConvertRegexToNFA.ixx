module;

#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <set>
#include <variant>
#include <type_traits>
#include <stdexcept>
#include <ranges>
#include <cctype>
#include <algorithm>

export module ConvertRegexToNFA;

import Automata;

export Automata convertRegexToNFA(const std::string& regexPattern);

module : private;

// helper type for the visitor
template<class... Ts>
struct matches : Ts... { using Ts::operator()...; };

struct RepetitionQualifier {
	enum Type { EXACTLY_N_TIMES, AT_LEAST_N_TIMES, BETWEEN_N_AND_M_TIMES } type;
	int n, m;
};

using InitialState = int;
using AcceptingState = int;

std::tuple<InitialState, AcceptingState> parseRegex(Automata& automata, const std::string regexPattern, int& cursor);
std::tuple<InitialState, AcceptingState> parseConcatenation(Automata& automata, const std::string& regexPattern, int& cursor);
std::tuple<InitialState, AcceptingState> parseRegexBasicUnit(Automata& automata, const std::string& regexPattern, int& cursor);
std::tuple<InitialState, AcceptingState> parseCharacterClass(Automata& automata, const std::string& regexPattern, int& cursor);
void eliminateOverlappingConditions(Transition& transition);
void addKleeneClosure(Automata& automata, int& initialState, int& acceptingState);
int parseInt(const std::string& regexPattern, int& cursor);
void skipBlanks(const std::string& str, int& cursor);
std::optional<RepetitionQualifier> parseRepetitionQualifier(const std::string& regexPattern, int& cursor);

/*
* The function will parse a regex and generate the correspondent states in the automata. If there are other states in
* the automata, for instance, we've used the automata to generate a NFA for another regex before, the new states are isolated
* from those old states. From a graph prespective, the new states together form a new component. The initial state and the
* accepting state of the new NFA are returned.
*
*/
std::tuple<InitialState, AcceptingState> parseRegex(Automata& automata, const std::string regexPattern, int& cursor) {
	const auto [initialStateOfSubpart, acceptingStateOfSubpart] = parseConcatenation(automata, regexPattern, cursor);

	if (cursor == regexPattern.size() || regexPattern[cursor] == ')') {
		return { initialStateOfSubpart, acceptingStateOfSubpart };
	}

	const int initialState = addNewState(automata);
	const int acceptingState = addNewState(automata);
	addTransition(automata, initialState, eps(initialStateOfSubpart));
	addTransition(automata, acceptingStateOfSubpart, eps(acceptingState));

	while (cursor < regexPattern.size() && regexPattern[cursor] != ')') {
		cursor++; // skip '|'

		const auto [initialStateOfSubpart, acceptingStateOfSubpart] = parseConcatenation(automata, regexPattern, cursor);
		addTransition(automata, initialState, eps(initialStateOfSubpart));
		addTransition(automata, acceptingStateOfSubpart, eps(acceptingState));
	}

	return { initialState, acceptingState };
}

void addKleeneClosure(Automata& automata, int& initialState, int& acceptingState) {
	// if there is a "*" operator right after the expression we've just parsed,
	// We will make a kleene closure for it.
	int initialStateOfKleeneClosure = addNewState(automata);
	int acceptingStateOfKleeneClosure = addNewState(automata);

	addTransition(automata, initialStateOfKleeneClosure, eps(acceptingStateOfKleeneClosure));
	addTransition(automata, initialStateOfKleeneClosure, eps(initialState));
	addTransition(automata, acceptingState, eps(acceptingStateOfKleeneClosure));
	addTransition(automata, acceptingState, eps(initialState));

	initialState = initialStateOfKleeneClosure;
	acceptingState = acceptingStateOfKleeneClosure;
}

void skipBlanks(const std::string& str, int& cursor) {
	while (cursor < str.size() && std::isspace(str[cursor])) cursor++;
}

int parseInt(const std::string& regexPattern, int& cursor) {
	int n = 0;
	do {
		int value = regexPattern[cursor] - '0';
		if (n > (INT_MAX - value) / 10) {
			n = INT_MAX;
		}
		else {
			n = n * 10 + value;
		}
		cursor++;
	} while (cursor < regexPattern.size() && std::isdigit(regexPattern[cursor]));

	return n;
}

std::optional<RepetitionQualifier> parseRepetitionQualifier(const std::string& regexPattern, int& cursor) {
	int nTimes, mTimes;
	int oldCursor = cursor;

	cursor++;
	skipBlanks(regexPattern, cursor);

	if (cursor == regexPattern.size() || !std::isdigit(regexPattern[cursor])) {
		goto fail;
	}

	nTimes = parseInt(regexPattern, cursor);
	skipBlanks(regexPattern, cursor);

	if (cursor == regexPattern.size()) {
		goto fail;
	}

	if (regexPattern[cursor] == '}') {
		cursor++;
		return std::optional<RepetitionQualifier> {
			std::in_place,
				RepetitionQualifier::EXACTLY_N_TIMES,
				nTimes
		};
	}

	if (regexPattern[cursor] == ',') {
		// case /a{,3}/
		cursor++;
		skipBlanks(regexPattern, cursor);

		if (cursor == regexPattern.size()) {
			goto fail;
		}

		if (regexPattern[cursor] == '}') {
			cursor++;
			return std::optional<RepetitionQualifier> {
				std::in_place,
					RepetitionQualifier::AT_LEAST_N_TIMES,
					nTimes
			};
		}

		if (std::isdigit(regexPattern[cursor])) {
			mTimes = parseInt(regexPattern, cursor);

			if (cursor == regexPattern.size() || regexPattern[cursor] != '}') {
				goto fail;
			}

			cursor++;
			return std::optional<RepetitionQualifier> {
				std::in_place,
					RepetitionQualifier::BETWEEN_N_AND_M_TIMES,
					nTimes,
					mTimes
			};
		}
	}

fail:
	cursor = oldCursor;
	return std::nullopt;
}

constexpr bool isTheFirstCharOfQualifier(const char ch) {
	return ch == '*' || ch == '+' || ch == '?' || ch == '{';
}

constexpr bool isTheFollowCharOfConcatenation(const char ch) {
	return ch == '|' || ch == ')';
}

void addPositiveClosure(
	Automata& automata,
	const std::string& basicUnit,
	int& initialStateOfBasicUnit,
	int& acceptingStateOfBasicUnit) {
	// Since /a+/ is just /aa*/, and we've got the leading 'a', 
	// // what we have to do is to generate the following /a*/ part.
	int _cursor = 0;
	const auto [initialState, acceptingState] = parseRegex(automata, basicUnit + '*', _cursor);
	addTransition(automata, acceptingStateOfBasicUnit, eps(initialState));
	acceptingStateOfBasicUnit = acceptingState;
}

void addRepetitionQualifier(
	Automata& automata,
	const std::string& basicUnit,
	int& initialStateOfBasicUnit,
	int& acceptingStateOfBasicUnit,
	const RepetitionQualifier& repetition) {

	if (repetition.n > 1) {
		for (int i = 0; i < repetition.n - 1; i++) {
			int _cursor = 0;
			const auto [initialState, acceptingState] = parseRegex(automata, basicUnit, _cursor);
			addTransition(automata, acceptingStateOfBasicUnit, eps(initialState));
			acceptingStateOfBasicUnit = acceptingState;
		}
	}

	if (repetition.type == RepetitionQualifier::AT_LEAST_N_TIMES) {
		int _cursor = 0;
		auto [initialState, acceptingState] = parseRegex(automata, basicUnit + "*", _cursor);
		addTransition(automata, acceptingStateOfBasicUnit, eps(initialState));
		acceptingStateOfBasicUnit = acceptingState;
	}
	else if (repetition.type == RepetitionQualifier::BETWEEN_N_AND_M_TIMES) {
		if (repetition.n > repetition.m) {
			throw std::runtime_error("numbers out of order in {} qualifier");
		}

		std::vector<int> vectorOfInitialStates;

		for (int i = 0; i < repetition.m - repetition.n; i++) {
			int _cursor = 0;
			const auto [initialState, acceptingState] = parseRegex(automata, basicUnit, _cursor);
			vectorOfInitialStates.push_back(initialState);
			addTransition(automata, acceptingStateOfBasicUnit, eps(initialState));
			acceptingStateOfBasicUnit = acceptingState;
		}

		for (const int state : vectorOfInitialStates) {
			addTransition(automata, state, eps(acceptingStateOfBasicUnit));
		}
	}
}

std::tuple<InitialState, AcceptingState> parseConcatenation(Automata& automata, const std::string& regexPattern, int& cursor) {
	bool isFirstBasicUnit = true;
	int initialState;
	int acceptingState;

	// for Corner case like //, /|/, /()/, returns a epsilon transition immediately.
	if (cursor == regexPattern.size() || isTheFollowCharOfConcatenation(regexPattern[cursor])) {
		initialState = addNewState(automata);
		acceptingState = addNewState(automata);
		addTransition(automata, initialState, eps(acceptingState));

		return { initialState, acceptingState };
	}

	if (regexPattern[cursor] == '*' ||
		regexPattern[cursor] == '+' ||
		regexPattern[cursor] == '?' ||
		parseRepetitionQualifier(regexPattern, cursor)) {
		// E.g. regex *abc isn't valid, so does foo(*abc) and bc|*de
		throw std::runtime_error(
			"A qualifier cannot be the first character of a regex, "
			"it must be placed after a character, a right parenthesis or a right bracket.\n"
			"For example: /a*/, /(abc)+/ and /[abc]{3}/"
		);
	}

	// notice: regexes /}/ and /]/ are valid. They just match the correspondent character, namely '}' and ']'.

	do {
		const auto basicUnitStartIndex = cursor;
		auto [initialStateOfBasicUnit, acceptingStateOfBasicUnit] = parseRegexBasicUnit(automata, regexPattern, cursor);
		const auto basicUnitEndIndex = cursor;


		if (cursor < regexPattern.size() && isTheFirstCharOfQualifier(regexPattern[cursor])) {

			if (regexPattern[cursor] == '*') {
				addKleeneClosure(automata, initialStateOfBasicUnit, acceptingStateOfBasicUnit);
				cursor++;
			}
			else if (regexPattern[cursor] == '+') {
				const auto basicUnit = regexPattern.substr(basicUnitStartIndex, basicUnitEndIndex);
				addPositiveClosure(
					automata,
					basicUnit,
					initialStateOfBasicUnit,
					acceptingStateOfBasicUnit
				);
				cursor++;
			}
			else if (regexPattern[cursor] == '?') {
				addTransition(automata, initialStateOfBasicUnit, eps(acceptingStateOfBasicUnit));
				cursor++;
			}
			else if (regexPattern[cursor] == '{') {
				// The cursor will only be advanced when we parse a repetition qualifier successfully.
				// Otherwise it won't be changed.
				const auto repetitionQualifier = parseRepetitionQualifier(regexPattern, cursor);

				// A invalid repetition qualifier are treat as character concatenation.
				// for example, /{1,/ matches '{', '1' and ',' in sequence.
				//
				// Skip to the next round then the function will treat these character normally.
				if (!repetitionQualifier) continue;

				const
					const auto basicUnit = regexPattern.substr(basicUnitStartIndex, basicUnitEndIndex);
				addRepetitionQualifier(
					automata,
					basicUnit,
					initialStateOfBasicUnit,
					acceptingStateOfBasicUnit,
					*repetitionQualifier
				);
			}

			if (cursor < regexPattern.size() && isTheFirstCharOfQualifier(regexPattern[cursor])) {
				// Expression like a** is not valid.
				throw std::runtime_error("Consecutive qualifiers is not valid.");
			}
		}

		if (isFirstBasicUnit) {
			initialState = initialStateOfBasicUnit;
			isFirstBasicUnit = false;
		}
		else {
			addTransition(automata, acceptingState, eps(initialStateOfBasicUnit));
		}

		acceptingState = acceptingStateOfBasicUnit;
	} while (cursor < regexPattern.size() && !isTheFollowCharOfConcatenation(regexPattern[cursor]));

	return { initialState, acceptingState };
}

std::tuple<InitialState, AcceptingState> parseRegexBasicUnit(Automata& automata, const std::string& regexPattern, int& cursor)
{
	if (regexPattern[cursor] == '(') {
		// parse a regex in a pair of parentheses, e.g. (foo|bar)
		cursor++;
		const auto initialAndAcceptingStates = parseRegex(automata, regexPattern, cursor);
		if (cursor == regexPattern.size()) {
			throw std::runtime_error("Missing the ending \")\".");
		}
		cursor++; // skip ')'
		return initialAndAcceptingStates;
	}

	if (regexPattern[cursor] == '[') {
		return parseCharacterClass(automata, regexPattern, cursor);
	}

	// parse a single character in the regex.
	int initialState = addNewState(automata);
	int acceptingState = addNewState(automata);

	if (regexPattern[cursor] == '.') {
		addTransition(automata, initialState, to(acceptingState, acceptsAnyCharacter()));
	}
	else {
		addTransition(automata, initialState, to(acceptingState, accepts(regexPattern[cursor])));
	}

	cursor++;
	return { initialState, acceptingState };
}

std::tuple<InitialState, AcceptingState> parseCharacterClass(Automata& automata, const std::string& regexPattern, int& cursor)
{
	std::vector<std::variant<char, Transition::Range>> conditions;
	Transition::AcceptingMode acceptingMode = Transition::INCLUDE_CHARS;

	cursor++; // skip leading '['

	if (cursor < regexPattern.size() && regexPattern[cursor] == '^') {
		acceptingMode = Transition::EXCLUDE_CHARS;
		cursor++;
	}

	// TODO: Apparently the program can only handle ASCII characters. Maybe consider Unicode later.
	while (cursor < regexPattern.size() && regexPattern[cursor] != ']') {
		if (cursor + 2 < regexPattern.size() &&
			regexPattern[cursor + 1] == '-' &&
			regexPattern[cursor + 2] != ']') {
			const char rangeStart = regexPattern[cursor];
			const char rangeEnd = regexPattern[cursor + 2];

			if (rangeStart > rangeEnd) {
				throw std::runtime_error("Invalid regex expression: range out of order in the character class.");
			}
			else if (rangeStart == rangeEnd) {
				conditions.push_back(rangeStart);
			}
			else {
				conditions.push_back(Transition::Range{ rangeStart, rangeEnd });
			}
			cursor += 3;
		}
		else {
			conditions.push_back(regexPattern[cursor]);
			cursor++;
		}
	}

	if (cursor == regexPattern.size()) {
		throw std::runtime_error("Invalid regex: missing ending bracket ']'.");
	}
	else {
		cursor++; // skip the ']'
	}
	
	Transition transition;
	const auto initialState = addNewState(automata);
	const auto acceptingState = addNewState(automata);
	if (conditions.empty()) {
		if (acceptingMode == Transition::INCLUDE_CHARS) {
			// /[]/
			transition = eps(acceptingState);
		}
		else {
			// /[^]/
			transition = to(acceptingState, acceptsAnyCharacter());
		}

	}
	else {
		if (acceptingMode == Transition::INCLUDE_CHARS) {
			transition = to(acceptingState, accepts(conditions));
		}
		else {
			transition = to(acceptingState, acceptsAnyExcept(conditions));
		}
	}

	eliminateOverlappingConditions(transition);
	
	addTransition(automata, initialState, to(acceptingState, transition));

	return { initialState, acceptingState };
}

bool compareTwoConditions(Transition::Condition& first, Transition::Condition& second) {
	return std::visit(matches{
		[](char a, char b) { return a < b; },
		[](char a, Transition::Range& b) { return a < b.from; },
		[](Transition::Range& a, char b) { return a.from < b;  },
		[](Transition::Range& a, Transition::Range& b) { return a < b; }
		}, first, second);
};

bool doesTwoConditionsOverlap(const Transition::Condition& theFirstOne, const Transition::Condition& theSecondOne) {
	return std::visit(matches{
		[](char a, char b) { return a == b; },
		[](char a, const Transition::Range& b) { return a >= b.from && a <= b.to; },
		[](const Transition::Range& a, char b) { return b >= a.from && b <= a.to;  },
		[](const Transition::Range& a, const Transition::Range& b) {
			return a.from < b.from && a.to >= b.from
				|| a.from >= b.from && a.from <= b.to;
		}
		}, theFirstOne, theSecondOne);
}

void mergeTwoConditions(Transition::Condition& destination, const Transition::Condition& source) {
	if (std::holds_alternative<char>(source)) return;
	else if (std::holds_alternative<char>(destination)) {
		destination = source;
	}
	else {
		auto& a = std::get<Transition::Range>(destination);
		const auto& b = std::get<Transition::Range>(source);
		if (b.from < a.from) a.from = b.from;
		if (b.to > a.to) a.to = b.to;
	}
}

void eliminateOverlappingConditions(Transition& transition)
{
	auto& conditions = transition.conditions;

	if (transition.mode == Transition::ACCEPT_ANY_CHARACTER) return;
	if (conditions.size() <= 1) return;
	
	std::sort(conditions.begin(), conditions.end(), compareTwoConditions);

	size_t i = 0;
	size_t j = 0;
	size_t k = 0;

	while (j < conditions.size()) {
		while (j < conditions.size() && doesTwoConditionsOverlap(conditions[i], conditions[j])) {
			mergeTwoConditions(conditions[i], conditions[j]);
			j++;
		}

		conditions[k++] = std::move(conditions[i]);
		i = j;
	}

	conditions.resize(k);
}

Automata convertRegexToNFA(const std::string& regexPattern) {
	Automata automata;
	int cursor = 0;

	const auto [theInitialState, theAcceptingState] = parseRegex(automata, regexPattern, cursor);
	automata.stateTypes[theInitialState] = INITIAL_STATE;
	automata.stateTypes[theAcceptingState] = ACCEPTING_STATE;

	return automata;
}
