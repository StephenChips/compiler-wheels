module;

#include <vector>
#include <functional>
#include <string>
#include <optional>
#include <set>
#include <variant>
#include <type_traits>
#include <stdexcept>
#include <ranges>
#include <cctype>

export module Regex;

export struct Transition {
	struct Range {
		char from, to;
		bool operator==(const Range&) const = default;
	};

	enum AcceptingMode { INCLUDE_CHARS, EXCLUDE_CHARS, ACCEPT_ANY_CHARACTER };

	int destination;
	AcceptingMode mode;
	std::vector<std::variant<char, Range>> conditions;

	bool operator==(const Transition&) const = default;
};

export Transition acceptsAnyCharacter();
export Transition accepts(std::initializer_list<std::variant<char, Transition::Range>> conditions);
export template<std::ranges::range Iter> Transition accepts(Iter begin, Iter end);
export Transition accepts(std::variant<char, Transition::Range> cond);
export Transition eps(int destination);

export Transition acceptsAnyExcept(std::initializer_list<std::variant<char, Transition::Range>> conditions);
export template<std::ranges::range Iter> Transition acceptsAnyExcept(Iter begin, Iter end);
export Transition acceptsAnyExcept(std::variant<char, Transition::Range> cond);

export Transition to(int destination, Transition transition);

export enum StateType {
	INITIAL_STATE,
	PLAIN_STATE,
	ACCEPTING_STATE
};

export constexpr const char EPSILON = '\0';

export struct Automata {
	std::vector<std::vector<Transition>> stateGraph;
	std::vector<StateType> stateTypes;

	bool operator==(const Automata&) const = default;
};

int addNewState(Automata& automata, StateType stateType = PLAIN_STATE);
void addTransition(Automata& automata, int fromNode, Transition node);

using InitialState = int;
using AcceptingState = int;

struct RepetitionQualifier {
	enum Type { EXACTLY_N_TIMES, AT_LEAST_N_TIMES, BETWEEN_N_AND_M_TIMES } type;
	int n, m;
};

std::tuple<InitialState, AcceptingState> parseRegex(Automata& automata, const std::string regexPattern, int& cursor);
std::tuple<InitialState, AcceptingState> parseConcatenation(Automata& automata, const std::string& regexPattern, int& cursor);
std::tuple<InitialState, AcceptingState> parseRegexBasicUnit(Automata& automata, const std::string& regexPattern, int& cursor);
std::tuple<InitialState, AcceptingState> parseCharacterClass(Automata& automata, const std::string& regexPattern, int& cursor);
std::tuple<InitialState, AcceptingState> addKleeneClosure(Automata& automata, int initialState, int acceptingState);
int parseInt(const std::string& regexPattern, int& cursor);
void skipBlanks(const std::string& str, int& cursor);
RepetitionQualifier parseRepetitionQuealifier(const std::string& regexPattern, int& cursor);
constexpr bool isTheFirstCharOfQualifier(const char ch);

export Automata convertRegexToNFA(const std::string regexPattern);
export Automata convertNFAToDFA(const Automata& automata);
export Automata calculateMinimumDFA(const Automata& automata);

export struct MatchResult {
	bool succeeded;
	std::string::const_iterator endIter;
};

export class Regex {
	Automata automata;
public:
	Regex(const std::string& pattern) {}
	MatchResult match(std::string::const_iterator str);
};

module : private;

Transition acceptsAnyCharacter() {
	Transition t;
	t.mode = Transition::ACCEPT_ANY_CHARACTER;
	return t;
}

Transition accepts(std::initializer_list<std::variant<char, Transition::Range>> conditions) {
	Transition t;
	t.conditions = std::vector(conditions.begin(), conditions.end());
	t.mode = Transition::INCLUDE_CHARS;
	return t;
}

template<std::ranges::range Iter>
Transition accepts(Iter &&iter) {
	Transition t;
	t.conditions.assign(iter.begin(), iter.end());
	t.mode = Transition::INCLUDE_CHARS;
	return t;
}

Transition accepts(std::variant<char, Transition::Range> cond) {
	Transition t;
	t.conditions = { cond };
	t.mode = Transition::INCLUDE_CHARS;
	return t;
}

Transition eps(int destination) {
	return to(destination,  accepts(EPSILON));
}

Transition acceptsAnyExcept(std::initializer_list<std::variant<char, Transition::Range>> conditions) {
	Transition t;
	t.conditions = std::vector(conditions.begin(), conditions.end());
	t.mode = Transition::EXCLUDE_CHARS;
	return t;
}

template<std::ranges::range Iter>
Transition acceptsAnyExcept(Iter &&iter) {
	Transition t;
	t.conditions = std::vector(iter.begin(), iter.end());
	t.mode = Transition::EXCLUDE_CHARS;
	return t;
}

Transition acceptsAnyExcept(std::variant<char, Transition::Range> cond) {
	Transition t;
	t.conditions = { cond };
	t.mode = Transition::EXCLUDE_CHARS;
	return t;
}

Transition to(int destination, Transition transition) {
	transition.destination = destination;
	return transition;
}

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

std::tuple<InitialState, AcceptingState> addKleeneClosure(Automata& automata, int initialState, int acceptingState) {
	// if there is a "*" operator right after the expression we've just parsed,
	// We will make a kleene closure for it.
	int initialStateOfKleeneClosure = addNewState(automata);
	int acceptingStateOfKleeneClosure = addNewState(automata);

	addTransition(automata, initialStateOfKleeneClosure, eps(acceptingStateOfKleeneClosure));
	addTransition(automata, initialStateOfKleeneClosure, eps(initialState));
	addTransition(automata, acceptingState, eps(acceptingStateOfKleeneClosure));
	addTransition(automata, acceptingState, eps(initialState));

	return { initialStateOfKleeneClosure, acceptingStateOfKleeneClosure };
}

void skipBlanks(const std::string& str, int& cursor) {
	while (cursor < str.size() && std::isspace(str[cursor])) cursor++;
}

int parseInt(const std::string& regexPattern, int &cursor) {
	int n = 0;
	do {
		n = n * 10 + regexPattern[cursor] - '0';
		cursor++;
	} while (cursor < regexPattern.size() && std::isdigit(regexPattern[cursor]));
	return n;
}

RepetitionQualifier parseRepetitionQuealifier(const std::string& regexPattern, int& cursor) {
	cursor++;
	skipBlanks(regexPattern, cursor);

	if (cursor == regexPattern.size() ||!std::isdigit(regexPattern[cursor])) {
		throw std::runtime_error("invalid regex.");
	}

	int nTimes = parseInt(regexPattern, cursor);
	skipBlanks(regexPattern, cursor);

	if (cursor == regexPattern.size()) {
		throw std::runtime_error("invalid regex");
	}

	if (regexPattern[cursor] == '}') {
		cursor++;
		return {
			.type = RepetitionQualifier::EXACTLY_N_TIMES,
			.n = nTimes
		};
	}

	if (regexPattern[cursor] == ',') {
		// case /a{,3}/
		cursor++;
		skipBlanks(regexPattern, cursor);

		if (cursor == regexPattern.size()) {
			throw std::runtime_error("invalid regex");
		}

		if (regexPattern[cursor] == '}') {
			cursor++;
			return {
				.type = RepetitionQualifier::AT_LEAST_N_TIMES,
				.n = nTimes
			};
		}

		if (std::isdigit(regexPattern[cursor])) {
			int mTimes = parseInt(regexPattern, cursor);

			if (cursor == regexPattern.size() || regexPattern[cursor] != '}') {
				throw std::runtime_error("invalid regex");
			}

			cursor++;
			return {
				.type = RepetitionQualifier::BETWEEN_N_AND_M_TIMES, 
				.n = nTimes,
				.m = mTimes
			};
		}

		throw std::runtime_error("invalid regex");
	}

	throw std::runtime_error("invalid regex");
}

constexpr bool isTheFirstCharOfQualifier(const char ch) {
	return ch == '*' || ch == '+' || ch == '?' || ch == '{';
}

const bool isTheFollowCharOfConcatenation(const char ch) {
	return ch == '|' || ch == ')';
}

// We assumed this function is called right after a basic unit is parsed.
// so the cursor argument is next to the position where the basic unit ends.
void addQualifierForBasicUnit(
	Automata& automata, 
	const std::string& regexPattern,
	int& cursor,
	int beginningCursorPositionOfBasicUnit,
	int& initialStateOfBasicUnit,
	int& acceptingStateOfBasicUnit) {
	
	if (regexPattern[cursor] == '*') {
		auto [initialState, acceptingState] = addKleeneClosure(automata, initialStateOfBasicUnit, acceptingStateOfBasicUnit);
		initialStateOfBasicUnit = initialState;
		acceptingStateOfBasicUnit = acceptingState;
		cursor++;
		return;
	}

	const auto basicUnit = regexPattern.substr(beginningCursorPositionOfBasicUnit, cursor);

	if (regexPattern[cursor] == '+') {
		// Since /a+/ is just /aa*/, and we've got the leading 'a', 
		// what we have to do is to generate the following /a*/ part.
		int _cursor = 0;
		auto [initialState, acceptingState] = parseRegex(automata, basicUnit + '*', _cursor);
		addTransition(automata, acceptingStateOfBasicUnit, eps(initialState));
		acceptingStateOfBasicUnit = acceptingState;
		cursor++;
	}
	else if (regexPattern[cursor] == '?') {
		addTransition(automata, initialStateOfBasicUnit, eps(acceptingStateOfBasicUnit));
		cursor++;
	}
	else if (regexPattern[cursor] == '{') {
		const auto repetition = parseRepetitionQuealifier(regexPattern, cursor);

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

	if (isTheFirstCharOfQualifier(regexPattern[cursor])) {
		// E.g. regex *abc isn't valid, so does foo(*abc) and bc|*de
		throw std::runtime_error(
			"A qualifier cannot be the first character of a regex, "
			"it must be placed after a character, a right parenthesis or a right bracket.\n"
			"For example: a*, (abc)* and [abc]*"
		);
	}

	if (regexPattern[cursor] == '}' || regexPattern[cursor] == ']') {
		// It isn't valid that read the ending brackets without reading the starting one first.
		// For example: /a}/, /]acb/ all all invalid regexes.
		throw std::runtime_error("Invalid regular expression");
	}

	do {
		auto beginningCursorPositionOfBasicUnit = cursor;
		auto [initialStateOfBasicUnit, acceptingStateOfBasicUnit] = parseRegexBasicUnit(automata, regexPattern, cursor);

		if (cursor < regexPattern.size() && isTheFirstCharOfQualifier(regexPattern[cursor])) {
			addQualifierForBasicUnit(
				automata,
				regexPattern,
				cursor,
				beginningCursorPositionOfBasicUnit,
				initialStateOfBasicUnit,
				acceptingStateOfBasicUnit
			);

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
	std::optional<char> previousCharacter; // The character right before one that the cursor's now pointing.
	std::vector<std::variant<char, Transition::Range>> conditions;
	Transition::AcceptingMode acceptingMode = Transition::INCLUDE_CHARS;

	cursor++; // skip leading '['

	if (cursor < regexPattern.size() && regexPattern[cursor] == '^') {
		acceptingMode = Transition::EXCLUDE_CHARS;
		cursor++;
	}

	while (cursor < regexPattern.size() && regexPattern[cursor] != ']') {
		// If the current character isn't a '-', we are sure that the previous
		// character is not the beginning character of a range.
		if (regexPattern[cursor] != '-') {
			if (previousCharacter) {
				conditions.push_back(*previousCharacter);
			}
			previousCharacter = regexPattern[cursor];
			cursor++;
			continue;
		}

		// If there isn't character being parsed, the '-' must be the first character
		// in the class. e.g. /[-a]/. 
		// Such character class means the '-' is one of its acceptable characters.
		if (!previousCharacter) {
			conditions.push_back('-');
			cursor++;
			continue;
		}

		// Otherwise we get the character before the '-'.
		const char characterBeforeDash = *previousCharacter;

		// If there is a character before '-', we shall inspect what follows it.
		cursor++;

		// If '-' is the last character of the string, we should breaks the loop
		// and throw an error, because it misses the ending ']'.
		// e.g. /[a-/
		if (cursor == regexPattern.size()) {
			break;
		}

		// If what follows '-' is a ']', it also cannot forms a range. e.g. /[a-]/.
		// Such character class means the '-' and the character before it are two
		// of its acceptable characters.
		if (regexPattern[cursor] == ']') {
			conditions.push_back(characterBeforeDash);
			conditions.push_back('-');
			cursor++;
			continue;
		}

		// Otherwise we get the character after the '-'.
		const char characterAfterDash = regexPattern[cursor];
		cursor++;

		// Test invalid case like /[z-a]/.
		if (characterBeforeDash > characterAfterDash) {
			throw std::runtime_error("Invalid regex expression: range out of order in the character class.");
		}

		// Finally, we come to the situation that we can create a range!
		conditions.push_back(Transition::Range{ characterBeforeDash, characterAfterDash });
	}

	// If the last condition is a char, it will be still in the 'previousCharacter' variable, we should add it
	// the the vector now.
	if (previousCharacter && (conditions.empty() || std::holds_alternative<char>(conditions.back()))) {
		conditions.push_back(*previousCharacter);
	}

	if (cursor == regexPattern.size()) {
		throw std::runtime_error("Invalid regular expression");
	}
	else {
		cursor++; // skip the ']'
	}

	const auto initialState = addNewState(automata);
	const auto acceptingState = addNewState(automata);
	if (conditions.empty()) {
		if (acceptingMode == Transition::INCLUDE_CHARS) {
			addTransition(automata, initialState, eps(acceptingState));
		}
		else {
			addTransition(automata, initialState, to(acceptingState, acceptsAnyCharacter()));
		}
		
	}
	else {
		if (acceptingMode == Transition::INCLUDE_CHARS) {
			addTransition(automata, initialState, to(acceptingState, accepts(conditions)));
		}
		else {
			addTransition(automata, initialState, to(acceptingState, acceptsAnyExcept(conditions)));
		}
	}

	return { initialState, acceptingState };
}

Automata convertRegexToNFA(const std::string regexPattern) {
	Automata automata;
	int cursor = 0;

	const auto [ theInitialState, theAcceptingState ] = parseRegex(automata, regexPattern, cursor);
	automata.stateTypes[theInitialState] = INITIAL_STATE;
	automata.stateTypes[theAcceptingState] = ACCEPTING_STATE;

	return automata;
}

MatchResult Regex::match(std::string::const_iterator str) {
	return MatchResult();
}

int addNewState(Automata& automata, StateType stateType) {
	automata.stateGraph.push_back(std::vector<Transition>());
	automata.stateTypes.push_back(stateType);
	return automata.stateGraph.size() - 1;
}


// Since the class is used internally, we can be sure that the "fromNode" won't be out of range.
void addTransition(Automata& automata, int fromNode, Transition transition) {
	auto& listOfTransitions = automata.stateGraph.at(fromNode);

	const auto iter = std::find_if(listOfTransitions.begin(), listOfTransitions.end(),
		[&](auto t) { return t.destination == transition.destination; });

	if (iter == listOfTransitions.end()) {
		listOfTransitions.push_back(std::move(transition));
	}
	else {
		for (const auto cond : transition.conditions) {
			auto iterOfCondition = std::find(iter->conditions.begin(), iter->conditions.end(), cond);
			if (iterOfCondition == iter->conditions.end()) {
				iter->conditions.push_back(cond);
			}
		}
	}
}
