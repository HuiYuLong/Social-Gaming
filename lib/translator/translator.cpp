#include "translator.h"
#include <unordered_map>
#include "boost/variant.hpp"
#include <iostream>
#include <algorithm>
#include <random>
#include <ctime>

using namespace std;

Variable buildVariables(const nlohmann::json& json)
{
    if (json.is_boolean()) {
        //std::cout << json << std::endl;
        return (Variable) (bool) json;
    }
    else if (json.is_number()) {
        //std::cout << json << std::endl;
        return (Variable) (int) json;
    }
    else if (json.is_string()) {
        //std::cout << json << std::endl;
        return (Variable) (std::string) json;
    }
    else if (json.is_object()) {
        Map map;
        for(const auto&[key, value]: json.items()) {
            //std::cout << key << std::endl;
            map[key] = buildVariables(value);
        }
        return (Variable) map;
    }
    else if (json.is_array()) {
        List list;
        for(const auto&[key, value]: json.items()) {
            //std::cout << key << std::endl;
            list.push_back(buildVariables(value));
        }
        return (Variable) list;
    }
    else {
        std::cout << "Invalid JSON variable type" << std::endl;
        std::terminate();
    }
}

std::regex Condition::equality_regex("\\s*(\\S+)\\s*==\\s*(\\S+)\\s*");
std::regex Condition::decimal_regex("\\d+");
//std::regex Condition::variable_regex("(\\w+(\\(\\w+\\)))?\\w+(\\(\\w+\\)))?");

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

//**** Control Structures ****//
ForEachRule::ForEachRule(const nlohmann::json& rule): list(rule["list"]), element_name(rule["element"])
{
    std::cout << "For each: " << element_name << std::endl;
    for (const auto& it : rule["rules"].items())
    {
		subrules.push_back(rulemap[it.value()["rule"]](it.value()));
    }
}

LoopRule::LoopRule(const nlohmann::json& rule): failCondition(rule["while"]) {
	std::cout << "Loop" << std::endl;
	for (const auto& it : rule["rules"].items()) {
		subrules.push_back(rulemap[it.value()["rule"]](it.value()));
	}
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

Case::Case(const nlohmann::json& case_): condition(case_["condition"]) {
	for (const auto& subrule : case_["rules"].items()) {
		subrules.push_back(rulemap[subrule.value()["rule"]](subrule.value()));
	}
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

DiscardRule::DiscardRule(const nlohmann::json& rule):from(rule["from"]), count(rule["count"]){
	std::cout << "Discard Variable: " << from << std::endl;
	std::cout << "Variable Size: " << count << std::endl;

}

ExtendRule::ExtendRule(const nlohmann::json& rule): list(rule["list"]), target(rule["target"]) {
	std::cout << "Extend: " << list << std::endl;
	std::cout << "Extend: " << target << std::endl;
}

//**** Arithmetic Operations ****//
AddRule::AddRule(const nlohmann::json& rule): to(rule["to"]), value(rule["value"]) { std::cout << "Add " << value << std::endl; }
//
// Todo: NumericalAttribues
//


//**** Timing ****//
TimerRule::TimerRule(const nlohmann::json& rule): duration(rule["duration"]), mode(rule["mode"]) {
	std::cout << "Timer: " << duration << std::endl;
    for (const auto& it : rule["rules"].items())
    {
		subrules.push_back(rulemap[it.value()["rule"]](it.value()));
    }
}

PauseRule::PauseRule(const nlohmann::json& rule): duration(rule["duration"]) { }


//**** Human Input ****//
//
// Todo: InputChoice, InputText & InputVote
//
InputChoiceRule::InputChoiceRule(const nlohmann::json& rule): to(rule["to"]), prompt(rule["prompt"]), choices(rule["choices"]), result(rule["result"]){
	std::cout << "Input Choice: " << rule["prompt"] << std::endl;
}
InputTextRule::InputTextRule(const nlohmann::json& rule): to(rule["to"]), prompt(rule["prompt"]), result(rule["result"]){
	std::cout << "Input Text: " << rule["prompt"] << std::endl;
}
InputVoteRule::InputVoteRule(const nlohmann::json& rule): to(rule["to"]), prompt(rule["prompt"]), choices(rule["choices"]),result(rule["result"]){
	std::cout << "Input vote: " << rule["prompt"] << std::endl;
}

//**** Output ****//
ScoresRule::ScoresRule(const nlohmann::json& rule): score(rule["score"]) { 
	std::cout << "Score Board: "  << std::endl; 
	}

GlobalMessageRule::GlobalMessageRule(const nlohmann::json& rule): value(rule["value"]) { std::cout << "Global message: " << rule["value"] << std::endl; }

MessageRule::MessageRule(const nlohmann::json& rule): to(rule["to"]), value(rule["value"]) { 
	std::cout << "message: " << rule["value"] << std::endl; 
}



RuleTree::RuleTree(const nlohmann::json& gameConfig)
{
    for (const auto& it: gameConfig.items())
    {
        const nlohmann::json& rule = it.value();
        const std::string& rulename = rule["rule"];
        rules.push_back(rulemap[rulename](rule));
    }
}

void RuleTree::launchGame(Server& server, GameState& state)
{
	for (const auto& ptr : rules) {
		ptr->run(server, state);
	}
}

RuleTree::RuleTree(RuleTree&& oldTree)
{
    for (std::unique_ptr<Rule>& ptr : oldTree.getRules()) {
        rules.push_back(std::move(ptr));
    }
}

RuleTree& RuleTree::operator=(RuleTree&& oldTree)
{
    rules.clear();
    for (std::unique_ptr<Rule>& ptr : oldTree.getRules()) {
        rules.push_back(std::move(ptr));
    }
    return *this;
}

RuleList& RuleTree::getRules() { return rules; }


void LoopRule::run(Server& server, GameState& state) {

	while (failCondition.evaluate(state.getVariables())) {
		for (const auto& ptr : subrules) {
			ptr->run(server, state);
		}
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

	List& ExtendList = boost::get<List>(boost::get<Map>(state.getVariables())[this->list]);
	List& Target = boost::get<List>(boost::get<Map>(state.getVariables())[this->target]);
	//it=Target.begin();
	
	//const std::string& name = boost::get<std::string>(boost::get<Map>(players.front())["name"]); 
	cout<<"Extend begin\n";
	Target.insert(Target.end(), ExtendList.begin(), ExtendList.end());

	for(auto weapon:Target){
		const std::string& weaponName = boost::get<std::string>(boost::get<Map>(weapon)["name"]);
		cout<<weaponName<<endl;
		const std::string& beatName = boost::get<std::string>(boost::get<Map>(weapon)["beats"]);
		
		cout<<weaponName<<" beat "<<beatName<<endl;
		// server.send({state.getConnectionByName(name), weaponName});
	}

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
	List& list = boost::get<List>(boost::get<Map>(state.getVariables())[std::string(from)]);
	if(list.size() == count){
		list.clear();
		std::cout<<"winner list have been discarded" <<std::endl;
	}else
	{
		std::cout<<"list size not equal to count" <<std::endl;
	}
	
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
	std::cout<<"Score Board" <<std::endl;
	List& winners = boost::get<List>(boost::get<Map>(state.getVariables())["winners"]);
	bool IsAscending;
	std::string scorestring;
	vector<std::pair<std::string, int>> scoreBoard;

	 for (Variable& winner : winners) {
		const std::string& name = boost::get<std::string>(boost::get<Map>(winner)["name"]);
		const int win = boost::get<int>(boost::get<Map>(winner)["wins"]);
		IsAscending = boost::get<bool>(boost::get<Map>(winner)["ascending"]);
		//std::cout<<name<<": "<<wins<<std::endl;
		scoreBoard.push_back(std::pair<std::string,int>(name,win));
	 }
	 if (!IsAscending){

		 std::sort(scoreBoard.begin(),scoreBoard.end(), [](auto item1,auto item2){
			 return item1.second > item2.second;
		 });

	 }else{
		  std::sort(scoreBoard.begin(),scoreBoard.end(), [](auto item1,auto item2){
			 return item1.second < item2.second;
		 });

	 }
	 for(auto item:scoreBoard){
		scorestring.append(item.first);
		scorestring.append(": ");
		scorestring.append(to_string(item.second));
		scorestring.append("\n");

	 }
	 for(auto item:scoreBoard){
		 server.send({state.getConnectionByName(item.first), scorestring });

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
		toplevel[element_name] = &element;
		//PrintTheThing p;
		//boost::apply_visitor(p, state.getVariables());
		for (const auto& ptr : subrules) {
			ptr->run(server, state);
		}
	}
}

void WhenRule::run(Server& server, GameState& state)
{
	for(Case& current_case : cases) {
		if (current_case.condition.evaluate(state.getVariables())) {
			for (const auto& ptr : current_case.subrules) {
				ptr->run(server, state);
			}
			break;
		}
	}
}


void InputChoiceRule::run(Server& server, GameState& state){ //IT'S WORKING
	//Send message to the player
	Getter getterTo(to, state.getVariables());
	GetterResult resultTo = getterTo.get();
	Map& p = boost::get<Map>(resultTo.result);
	const std::string& name = boost::get<std::string>(p["name"]);
	server.send({state.getConnectionByName(name), prompt.fill_with(state.getVariables()) });
	
	//send Choice list to the player
	if(choices.find(".name")!= string::npos){ //might have a better way to do this, but it works
		choices.erase(choices.size()-5, 5);
	}
	List& choiceList = boost::get<List>(boost::get<Map>(state.getVariables())[choices]);
	vector<std::string> choiceCheck; //vector to check if the choice valid
	for(auto choice:choiceList){
		const std::string choiceName = boost::get<std::string>(boost::get<Map>(choice)["name"]);
		choiceCheck.push_back(choiceName);
		server.send({state.getConnectionByName(name), choiceName + "\t"});
	}	
	server.send({state.getConnectionByName(name), "\n"});
	
	//read user input
	bool isReceived = false;
	bool isValid = false;
	std::string input = "";
	while(!isValid){
		isReceived = false;
		while(!isReceived){
			Connection connection = state.getConnectionByName(name);
			auto received = server.receive(connection);
			if(received.has_value()){
				input = std::move(received.value());
				server.send({connection,input});
				isReceived = true;
			}
		}
		isValid = std::any_of(choiceCheck.begin(), choiceCheck.end(), [&input](auto &item){
			return (input.compare(item) == 0);
		});

		if(!isValid){
			server.send({state.getConnectionByName(name),"Please input correct choice!"});
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
	//Send message to the player/audience list
	List& players = boost::get<List>(boost::get<Map>(state.getVariables())[to]);
	for (Variable& player : players) {
		const std::string& name = boost::get<std::string>(boost::get<Map>(player)["name"]);
		server.send({state.getConnectionByName(name), prompt.fill_with(state.getVariables()) });
	}
	//defining list or list name
	if(choices.find(".name")!= string::npos){ //might have a better way to do this, but it works
		choices.erase(choices.size()-5, 5);
	}
	List& choiceList = boost::get<List>(boost::get<Map>(state.getVariables())[choices]);
	vector<std::string> choiceCheck; //vector to check if the choice valid
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

void TimerRule::run(Server& server, GameState& state) {
	// Todo: An "exact" timer will pad the execution time to the given duration
	std::cout << mode << std::endl;
	std::clock_t start;
    start = std::clock();
	bool flag = false;

	float timer = float(std::clock()-start)/CLOCKS_PER_SEC;
	for (const auto& ptr : subrules) {
		timer = float(std::clock()-start)/CLOCKS_PER_SEC;
		if( (timer>duration) && (mode == "exact")) {
			std::cout << "times up!" << std::endl;
			// TODO: send a msg to the server to stop the rest of the rules
			return;
		}
		else if( (timer>duration) && (mode == "track")) {
			std::cout << "!!flag" << std::endl;
		}
		ptr->run(server, state);
	}
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

