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
        // {"inparallel", [](const nlohmann::json&rule) {return std::make_unique<InParallelRule>(rule);}},
        // {"parallelfor", [](const nlohmann::json&rule) {return std::make_unique<ParallelForRule>(rule);}},
        //{"switch", [](const nlohmann::json&rule) {return std::make_unique<SwitchRule>(rule);}},
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

void RuleList::run(Server& server, GameState& state)
{
	for (const auto& ptr : rules) {
		ptr->run(server, state);
		if(!state.checkCallbacks()) {
			return;
		}
	}
}

//**** Control Structures ****//
ForEachRule::ForEachRule(const nlohmann::json& rule): list(rule["list"]), element_name(rule["element"]), subrules(rule["rules"])
{
    std::cout << "For each: " << element_name << std::endl;
}

LoopRule::LoopRule(const nlohmann::json& rule): failCondition(rule["while"]), subrules(rule["rules"]) {
	std::cout << "Loop" << std::endl;
}
	
WhenRule::WhenRule(const nlohmann::json& rule)
{
    std::cout << "When" << std::endl;
	cases.reserve(rule["cases"].size());
    for(const auto& case_ : rule["cases"].items()) {
        std::cout << case_.value()["condition"] << std::endl;
        cases.emplace_back(case_.value());
    }
}

Case::Case(const nlohmann::json& case_): condition(case_["condition"]), subrules(case_["rules"]) {
	std::cout << "Case " << case_["condition"] << std::endl;
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

TimerRuleImplementation::TimerRuleImplementation(int duration, const nlohmann::json& json_subrules): duration(duration), subrules(json_subrules)
{
	std::cout << "Timer with duration " << duration << std::endl;
}

AtMostTimer::AtMostTimer(int duration, const nlohmann::json& json_subrules): TimerRuleImplementation(duration, json_subrules) { }

void AtMostTimer::run(Server& server, GameState& state)
{
	timer = std::make_unique<Timer>(duration);
	state.registerCallback(*this);
	subrules.run(server, state);
	state.deregisterCallback(*this);
}

bool AtMostTimer::check(GameState&)
{
	return timer->hasnt_expired();	// returns false when the timer has expired, which will force all subrules to shut down
}


ExactTimer::ExactTimer(int duration, const nlohmann::json& json_subrules): TimerRuleImplementation(duration, json_subrules) { }

void ExactTimer::run(Server& server, GameState& state)
{
	// Just repeats the subrules until the timer expires
	timer = std::make_unique<Timer>(duration);
	state.registerCallback(*this);
	subrules.run(server, state);
	state.deregisterCallback(*this);
	while (timer->hasnt_expired()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

bool ExactTimer::check(GameState&)
{
	return timer->hasnt_expired();	// returns false when the timer has expired, which will force all subrules to shut down
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
	timer = std::make_unique<Timer>(duration);

	// Run the subrules while checking on the timer
	state.registerCallback(*this);
	subrules.run(server, state);
	state.deregisterCallback(*this);
}

bool TrackTimer::check(GameState& state)
{
	if (timer && !timer->hasnt_expired()) {
		// Set the flag to true
		Getter getter(flag, state.getVariables());
		GetterResult result = getter.get();
		assert(!result.needs_to_be_saved);
		result.result = true;
		// Disable the timer
		timer.reset(nullptr);
	}
	return true; // track timer never stops the rules
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

	while (failCondition.evaluate(state.getVariables())) {
		subrules.run(server, state);
	}
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

void ForEachRule::run(Server& server, GameState& state)
{
	Getter getter(list, state.getVariables());
	GetterResult result = getter.get();
	List temp;
	List& elements = result.needs_to_be_saved ? temp = std::move(boost::get<List>(result.result)), temp : boost::get<List>(result.result);
	Map& toplevel = boost::get<Map>(state.getVariables());
	for (Variable& element : elements) {
		toplevel[element_name] = getReference(element);
		//PrintTheThing p;
		//boost::apply_visitor(p, state.getVariables());
		subrules.run(server, state);
	}
}

void WhenRule::run(Server& server, GameState& state)
{
	for(Case& current_case : cases) {
		if (current_case.condition.evaluate(state.getVariables())) {
			current_case.subrules.run(server, state);
			break;
		}
	}
}

void InputChoiceRule::run(Server& server, GameState& state){
	//  Get the player name
	Getter getter(to, state.getVariables());
	GetterResult player_result = getter.get();
	assert(!player_result.needs_to_be_saved);
	Map& player = boost::get<Map>(player_result.result);
	const std::string& player_name = boost::get<std::string>(player["name"]);
	Connection player_connection = state.getConnectionByName(player_name);

	// Write the prompt to buffer
	std::ostringstream buffer;
	buffer << prompt.fill_with(state.getVariables()) << std::endl;

	// Get the list of choices (from a <Query, List> variant)
	ResolveQuery<List> get_list_of_choices(state.getVariables());
	List& list_of_choices = boost::apply_visitor(get_list_of_choices, choices);
	if (list_of_choices.size() == 0u) {
		throw std::runtime_error{"Input Choice: list of choices must be non-empty"};
	}

	// Write the choices to buffer
	StringConverter to_string;
	for(size_t i = 0u; i < list_of_choices.size(); ++i){
		buffer << '\t' << i + 1 << ". " << boost::apply_visitor(to_string, list_of_choices[i]) << std::endl;
	}
	buffer << std::endl;
	server.send({player_connection, buffer.str()});
	
	// Read user input
	Timer timer(timeout.value_or(300));	// 5 minutes max
	size_t player_choice = list_of_choices.size();	// invalid choice
	while(timer.hasnt_expired()) {
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
		if (!state.checkCallbacks()) {
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

	// PrintTheThing p;
	// boost::apply_visitor(p, state.getVariables());
}

void InputTextRule::run(Server& server, GameState& state){ //IT'S WORKING
	Getter getter(to, state.getVariables());
	GetterResult result = getter.get();
	Map& p = boost::get<Map>(result.result);
	const std::string& name = boost::get<std::string>(p["name"]);
	server.send({state.getConnectionByName(name), prompt.fill_with(state.getVariables()) });

	bool isReceived = false;
	std::string input = "";

	while(!isReceived){
		Connection connection = state.getConnectionByName(name);
		auto received = server.receive(connection);
		if(received.has_value()){
			input = std::move(received.value());
			server.send({connection,input});
			isReceived = true;
		}
		if (!state.checkCallbacks()) {
			return;
		}
	}

	Getter getterResult(this->result, state.getVariables());
	GetterResult resultResult = getterResult.get();
	std::string& resultString = boost::get<std::string>(resultResult.result);
	resultString = input;

	//for testing
	// PrintTheThing p2;
    // boost::apply_visitor(p2, state.getVariables());

}


void InputVoteRule::run(Server& server, GameState& state){
	// Has to be placed somewhere to allow timers to terminate the rule
	// if (!state.checkCallbacks()) {
	// 	return;
	// }

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

