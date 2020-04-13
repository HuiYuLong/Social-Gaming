#include "translator.h"
#include <unordered_map>
#include "boost/variant.hpp"
#include <iostream>
#include <algorithm>
#include <random>
#include <ctime>


Variable buildVariables(const nlohmann::json& json)
{
    if (json.is_boolean()) {
        //std::cout << json << std::endl;
        return bool(json);
    }
    else if (json.is_number()) {
        //std::cout << json << std::endl;
        return int(json);
    }
    else if (json.is_string()) {
        //std::cout << json << std::endl;
        return std::string(json);
    }
    else if (json.is_object()) {
        Map map;
        for(const auto&[key, value]: json.items()) {
            //std::cout << key << std::endl;
            map[key] = buildVariables(value);
        }
        return map;
    }
    else if (json.is_array()) {
        List list;
        for(const auto&[key, value]: json.items()) {
            //std::cout << key << std::endl;
            list.push_back(buildVariables(value));
        }
        return list;
    }
    else {
		throw std::runtime_error{"Translator: Invalid JSON variable type"};
    }
}

Timer::Timer(int timeout):
	expected_end(std::chrono::steady_clock::now() + std::chrono::seconds(timeout)) { }

bool Timer::hasnt_expired() const
{
	return std::chrono::steady_clock::now() < expected_end;
}

template<class T>
class ResolveQuery : public boost::static_visitor<T&>
{
	Variable& toplevel;
public:
	ResolveQuery(Variable& toplevel): toplevel(toplevel) {}

	T& operator()(const Query& query) const
	{
		Getter getter(query, toplevel);
		return boost::get<T>(getter.get().result);
	}

	T& operator()(T& value) const
	{
		return value;
	}
};

std::unordered_map<std::string, std::function<std::unique_ptr<Rule>(const nlohmann::json&)>> rulemap = {

        //Control Structures
		{"foreach", [](const nlohmann::json& rule) { return std::make_unique<ForEachRule>(rule); }},
        {"loop", [](const nlohmann::json&rule) {return std::make_unique<LoopRule>(rule);}},
        {"inparallel", [](const nlohmann::json&rule) {return std::make_unique<InParallelRule>(rule);}},
        {"parallelfor", [](const nlohmann::json&rule) {return std::make_unique<ParallelForRule>(rule);}},
        {"switch", [](const nlohmann::json&rule) {return std::make_unique<SwitchRule>(rule);}},
        {"when", [](const nlohmann::json& rule) { return std::make_unique<WhenRule>(rule); }},

        //List Operations
        {"extend", [](const nlohmann::json& rule) {return std::make_unique<ExtendRule>(rule); }}, 
        {"reverse", [](const nlohmann::json& rule) {return std::make_unique<ReverseRule>(rule); }},
        {"shuffle", [](const nlohmann::json& rule) {return std::make_unique<ShuffleRule>(rule); }},
        {"sort",[](const nlohmann::json& rule) {return std::make_unique<SortRule>(rule);}},
        {"discard", [](const nlohmann::json& rule) {return std::make_unique<DiscardRule>(rule); }}, 
        {"deal",[](const nlohmann::json& rule) {return std::make_unique<DealRule>(rule);}},
        //Arithmetic Operations
        {"add", [](const nlohmann::json& rule) {return std::make_unique<AddRule>(rule); }},

        //Timing
        {"timer", [](const nlohmann::json& rule) {return std::make_unique<TimerRule>(rule); }},
		{"pause", [](const nlohmann::json& rule) {return std::make_unique<PauseRule>(rule); }},

        //Human Input 
        {"input-choice", [](const nlohmann::json& rule) {return std::make_unique<InputChoiceRule>(rule); }},
        {"input-text", [](const nlohmann::json& rule) {return std::make_unique<InputTextRule>(rule); }},
        {"input-vote", [](const nlohmann::json& rule) {return std::make_unique<InputVoteRule>(rule); }},

        //Output
        {"message", [](const nlohmann::json& rule) { return std::make_unique<MessageRule>(rule); }},
        {"global-message", [](const nlohmann::json& rule) { return std::make_unique<GlobalMessageRule>(rule); }},
        {"scores", [](const nlohmann::json& rule) {return std::make_unique<ScoresRule>(rule); }}
};

RuleList::RuleList(const nlohmann::json& json_rules)
{
	for (const auto& it : json_rules.items())
	{
		rules.push_back(rulemap[it.value()["rule"]](it.value()));
	}
}

std::vector<std::unique_ptr<Rule>>& RuleList::get() { return rules; }

class RuleListState : public RuleState
{
public:
	RuleListState(std::vector<std::unique_ptr<Rule>>& rules): iterator(rules.begin()) {}

	std::vector<std::unique_ptr<Rule>>::iterator iterator;
};

void RuleList::run(Server& server, GameState& state)
{
	auto& rule_state_ptr = state.getState(this);
	if(!rule_state_ptr) {
		rule_state_ptr = std::make_unique<RuleListState>(rules);
	}
	RuleListState& rule_list_state = *static_cast<RuleListState*>(rule_state_ptr.get());

	for ( ; rule_list_state.iterator != rules.end(); ++rule_list_state.iterator) {
		// Run one rule
		const auto& ptr = *rule_list_state.iterator;
		ptr->run(server, state);

		// If so, check the callbacks to let the timers stop it, or parallel rules to do something else
		if(auto [should_stop, will_be_resumed] = state.checkCallbacks(this); should_stop) {
			++rule_list_state.iterator;
			if (!will_be_resumed) {
				state.deleteState(this);
			}
			return;
		}
	}
	state.deleteState(this);
}

class ParameterVisitor : public boost::static_visitor<>
{
	Map parameters;
	Server& server;
	Connection game_owner;
	const std::string* name;
public:
	ParameterVisitor(Server& server, GameState& state, Connection game_owner): server(server), game_owner(game_owner) { }

	Map getParameters() { return std::move(parameters); }

	void setName(const std::string& _name) { name = &_name; }

	bool receiveBoolean(const std::string& prompt)
	{
		while (true) {
			auto received = server.receive(game_owner);
			if(received.has_value()) {
				std::string input = std::move(received.value());
				if(input.size() == 0u) {
					return false;
				}
				server.send({game_owner, input + "\n\n"});
				if (input == "true") {
					parameters[*name] = true;
					break;
				}
				else if (input == "false") {
					parameters[*name] = false;
					break;
				}
				else {
					server.send({game_owner, "Value must be \"true\" or \"false\"\n\n"});
					server.send({game_owner, prompt});
				}
			}
		}
		return true;
	}

	void operator()(bool boolean)
	{
		std::ostringstream buffer;
		buffer << "bool " << *name << ": " << (boolean ? "true": "false") << " -> ";
		server.send({game_owner, buffer.str()});
		if (!receiveBoolean(buffer.str())) {
			server.send({game_owner, (boolean ? "true\n\n" : "false\n\n")});
			parameters[*name] = boolean;
		}
	}

	bool receiveInteger(const std::string& prompt)
	{
		while (true) {
			auto received = server.receive(game_owner);
			if(received.has_value()) {
				std::string input = std::move(received.value());
				if(input.size() == 0u) {
					return false;
				}
				server.send({game_owner, input + "\n\n"});
				try {
					int new_integer = std::stoi(input);
					parameters[*name] = new_integer;
					return true;
				}
				catch (std::exception& e) {
					server.send({game_owner, "Value must be of integer type\n\n"});
					server.send({game_owner, prompt});
				}
			}
		}
	}   

	void operator()(int integer)
	{
		std::ostringstream buffer;
		buffer << "integer " << *name << ": " << integer << " -> ";
		server.send({game_owner, buffer.str()});
		if(!receiveInteger(buffer.str())) {
			server.send({game_owner, std::to_string(integer) + "\n\n"});
			parameters[*name] = integer;
		}
	}

	bool receiveString(const std::string& prompt)
	{
		while (true) {
			auto received = server.receive(game_owner);
			if(received.has_value()) {
				std::string input = std::move(received.value());
				if(input.size() == 0u) {
					return false;
				}
				server.send({game_owner, input + "\n\n"});
				parameters[*name] = input;
				return true;
			}
		}
	}

	void operator()(const std::string& string)
	{
		std::ostringstream buffer;
		buffer << "string " << *name << ": " << string << " -> ";
		server.send({game_owner, buffer.str()});
		if(!receiveString(buffer.str())) {
			server.send({game_owner, string + "\n\n"});
			parameters[*name] = string;
		}
	}

	void receiveQuestionAnswer()
	{
		List questions;
		for(size_t current_question = 1u; ; ++current_question) {
			Map question;
			server.send({game_owner, "Question " + std::to_string(current_question) + ": "});
			while (true) {
				auto received = server.receive(game_owner);
				if(received.has_value()) {
					std::string input = std::move(received.value());
					if(input.size() == 0u) {
						server.send({game_owner, "...\n\n"});
						parameters[*name] = std::move(questions);
						return;
					}
					server.send({game_owner, input + "\n"});
					question["question"] = std::move(input);
					break;
				}
			}
			server.send({game_owner, "Answer: "});
			while (true) {
				auto received = server.receive(game_owner);
				if(received.has_value()) {
					std::string input = std::move(received.value());
					if(input.size() == 0u) {
						continue;
					}
					server.send({game_owner, input + "\n\n"});
					question["answer"] = std::move(input);
					break;
				}
			}
			questions.push_back(std::move(question));
		}
	}

	void receiveMultipleChoice()
	{
		List questions;
		for(size_t current_question = 1u; ; ++current_question) {
			Map multiple_choice;
			server.send({game_owner, "Question " + std::to_string(current_question) + ": "});
			while (true) {
				auto received = server.receive(game_owner);
				if(received.has_value()) {
					std::string input = std::move(received.value());
					if(input.size() == 0u) {
						server.send({game_owner, "...\n\n"});
						parameters[*name] = std::move(questions);
						return;
					}
					server.send({game_owner, input + "\n"});
					multiple_choice["question"] = std::move(input);
					break;
				}
			}
			List choices;
			bool more_choices = true;
			for (size_t choice = 1u; more_choices; ++choice) {
				server.send({game_owner, std::to_string(choice) + ". "});
				while(true) {
					auto received = server.receive(game_owner);
					if(received.has_value()) {
						std::string input = std::move(received.value());
						if(input.size() == 0u) {
							if (choices.size() == 0u) {
								continue;
							}
							else {
								more_choices = false;
								break;
							}
						}
						server.send({game_owner, input + "\n"});
						choices.push_back(std::move(input));
						break;
					}
				}
			}
			server.send({game_owner, "\nCorrect choice: "});
			while (true) {
				auto received = server.receive(game_owner);
				if(received.has_value()) {
					std::string input = std::move(received.value());
					if(input.size() == 0u) {
						continue;
					}
					server.send({game_owner, input + "\n\n"});
					try {
						size_t choice = std::stoul(input) - 1u;
						Variable& correct_choice = choices.at(choice);
						multiple_choice["correct_choice"] = correct_choice;
						break;
					}
					catch (std::exception& e) {
						server.send({game_owner, "Please enter a number from the list of choices\n\nCorrect choice: "});
					}
				}
			}
			multiple_choice["choices"] = std::move(choices);
			questions.push_back(std::move(multiple_choice));
		}
	}

	void operator()(const Map& specification)
	{
		std::ostringstream buffer;
		const std::string& type = boost::get<std::string>(specification.at("kind"));
		const std::string& prompt = boost::get<std::string>(specification.at("prompt"));
		buffer << prompt << '\n' << type << ' ' << *name << " -> ";
		server.send({game_owner, buffer.str()});
		if (type == "boolean") {
			while(!receiveBoolean(buffer.str())) { }
		}
		else if (type == "integer") {
			while(!receiveInteger(buffer.str())) { }
		}
		else if (type == "string") {
			while(!receiveString(buffer.str())) { }
		}
		else if (type == "question-answer") {
			server.send({game_owner, "\n\n"});
			receiveQuestionAnswer();
		}
		else if (type == "multiple-choice") {
			server.send({game_owner, "\n\n"});
			receiveMultipleChoice();
		}
		else {
			throw std::runtime_error{"Unsupported setup variable kind"};
		}
	}
};

SetupRule::SetupRule(const nlohmann::json& setup)
{
	for (const auto& [key, value]: setup.items()) {
		if(value.is_object()) {
			parameters.emplace_back(key, boost::get<Map>(buildVariables(value)));
			const auto& parameter = boost::get<Map>(parameters.back().second);
			assert(parameter.find("kind") != parameter.end());
			assert(parameter.find("prompt") != parameter.end());
		}
		else if (value.is_boolean()) {
			parameters.emplace_back(key, bool(value));
		}
		else if (value.is_number()) {
			parameters.emplace_back(key, int(value));
		}
		else if (value.is_string()) {
			parameters.emplace_back(key, std::string(value));
		}
		else {
			throw std::runtime_error{"Unrecognized setup parameter"};
		}
	}
}

void SetupRule::run(Server& server, GameState& state)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(1100)); // wait for all the server messages to get processed
	if(parameters.size() == 0u) {
		return;
	}
	Connection game_owner = state.getGameOwnerConnection();
	ParameterVisitor visitor(server, state, game_owner);
	server.send({game_owner, "Set new values for game parameters or enter nothing to skip:\n\n"});
	for (const auto& [key, value]: parameters) {
		visitor.setName(key);
		boost::apply_visitor(visitor, value);
	}
	Map& toplevel = boost::get<Map>(state.getVariables());
	toplevel["configuration"] = visitor.getParameters();
}

//**** Control Structures ****//
ForEachRule::ForEachRule(const nlohmann::json& rule): list(rule["list"]), element_name(rule["element"]), subrules(rule["rules"])
{
    std::cout << "For each: " << element_name << std::endl;
}

LoopRule::LoopRule(const nlohmann::json& rule)
:	untilLoop(rule.find("until") != rule.end()),
	failCondition(untilLoop ? rule["until"] : rule["while"]),
	subrules(rule["rules"]) {
	std::cout << "Loop" << std::endl;
}

InParallelRule::InParallelRule(const nlohmann::json& rule): subrules(rule["rules"])
{
	std::cout << "In parallel" << std::endl;
}

ParallelForRule::ParallelForRule(const nlohmann::json& rule): subrules(rule["rules"]), list(rule["list"]), element_name(rule["element"])
{
	std::cout << "Parallel for" << std::endl;
}

Case::Case(const nlohmann::json& json_case): condition(json_case["condition"]), subrules(json_case["rules"]) {
	std::cout << "Case " << json_case["condition"] << std::endl;
}

Case::Case(const nlohmann::json& json_case, const Query& value): condition(buildVariables(json_case["case"]), value), subrules(json_case["rules"])
{
	std::cout << "Case " << json_case["case"] << std::endl;
}

Cases::Cases(const nlohmann::json& json_cases)
{
	cases.reserve(json_cases.size());
	for(const auto& case_ : json_cases.items()) {
		std::cout << case_.value()["condition"] << std::endl;
		cases.emplace_back(case_.value());
	}
}

Cases::Cases(const nlohmann::json& json_cases, const Query& value)
{
	cases.reserve(json_cases.size());
	for(const auto& case_ : json_cases.items()) {
		cases.emplace_back(case_.value(), value);
	}
}
	
WhenRule::WhenRule(const nlohmann::json& rule): cases(rule["cases"])
{
    std::cout << "When" << std::endl;
}

SwitchRule::SwitchRule(const nlohmann::json& rule): cases(rule["cases"], Query(rule["value"]))
{
	std::cout << "Switch" << std::endl;
}

//
// Todo: ParallelFor
//


//**** List Operations ****//
ReverseRule::ReverseRule(const nlohmann::json& rule): list(rule["list"]) {
	std::cout << "Reverse: " << list << std::endl;
}

SortRule::SortRule(const nlohmann::json& rule): list(rule["list"]) {
	std::cout << "Sort: " << list << std::endl;
}

ShuffleRule::ShuffleRule(const nlohmann::json& rule): list(rule["list"]) {
	std::cout << "Shuffle: " << list << std::endl;
}

// Todo: Extend, Deal, Discard & ListAttributes
DealRule::DealRule(const nlohmann::json& rule): from(rule["from"]), to(rule["to"]), count(rule["count"]){
	std::cout << "Deal: " << "from " << from << " to " << to << std::endl;
}

DiscardRule::DiscardRule(const nlohmann::json& rule): from(rule["from"]) {
	auto json_count = rule["count"];
	if (json_count.is_number()) {
		count = int(json_count);
	}
	else {
		count = Query(json_count);
	}
	std::cout << "Discard Variable: " << from.query << std::endl;
	std::cout << "Variable Size: " << count << std::endl;
}

ExtendRule::ExtendRule(const nlohmann::json& rule): list(rule["list"]), target(rule["target"]) {
	std::cout << "Extend: " << target.query << " with " << list.query << std::endl;
}

//**** Arithmetic Operations ****//
AddRule::AddRule(const nlohmann::json& rule): to(rule["to"]), value(rule["value"]) { std::cout << "Add " << value << std::endl; }
//
// Todo: NumericalAttribues
//

TimerRuleState::TimerRuleState(int duration): timer(duration) { }

TimerRuleImplementation::TimerRuleImplementation(int duration, const nlohmann::json& json_subrules): duration(duration), subrules(json_subrules)
{
	std::cout << "Timer with duration " << duration << std::endl;
}

AtMostTimer::AtMostTimer(int duration, const nlohmann::json& json_subrules): TimerRuleImplementation(duration, json_subrules) { }

void AtMostTimer::run(Server& server, GameState& state)
{
	// Place the timer into this game's state
	auto& rule_state_ptr = state.getState(this);
	rule_state_ptr = std::make_unique<TimerRuleState>(duration);

	// Run the rules while checking on the timer
	state.registerCallback(this);
	subrules.run(server, state);
	state.deregisterCallback(this);

	// Delete the timer
	state.deleteState(this);
}

CallbackResult AtMostTimer::check(GameState& state, Rule*)
{
	auto& rule_state_ptr = state.getState(this);
	Timer& timer = static_cast<TimerRuleState*>(rule_state_ptr.get())->timer;
	return {!timer.hasnt_expired(), false};	// returns true when the timer has expired, which will force all subrules to shut down
}


ExactTimer::ExactTimer(int duration, const nlohmann::json& json_subrules): TimerRuleImplementation(duration, json_subrules) { }

void ExactTimer::run(Server& server, GameState& state)
{
	// Place the timer into this game's state
	auto& rule_state_ptr = state.getState(this);
	if(!rule_state_ptr) {
		rule_state_ptr = std::make_unique<TimerRuleState>(duration);
	}

	// Make sure that the rules run at most the given duration
	state.registerCallback(this);
	subrules.run(server, state);
	state.deregisterCallback(this);

	// Pad the execution time to the given duration
	Timer& timer = static_cast<TimerRuleState*>(rule_state_ptr.get())->timer;
	while (timer.hasnt_expired()) {
		if(auto [should_stop, will_be_resumed] = state.checkCallbacks(this); should_stop) {
			if (!will_be_resumed) {
				state.deleteState(this);
			}
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// Delete the timer
	state.deleteState(this);
}

CallbackResult ExactTimer::check(GameState& state, Rule*)
{
	auto& rule_state_ptr = state.getState(this);
	Timer& timer = static_cast<TimerRuleState*>(rule_state_ptr.get())->timer;
	return {!timer.hasnt_expired(), false};	// returns true when the timer has expired, which will force all subrules to shut down
}

TrackTimer::TrackTimer(int duration, const nlohmann::json& json_subrules, const std::string& flag): TimerRuleImplementation(duration, json_subrules), flag(flag) { }

void TrackTimer::run(Server& server, GameState& state)
{
	// Set the flag to false
	Getter getter(flag, state.getVariables());
	GetterResult result = getter.get(true); // create if doesn't exist
	assert(!result.needs_to_be_saved);
	result.result = false;

	// Initialize timer
	auto& rule_state_ptr = state.getState(this);
	rule_state_ptr = std::make_unique<TimerRuleState>(duration);

	// Run the subrules while checking on the timer
	state.registerCallback(this);
	subrules.run(server, state);
	state.deregisterCallback(this);

	// Delete the timer
	state.deleteState(this);
}

CallbackResult TrackTimer::check(GameState& state, Rule*)
{
	auto& rule_state_ptr = state.getState(this);
	if (!rule_state_ptr) {
		return {false, false};	// the flag has already been set, move on
	}
	Timer& timer = static_cast<TimerRuleState*>(rule_state_ptr.get())->timer;
	if (!timer.hasnt_expired()) {
		// Set the flag to true
		Getter getter(flag, state.getVariables());
		GetterResult result = getter.get();
		assert(!result.needs_to_be_saved);
		result.result = true;
		// Disable the timer
		rule_state_ptr.reset(nullptr);
	}
	return {false, false}; // track timer never stops the rules
}


//**** Timing ****//
TimerRule::TimerRule(const nlohmann::json& rule) {
	const std::string& mode = rule["mode"];
	if (mode == "at most") {
		impl = std::make_unique<AtMostTimer>(rule["duration"], rule["rules"]);
	}
	else if (mode == "exact") {
		impl = std::make_unique<ExactTimer>(rule["duration"], rule["rules"]);
	}
	else if (mode == "track") {
		impl = std::make_unique<TrackTimer>(rule["duration"], rule["rules"], rule["flag"]);
	}
	else {
		throw std::runtime_error{"Invalid timer mode\n"};
	}
}

PauseRule::PauseRule(const nlohmann::json& rule): duration(rule["duration"]) { }


//**** Human Input ****//
InputChoiceRule::InputChoiceRule(const nlohmann::json& rule): to(rule["to"]), prompt(rule["prompt"]), result(rule["result"]) {
	if (auto timeout_it = rule.find("timeout"); timeout_it != rule.end()) {
		timeout = *timeout_it;
	}
	auto json_choices = rule["choices"];
	if (json_choices.is_array()) {
		choices = boost::get<List>(buildVariables(json_choices));
	}
	else {
		choices = Query(json_choices); // assume it's a variable name
	}
	std::cout << "Input Choice: " << rule["prompt"] << std::endl;
}
InputTextRule::InputTextRule(const nlohmann::json& rule): to(rule["to"]), prompt(rule["prompt"]), result(rule["result"]){
	if (auto timeout_it = rule.find("timeout"); timeout_it != rule.end()) {
		timeout = *timeout_it;
	}
	std::cout << "Input Text: " << rule["prompt"] << std::endl;
}
InputVoteRule::InputVoteRule(const nlohmann::json& rule): to(rule["to"]), prompt(rule["prompt"]), choices(rule["choices"]),result(rule["result"]){
	std::cout << "Input vote: " << rule["prompt"] << std::endl;
}

//**** Output ****//
ScoresRule::ScoresRule(const nlohmann::json& rule): score_attribute(rule["score"]), ascending(rule["ascending"]) { 
	std::cout << "Score Board: "  << std::endl; 
}

GlobalMessageRule::GlobalMessageRule(const nlohmann::json& rule): value(rule["value"]) { std::cout << "Global message: " << rule["value"] << std::endl; }

MessageRule::MessageRule(const nlohmann::json& rule): to(rule["to"]), value(rule["value"]) { 
	std::cout << "message: " << rule["value"] << std::endl; 
}

void LoopRule::run(Server& server, GameState& state) {

	while (untilLoop ^ failCondition.evaluate(state.getVariables())) {
		subrules.run(server, state);
		if(auto [should_stop, will_be_resumed] = state.checkCallbacks(this); should_stop) {
			return;
		}
	}
}

class ParallelRulesState : public RuleState
{
public:

	std::vector<Rule*> last_run;
};

void InParallelRule::run(Server& server, GameState& state)
{
	auto& rule_state_ptr = state.getState(this);
	if(!rule_state_ptr) {
		rule_state_ptr = std::make_unique<ParallelRulesState>();
	}
	ParallelRulesState& rule_state = *static_cast<ParallelRulesState*>(rule_state_ptr.get());

	std::unordered_map<uint16_t, std::vector<Rule*>> todo;
	auto& rules = subrules.get();
	todo.reserve(rules.size());
	uint16_t world_number = 0u;
	for (const auto& ptr : rules) {
		std::vector<Rule*> next = {ptr.get()};
		todo.emplace(world_number++, std::move(next));
	}

	state.registerCallback(this);
	while(!todo.empty()) {
		for (auto it = todo.begin(); it != todo.end();) {
			auto& unfinished_rules = it->second;
			Rule* last_unfinished  = unfinished_rules.back();
			unfinished_rules.pop_back();
			last_unfinished->run(server, state);
			for (auto it = rule_state.last_run.rbegin(); it != rule_state.last_run.rend(); ++it) {
				unfinished_rules.push_back(*it);
			}
			rule_state.last_run.clear();
			if (unfinished_rules.size() == 0u) {
				it = todo.erase(it);
			}
			else {
				++it;
			}
		}
		// std::remove_if(rule_state.todo.begin(), rule_state.todo.end(), [](const auto& iter) {
		// 	return iter.second.size() == 0u;	// remove finished rules
		// });
	}
	state.deregisterCallback(this);
	state.deleteState(this);
}

CallbackResult InParallelRule::check(GameState& state, Rule* ptr)
{
	auto& rule_state_ptr = state.getState(this);
	ParallelRulesState& rule_state = *static_cast<ParallelRulesState*>(rule_state_ptr.get());
	rule_state.last_run.push_back(ptr);
	return {true, true}; // first true: the rule should stop, second true: the rule will be resumed
}

void ParallelForRule::run(Server& server, GameState& state)
{
	auto& rule_state_ptr = state.getState(this);
	if(!rule_state_ptr) {
		rule_state_ptr = std::make_unique<ParallelRulesState>();
	}
	ParallelRulesState& rule_state = *static_cast<ParallelRulesState*>(rule_state_ptr.get());

	Getter getter(list, state.getVariables());
	GetterResult result = getter.get();
	List temp_elements;
	List& getter_elements = boost::get<List>(result.result);
	List& elements = (result.needs_to_be_saved ? temp_elements = std::move(getter_elements), temp_elements : getter_elements);

	std::unordered_map<uint16_t, std::pair<Variable&, std::vector<Rule*>>> todo;
	todo.reserve(elements.size());
	uint16_t world_number = 0u;
	Rule *first_rule = subrules.get().front().get();
	std::vector<Rule*> list_with_first_rule = {first_rule};
	for (Variable& element : elements) {
		todo.emplace(std::piecewise_construct,
                    std::forward_as_tuple(world_number++),
                    std::forward_as_tuple(element, list_with_first_rule));
	}

	state.registerCallback(this);
	Map& toplevel = boost::get<Map>(state.getVariables());
	while(!todo.empty()) {
		for (auto it = todo.begin(); it != todo.end();) {
			auto& [element, unfinished_rules] = it->second;
			// Place the current element into the game's variable tree
			toplevel[element_name] = getReference(element);
			// Reflect the changed state using the "parallel world number"
			// Due to the parallel world number, all subsequent rules, even if the are the same rules inside the configuration
			// will have a different state
			state.setParallelWorldNumber(it->first);
			// Run the unfinished rules
			Rule* last_unfinished  = unfinished_rules.back();
			unfinished_rules.pop_back();
			last_unfinished->run(server, state);
			for (auto it = rule_state.last_run.rbegin(); it != rule_state.last_run.rend(); ++it) {
				unfinished_rules.push_back(*it);
			}
			rule_state.last_run.clear();
			if (unfinished_rules.size() == 0u) {
				it = todo.erase(it);
			}
			else {
				++it;
			}
		}
		// std::remove_if(rule_state.todo.begin(), rule_state.todo.end(), [](const auto& iter) {
		// 	return iter.second.size() == 0u;	// remove finished rules
		// });
	}
	state.setParallelWorldNumber(0u);
	state.deregisterCallback(this);
	state.deleteState(this);
}

CallbackResult ParallelForRule::check(GameState& state, Rule* ptr)
{
	auto& rule_state_ptr = state.getPureState(this);
	ParallelRulesState& rule_state = *static_cast<ParallelRulesState*>(rule_state_ptr.get());
	rule_state.last_run.push_back(ptr);
	return {true, true}; // first true: the rule should stop, second true: the rule will be resumed
}

void GlobalMessageRule::run(Server& server, GameState& state)
{
	List& players = boost::get<List>(boost::get<Map>(state.getVariables())["players"]);
	for (Variable& player : players) {
		const std::string& name = boost::get<std::string>(boost::get<Map>(player)["name"]);
		server.send({state.getConnectionByName(name), value.fill_with(state.getVariables()) });
	}
}

void MessageRule::run(Server& server, GameState& state) { //IT'S WORKING
	Getter getter(to, state.getVariables());
	GetterResult result = getter.get();
	Map& p = boost::get<Map>(result.result);
	const std::string& name = boost::get<std::string>(p["name"]);
	server.send({state.getConnectionByName(name), value.fill_with(state.getVariables()) });
}

//List Operation

void ExtendRule::run(Server& server, GameState& state) {
	Getter getter(list, state.getVariables());
	List& extension = boost::get<List>(getter.get().result);

	getter.setQuery(target);
	GetterResult result = getter.get();
	assert(!result.needs_to_be_saved);
	List& target = boost::get<List>(result.result);
	
	target.reserve(target.size() + extension.size());
	for (Variable& element : extension) {
		target.push_back(getReference(element));
	}
	// Apparently Rock, Paper, Scissors requires the winners array to contain references to the players array
	// target.insert(target.end(), extension.begin(), extension.end());

	// PrintTheThing printer;
	// boost::apply_visitor(printer, state.getVariables());
	// std::cout << "Extended list: " << boost::apply_visitor(StringConverter(), result.result) << std::endl;
}

void ReverseRule::run(Server& server, GameState& state) {
	
	std::string toReverse = this->list;
	List& toReverseList = boost::get<List>(boost::get<Map>(state.getVariables())[toReverse]);
	std::reverse(toReverseList.begin(), toReverseList.end());
	//** For testing **//
	// for (Variable& weapon : reverseList) {
	// 	const std::string& weapons = boost::get<std::string>(boost::get<Map>(weapon)["name"]);
	// 	std::cout << "***After reversing weapons list***" << weapons << std::endl;
	// }
}

// int randomfunc(int j) 
// { 
//     return rand() % j; 
// } 

void ShuffleRule::run(Server& server, GameState& state) {
	std::string toShuffle= this->list;
	List& toShuffleList = boost::get<List>(boost::get<Map>(state.getVariables())[toShuffle]);
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(toShuffleList.begin(), toShuffleList.end(),g);
	
}

void DiscardRule::run(Server& server, GameState& state){
	// DEBUG
	// PrintTheThing printer;
	// boost::apply_visitor(printer, state.getVariables());

	Getter getter(from, state.getVariables());
	GetterResult result = getter.get();
	assert(!result.needs_to_be_saved);
	List& list = boost::get<List>(result.result);
	ResolveQuery<int> get_count(state.getVariables());
	int actual_count = boost::apply_visitor(get_count, this->count);
	if (actual_count < 0 || actual_count > list.size()) {
		throw std::runtime_error{"Discard Rule: invalid number of discarded elements"};
	}
	list.resize(list.size() - actual_count);
}

bool sort_variant_ascending(Variable& lhs, Variable& rhs) {
	return boost::get<std::string>(boost::get<Map>(lhs)["name"]) < boost::get<std::string>(boost::get<Map>(rhs)["name"]);
}

void SortRule::run(Server& server, GameState& state) {
	std::string toSort = this->list;
	List& sortList = boost::get<List>(boost::get<Map>(state.getVariables())[toSort]);
	std::sort(sortList.begin(), sortList.end(), sort_variant_ascending);
	// //** For testing **//
	for (Variable& weapon : sortList) {
	 	const std::string& weapons = boost::get<std::string>(boost::get<Map>(weapon)["name"]);
	 	std::cout << "***After weapons***" << weapons << std::endl;
	}
}


void ScoresRule::run(Server& server, GameState& state)
{
	// List& players = boost::get<List>(boost::get<Map>(state.getVariables())["players"]);
	// for (Variable& player : players) {
	// 	const std::string& name = boost::get<std::string>(boost::get<Map>(player)["name"]);
	// 	server.send({state.getConnectionByName(name), value.fill_with(state.getVariables())});
	// }
	std::cout<< "Score Board" <<std::endl;
	List& players = boost::get<List>(boost::get<Map>(state.getVariables()).at("players"));
	std::vector<std::pair<int, std::string>> score_board;
	score_board.reserve(players.size());

	std::transform(players.begin(), players.end(), std::back_inserter(score_board), [this](Variable& player) {
		const std::string& name = boost::get<std::string>(boost::get<Map>(player).at("name"));
		const int score = boost::get<int>(boost::get<Map>(player).at(score_attribute));
		return std::make_pair(score, name);
	});
	if (ascending){
		std::sort(score_board.begin(),score_board.end(), [](auto item1,auto item2){
			return item1.first < item2.first;
		});
	}
	else{
		std::sort(score_board.begin(),score_board.end(), [](auto item1,auto item2){
			return item1.first > item2.first;
		});
	}

	std::ostringstream buffer;
	for(const auto& [score, name] : score_board) {
		buffer << name << ": " << score << std::endl;
	}
	buffer << std::endl;
	for(auto item: score_board) {
		server.send({state.getConnectionByName(item.second), buffer.str() });
	}
}

class ForEachState : public RuleState
{
public:
	ForEachState(List& getter_elements, bool needs_to_be_saved)
	:	elements(needs_to_be_saved ? temp_elements = std::move(getter_elements), temp_elements : getter_elements),
		iterator(elements.begin())
	{ }

	List temp_elements;
	List& elements;
	List::iterator iterator;
};

void ForEachRule::run(Server& server, GameState& state)
{
	auto& rule_state_ptr = state.getState(this);
	if(!rule_state_ptr) {
		Getter getter(list, state.getVariables());
		GetterResult result = getter.get();
		rule_state_ptr = std::make_unique<ForEachState>(boost::get<List>(result.result), result.needs_to_be_saved);
	}
	ForEachState& foreach_state = *static_cast<ForEachState*>(rule_state_ptr.get());
	
	Map& toplevel = boost::get<Map>(state.getVariables());
	for (; foreach_state.iterator != foreach_state.elements.end(); ++foreach_state.iterator) {
		Variable& element = *foreach_state.iterator;
		toplevel[element_name] = getReference(element);
		//PrintTheThing p;
		//boost::apply_visitor(p, state.getVariables());
		subrules.run(server, state);
		if(auto [should_stop, will_be_resumed] = state.checkCallbacks(this); should_stop) {
			++foreach_state.iterator;
			if (!will_be_resumed) {
				state.deleteState(this);
			}
			return;
		}
	}
	state.deleteState(this);
}

void Cases::run(Server& server, GameState& state)
{
	for(Case& current_case : cases) {
		if (current_case.condition.evaluate(state.getVariables())) {
			current_case.subrules.run(server, state);
			break;
		}
	}
}

void WhenRule::run(Server& server, GameState& state)
{
	cases.run(server, state);
}

void SwitchRule::run(Server& server, GameState& state)
{
	cases.run(server, state);
}

class InputRulesState : public RuleState
{
public:
	InputRulesState(int duration) : prompt_sent(false), timer(duration) { }

	bool prompt_sent;
	Timer timer;
};

void InputChoiceRule::run(Server& server, GameState& state){
	auto& rule_state_ptr = state.getState(this);
	if(!rule_state_ptr) {
		rule_state_ptr = std::make_unique<InputRulesState>(timeout.value_or(300));
	}
	InputRulesState& input_rule_state = *static_cast<InputRulesState*>(rule_state_ptr.get());

	//  Get the player name
	Getter getter(to, state.getVariables());
	GetterResult player_result = getter.get();
	assert(!player_result.needs_to_be_saved);
	Map& player = boost::get<Map>(player_result.result);
	const std::string& player_name = boost::get<std::string>(player["name"]);
	Connection player_connection = state.getConnectionByName(player_name);

	// Get the list of choices (from a <Query, List> variant)
	ResolveQuery<List> get_list_of_choices(state.getVariables());
	List& list_of_choices = boost::apply_visitor(get_list_of_choices, choices);
	if (list_of_choices.size() == 0u) {
		throw std::runtime_error{"Input Choice: list of choices must be non-empty"};
	}

	if(!input_rule_state.prompt_sent) {
		// Write the prompt to buffer
		std::ostringstream buffer;
		buffer << prompt.fill_with(state.getVariables()) << std::endl;

		// Write the choices to buffer
		StringConverter to_string;
		for(size_t i = 0u; i < list_of_choices.size(); ++i){
			buffer << '\t' << i + 1 << ". " << boost::apply_visitor(to_string, list_of_choices[i]) << std::endl;
		}
		buffer << std::endl;
		server.send({player_connection, buffer.str()});
		input_rule_state.prompt_sent = true;
	}
	
	// Read user input
	size_t player_choice = list_of_choices.size();	// invalid choice
	while(input_rule_state.timer.hasnt_expired()) {
		auto received = server.receive(player_connection);
		if(received.has_value()) {
			std::string input = std::move(received.value());
			try {
				player_choice = std::stoul(input) - 1u;	// user-visible list starts with one
			}
			catch (std::exception& e) {
				player_choice = list_of_choices.size(); // invalid choice
			}
			if (player_choice < list_of_choices.size()) {
				server.send({player_connection, "You selected: " + input + "\n\n"});
				break;	// valid choice has been entered
			}
			else {
				server.send({player_connection,"Please enter a valid choice!\n\n"});
			}
		}
		if(auto [should_stop, will_be_resumed] = state.checkCallbacks(this); should_stop) {
			if (!will_be_resumed) {
				state.deleteState(this);
			}
			return;
		}
	}
	
	if (player_choice >= list_of_choices.size()) {
		player_choice = 0u;
		server.send({player_connection,"Timeout! Choosing the default first element.\n\n"});
	}

	// Store the result in a variable
	getter.setQuery(this->result); 
	GetterResult getter_result = getter.get(true);	// create a variable attribute if it doesn't exist
	assert(!getter_result.needs_to_be_saved);
	getter_result.result = list_of_choices.at(player_choice);

	state.deleteState(this);
	// PrintTheThing p;
	// boost::apply_visitor(p, state.getVariables());
}

void InputTextRule::run(Server& server, GameState& state){ //IT'S WORKING
	auto& rule_state_ptr = state.getState(this);
	if(!rule_state_ptr) {
		rule_state_ptr = std::make_unique<InputRulesState>(timeout.value_or(300));
	}
	InputRulesState& input_rule_state = *static_cast<InputRulesState*>(rule_state_ptr.get());

	//  Get the player name
	Getter getter(to, state.getVariables());
	GetterResult player_result = getter.get();
	assert(!player_result.needs_to_be_saved);
	Map& player = boost::get<Map>(player_result.result);
	const std::string& player_name = boost::get<std::string>(player["name"]);
	Connection player_connection = state.getConnectionByName(player_name);
	
	if(!input_rule_state.prompt_sent) {
		server.send({player_connection, prompt.fill_with(state.getVariables())});
		input_rule_state.prompt_sent = true;
	}

	std::string input;
	// Read user input
	while(input_rule_state.timer.hasnt_expired()) {
		auto received = server.receive(player_connection);
		if(received.has_value()) {
			input = std::move(received.value());
			server.send({player_connection, input + "\n\n"});
			break;	// valid choice has been entered
		}
		if(auto [should_stop, will_be_resumed] = state.checkCallbacks(this); should_stop) {
			if (!will_be_resumed) {
				state.deleteState(this);
			}
			return;
		}
	}
	if(input.size() == 0u) {
		server.send({player_connection, "Timeout!\n\n"});
	}

	getter.setQuery(this->result); 
	GetterResult getter_result = getter.get(true);	// create a variable attribute if it doesn't exist
	assert(!getter_result.needs_to_be_saved);
	getter_result.result = input;

	state.deleteState(this);
}


// This rule needs to be finished
void InputVoteRule::run(Server& server, GameState& state){
	// Implement the timeout!

	// auto& rule_state_ptr = state.getState(this);
	// if(!rule_state_ptr) {
	// 	rule_state_ptr = std::make_unique<InputRulesState>(timeout.value_or(300));
	// }
	// InputRulesState& input_rule_state = *static_cast<InputRulesState*>(rule_state_ptr.get());

	//Send message to the player/audience list
	Getter getter(to, state.getVariables());
	Variable& varplayers = getter.get().result;
	List& players = boost::get<List>(varplayers);
	for (Variable& player : players) {
		const std::string& name = boost::get<std::string>(boost::get<Map>(player)["name"]);
		server.send({state.getConnectionByName(name), prompt.fill_with(state.getVariables()) });
	}
	//defining list or list name
	if(choices.find(".name")!= std::string::npos){ //might have a better way to do this, but it works
		choices.erase(choices.size()-5, 5);
	}
	List& choiceList = boost::get<List>(boost::get<Map>(state.getVariables())[choices]);
	std::vector<std::string> choiceCheck; //vector to check if the choice valid
	//construct choice check vector
	for(auto choice:choiceList){
		const std::string choiceName = boost::get<std::string>(boost::get<Map>(choice)["name"]);
		choiceCheck.push_back(choiceName);
	}
	//send choice list to player/audience list
	for (Variable& player : players) {
		const std::string& name = boost::get<std::string>(boost::get<Map>(player)["name"]);
		for(auto choice:choiceCheck){
			server.send({state.getConnectionByName(name), choice + "\t"});
		}
		server.send({state.getConnectionByName(name), "\n"});
	}
	
	//construct vote list
	Getter getterResult(this->result, state.getVariables());
	GetterResult resultResult = getterResult.get();
	List& voteList = boost::get<List>(resultResult.result);
	voteList.clear();  //remove everything in current voteList
	for(auto choice:choiceCheck){
		Map voteMap;
		int count = 0;
		voteMap["count"] = count;
		std::string choiceName = choice;
		voteMap["name"] = choiceName;
		voteList.emplace_back(voteMap);
	}
	
	//read user input
	for (Variable& player : players) {
		bool isReceived = false;
		bool isValid = false;
		std::string input = "";
		const std::string& name = boost::get<std::string>(boost::get<Map>(player)["name"]);
		while(!isValid){ //read user input till it's a valid choice
			isReceived = false;
			while(!isReceived){
				Connection connection = state.getConnectionByName(name);
				auto received = server.receive(connection);
				if(received.has_value()){
					input = std::move(received.value());
					server.send({connection,input+"\n"});
					isReceived = true;
				}
			}
			isValid = std::any_of(choiceCheck.begin(), choiceCheck.end(), [&input](auto &item){
				return (input.compare(item) == 0);
			});

			if(!isValid){
				server.send({state.getConnectionByName(name),"Please input correct choice!\n"});
			}

			// if(auto [should_stop, will_be_resumed] = state.checkCallbacks(); should_stop) {
			// 	// if (!will_be_resumed) {
			// 	// 	state.deleteState(this);
			// 	// }
			// 	return;
			// }
		}

		//modify vote count
		for(Variable& vote:voteList){
			const std::string& name = boost::get<std::string>(boost::get<Map>(vote)["name"]);
			int& count = boost::get<int>(boost::get<Map>(vote)["count"]);
			if(input.compare(name) == 0){
				count++;
				break;
			}
		}
	}

	//for testing
	// PrintTheThing p2;
    // boost::apply_visitor(p2, state.getVariables());
}

void DealRule::run(Server& server, GameState& state){ //ONLY WORKS FOR INTEGER COUNT :(
	List& fromList = boost::get<List>(boost::get<Map>(state.getVariables())[from]);
	List& toList = boost::get<List>(boost::get<Map>(state.getVariables())[to]);
	for(int i = 0; i < count; i++){
		if(fromList.empty()){
			break;
		}
		auto temp = fromList.back();
		fromList.pop_back();
		toList.emplace_back(temp);
	}
	//for testing
	// PrintTheThing p2;
    // boost::apply_visitor(p2, state.getVariables());
}
//Helper functions
//Crop The big JSON file into short target secction with input name
nlohmann::json CropSection(const nlohmann::json& j,std::string name){
	for(auto& item: j.items()){
		if((item.key() == name)){
			return item.value();
		}
	}
	return nullptr;
}


void AddRule::run(Server& server, GameState& state)
{
	Getter getter(to, state.getVariables());
	GetterResult result = getter.get();
	assert(!result.needs_to_be_saved);
	int& integer = boost::get<int>(result.result);
	integer += value;
}

// void TimerRule::run(Server& server, GameState& state) {
// 	std::cout << mode << std::endl;
// 	std::clock_t start;
//     start = std::clock();
// 	bool flag = false;
// 	std::cout << "before " << std::endl;
// 	float timer = float(std::clock()-start)/CLOCKS_PER_SEC;
// 	// typedef std::unique_ptr<int> ptr;
// 	// while(auto const& ptr != subrules.end()) {
// 	for(auto const& ptr:subrules) {
// 		timer = float(std::clock()-start)/CLOCKS_PER_SEC;
// 		std::cout << "enter subrules" << std::endl;
// 		if(mode == "exact") {
// 			if(timer>duration) {
// 				std::cout << "times up!" << std::endl;
// 				// TODO: send a msg to the server to stop the rest of the rules
// 				return;
// 			}
// 			else if( (timer<duration) && (ptr==NULL)) {
// 				// keep executing until time > the given duration
// 				std::cout << "prt is null" << std::endl;
// 				while(timer<duration);
// 				return;
// 			}
// 		}
// 		else if( (timer>duration) && (mode == "track")) {
// 			std::cout << "!!flag" << std::endl;
// 		}
// 		else if( (timer>duration) && (mode == "atmost")) {
// 			// TODO: send a msg to the server to stop the rest of the rules
// 			std::cout << "times up!" << std::endl;
// 			return;
// 		}

// 		ptr->run(server, state);
// 		// ptr++;
// 	}
// }

void TimerRule::run(Server& server, GameState& state) {
	impl->run(server, state);
}

void PauseRule::run(Server& server, GameState& state) {
	std::this_thread::sleep_for(std::chrono::seconds(duration));
}

class TEST : public boost::static_visitor<>
{
public:

    void operator()(bool boolean) const
    {
        std::cout << (boolean ? "true" : "false") << std::endl;
    }

    void operator()(int integer) const
    {
        std::cout << integer << std::endl;
    }

    void operator()(const std::string& string) const
    {
        std::cout << string << std::endl;
    }

	void operator()(const Query& query) const
    {
        std::cout << "Query: " << query.query << std::endl;
    }

    void operator()(const List& list) const
    {
        std::cout << "Size: " << list.size() << std::endl;
    }

    void operator()(const Map& map) const
    {
		std::cout << "Map: " << map.size() << std::endl;
    }
};

// main is in gameserver.cpp

// int main(int argc, char** argv) {
// 	if (argc < 2) {
// 		std::cerr << "Usage:\n  " << argv[0] << "<server config>\n"
// 				<< "  e.g. " << argv[0] << " ../configs/server/congfig1.json\n";
// 		return 1;
// 	}

// 	// Variable a = "a";
// 	// boost::apply_visitor(TEST(), a);
// 	// a = Query{"b"};
// 	// boost::apply_visitor(TEST(), a);
// 	// return 0;

// 	std::ifstream serverconfig{argv[1]};
// 	if (serverconfig.fail())
//     {
//         std::cout << "cannot open the configuration file" << std::endl;
//         return 0;
//     }
// 	nlohmann::json j = nlohmann::json::parse(serverconfig);

// 	std::vector<Configuration> configurations;
// 	std::vector<Player> players;
// 	for (const std::string& name : {"a", "b"})
// 		players.emplace_back(name, Connection());

// 	configurations.reserve(j["games"].size());

// 	for ([[maybe_unused]] const auto& [key, gamespecfile]: j["games"].items())
// 	{
// 		std::ifstream gamespecstream{std::string(gamespecfile)};
// 		if (gamespecstream.fail())
// 		{
// 			std::cout << "cannot open the game configuration file " << gamespecfile << std::endl;
// 			return 0;
// 		}
// 		nlohmann::json gamespec = nlohmann::json::parse(gamespecstream);
// 		configurations.emplace_back(gamespec);
// 		std::cout << "\nTranslated game " << key << "\n\n";
//     }

//     // test that moving the players out of configurations works
//     // in the future populating the players list would be done inside GameSession
//     // for (auto& configuration : configurations) {
//     // 	Map& map = boost::get<Map>(configuration.getVariables());
//     //     List& player_list = boost::get<List>(map["players"]);
//     //     for (const Player& player : players) {
//     //     	std::cout << "harro" << std::endl;
//     //     	// this needs to be handled differently in GameSession
//     //     	//Map player_map = boost::get<Map>(buildVariables(j["per-player"]));
//     //     	Map player_map = Map();
//     //     	player_map["name"] = player.name;
//     //     	PlayerMap& players_map = configuration.getPlayersMap();
//     //     	players_map[player.name] = player.connection;
//     //     	player_list.push_back(player_map);
//     //     }
//     // }

// 	// TEST
// 	Variable& variables = configurations.front().getVariables();
// 	PrintTheThing p;
// 	boost::apply_visitor(p, variables);

// 	std::cout << "\nStarting a test\n\n";
// 	Server server;
// 	std::thread t1 = configurations.front().launchGameDetached(server);
// 	// std::thread t2 = configurations.back().launchGameDetached(server);
// 	t1.join();
// 	// t2.join();
// 	//Server server;
// 	//configurations.back().launchGame(server);
// 	// for(Configuration& c : configurations) {
// 	// 	std::cout << "\nGame " << c.getName() << "\n\n";
// 	// 	c.launchGame(server);
// 	// }
// 	std::cout << "\nFinished\n";

// 	return 0;
// };






// original code

//---------------------------------------------------------------------------------------------------------------------

// std::unique_ptr<Constants> parseConstants(const nlohmann::json& constantsConfig) {
// 	std::unique_ptr<Constants> constants = std::make_unique<Constants>();
// 	for (auto& item : constantsConfig.items()) {
// 		for (auto& item2 : item.value().items()) {
// 			string k = item2.value()["name"];
// 			string v = item2.value()["beats"];
// 			constants->insertToConstantsMap(k,v);
// 		}
// 	}
// 	return constants;
// }

// void parseConstantsTest(const nlohmann::json& gameConfig){
// 	nlohmann::json constants = DivideSection(gameConfig,"constants");
// 	std::unique_ptr<Constants> constant = parseConstants(constants);
// 	for(auto& item: constant->getConstantsMap()){
// 		std::cout << item.first << " -> " << item.second << std::endl;
// 	}
// }

//----------------------------------------------------------------------------------------------

// std::unique_ptr<PerPlayer> parsePerPlayer(const nlohmann::json& j) {
// 	std::unique_ptr<PerPlayer> perPlayer = std::make_unique<PerPlayer>();

// 	for(auto& item : j.items()){
// 		std::string ruleName = item.key();
// 		DataType value;
// 		definingDataType(item.value(),value);

// 		perPlayer->insertToPlayerMap(ruleName,value);
// 	}
// 	return perPlayer;
// }

// void parsePerPlayerTest(const nlohmann::json& gameConfig){
// 	nlohmann::json perPlayerConfig = DivideSection(gameConfig,"per-player");
// 	std::unique_ptr<PerPlayer> player = parsePerPlayer(perPlayerConfig);

// 	for(auto& item: player->getPerPlayer()){
// 		std::cout << item.first << " -> " << item.second << std::endl;
// 	}

// }

//---------------------------------------------------------------------------------------------
// std::unique_ptr<Variables> parseVariables(const nlohmann::json& j) {
// 	std::unique_ptr<Variables> variables = std::make_unique<Variables>();
// 	for (auto& item : j.items()) { 
// 		if (item.key().compare("variables") == 0) {
// 			variables->setWinners(item.value()[""]);
// 		}
// 	}
// 	return variables;
// }

// void parseVariablesTest(const nlohmann::json& gameConfig){
// 	nlohmann::json parseVariableConfig = DivideSection(gameConfig,"per-player");
// 	std::unique_ptr<PerPlayer> Variables = parsePerPlayer(parseVariableConfig);

// 	for(auto& item: Variables->getPerPlayer()){
// 		std::cout << item.first << " -> " << item.second << std::endl;
// 	}

// }


// //parseRule function recursively searching for "rule" key and print out the value (name of the rule)
// void parseRule(const nlohmann::json& j){
// 	if(j.is_object()){
// 		for (const auto& item: j.items()){
// 			if (!item.key().compare("rule")){
// 				std::cout << item.value() << "\n";
// 			} else if(!item.key().compare("rules") || item.value().is_array()){
// 				parseRule(item.value());
// 			}
// 		}
// 	} else if (j.is_array()){
// 		// std::cout << j.size() << "\n";
// 		for (const auto& item: j){
// 			parseRule(item);
// 		}
// 	}
// }

// //Define type of value in item of json.xs
// void definingDataType( const nlohmann::basic_json<> &item, DataType& value){
// 	using type = nlohmann::json::value_t;	
// 	if (item.type() == type::number_unsigned){
// 		unsigned temp = item;
// 		value = temp;
		
// 	} else if (item.type() == type::number_integer){
// 		int temp = item;
// 		value = temp;
// 	} else if (item.type() == type::boolean){
// 		bool temp = item;
// 		value = temp;
// 	} else if (item.type() == type::string){
// 		std::string temp = item;
// 		value = temp;
// 	}
// }

