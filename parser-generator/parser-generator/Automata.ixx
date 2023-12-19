module;

#include <vector>
#include <variant>

export module Automata;

export struct Transition {
	struct Range {
		char from, to;
		bool operator==(const Range&) const = default;
	};

	enum AcceptingMode {
		INCLUDE_CHARS,
		EXCLUDE_CHARS,
		ACCEPT_ANY_CHARACTER
	} mode;

	int destination;
	std::vector<std::variant<char, Range>> conditions;

	bool operator==(const Transition&) const = default;
};

export Transition acceptsAnyCharacter();
export Transition accepts(std::initializer_list<std::variant<char, Transition::Range>> conditions);
export template<std::ranges::range Iter>
Transition accepts(Iter&& iter) {
	Transition t;
	t.conditions.assign(iter.begin(), iter.end());
	t.mode = Transition::INCLUDE_CHARS;
	return t;
}

export Transition accepts(std::variant<char, Transition::Range> cond);
export Transition eps(int destination);

export Transition acceptsAnyExcept(std::initializer_list<std::variant<char, Transition::Range>> conditions);
export template<std::ranges::range Iter>
Transition acceptsAnyExcept(Iter&& iter) {
	Transition t;
	t.conditions = std::vector(iter.begin(), iter.end());
	t.mode = Transition::EXCLUDE_CHARS;
	return t;
}

export Transition acceptsAnyExcept(std::variant<char, Transition::Range> cond);
export Transition to(int destination, Transition transition);

export enum StateType {
	INITIAL_STATE,
	PLAIN_STATE,
	ACCEPTING_STATE
};

export constexpr char EPSILON = '\0';

export struct Automata {
	std::vector<std::vector<Transition>> stateGraph;
	std::vector<StateType> stateTypes;

	bool operator==(const Automata&) const = default;
};

export int addNewState(Automata& automata, StateType stateType = PLAIN_STATE);
export void addTransition(Automata& automata, int fromNode, Transition node);

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

Transition accepts(std::variant<char, Transition::Range> cond) {
	Transition t;
	t.conditions = { cond };
	t.mode = Transition::INCLUDE_CHARS;
	return t;
}

Transition eps(int destination) {
	return to(destination, accepts(EPSILON));
}

Transition acceptsAnyExcept(std::initializer_list<std::variant<char, Transition::Range>> conditions) {
	Transition t;
	t.conditions = std::vector(conditions.begin(), conditions.end());
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
