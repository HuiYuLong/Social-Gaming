#include "include/translator.h"
#include <unordered_map>
#include "boost/variant.hpp"
#include <iostream>
using namespace std;

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
//
// Todo: Extend, Shuffle, Sort, Deal, Discard & ListAttributes
//


//**** Arithmetic Operations ****//
AddRule::AddRule(const nlohmann::json& rule): to(rule["to"]), value(rule["value"]) { std::cout << "Add " << value << std::endl; }
//
// Todo: NumericalAttribues
//


//**** Timing ****//
//
// Todo: Timer
//


//**** Human Input ****//
//
// Todo: InputChoice, InputText & InputVote
//
InputChoiceRule::InputChoiceRule(const nlohmann::json& rule): to(rule["to"]), prompt(rule["prompt"]), choices(rule["choices"]), result(rule["result"]){
	std::cout << "Input Choice: " << rule["prompt"] << std::endl;
}

//**** Output ****//
ScoresRule::ScoresRule(const nlohmann::json& rule): score(rule["score"]) { 
	std::cout << "Score Board: "  << std::endl; 
	}

GlobalMessageRule::GlobalMessageRule(const nlohmann::json& rule): value(rule["value"]) { std::cout << "Global message: " << rule["value"] << std::endl; }
//
// Todo: Message
//



RuleTree::RuleTree(const nlohmann::json& gameConfig)
{
    for (const auto& it: gameConfig.items())
    {
        const nlohmann::json& rule = it.value();
        const std::string& rulename = rule["rule"];
        rules.push_back(rulemap[rulename](rule));
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


void LoopRule::run(PseudoServer& server, GameSpec& spec) {

	while (failCondition.evaluate(spec.getVariables())) {
		for (const auto& ptr : subrules) {
			ptr->run(server, spec);
		}
	}
}

void GlobalMessageRule::run(PseudoServer& server, GameSpec& spec)
{
	List& players = boost::get<List>(boost::get<Map>(spec.getVariables())["players"]);
	for (Variable& player : players) {
		const std::string& name = boost::get<std::string>(boost::get<Map>(player)["name"]);
		server.send({spec.getConnectionByName(name), value.fill_with(spec.getVariables())});
	}
}

void ReverseRule::run(PseudoServer& server, GameSpec& spec) {
	std::string toReverse = this->list;
	List& reverseList = boost::get<List>(boost::get<Map>(spec.getVariables())[toReverse]);
	std::reverse(reverseList.begin(), reverseList.end());
	//** For testing **//
	// for (Variable& weapon : reverseList) {
	// 	const std::string& weapons = boost::get<std::string>(boost::get<Map>(weapon)["name"]);
	// 	std::cout << "***After weapons***" << weapons << std::endl;
	// }
}


void ScoresRule::run(PseudoServer& server, GameSpec& spec)
{
	// List& players = boost::get<List>(boost::get<Map>(spec.getVariables())["players"]);
	// for (Variable& player : players) {
	// 	const std::string& name = boost::get<std::string>(boost::get<Map>(player)["name"]);
	// 	server.send({spec.getConnectionByName(name), value.fill_with(spec.getVariables())});
	// }
	std::cout<<"Score Board" <<std::endl;
	List& winners = boost::get<List>(boost::get<Map>(spec.getVariables())["winners"]);
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
		 server.send({spec.getConnectionByName(item.first), scorestring});

	 }
	 
}

void ForEachRule::run(PseudoServer& server, GameSpec& spec)
{
	Getter getter(list, spec.getVariables());
	GetterResult result = getter.get();
	List temp;
	List& elements = result.needs_to_be_saved ? temp = std::move(boost::get<List>(result.result)), temp : boost::get<List>(result.result);
	Map& toplevel = boost::get<Map>(spec.getVariables());
	for (Variable& element : elements) {
		toplevel[element_name] = &element;
		//PrintTheThing p;
		//boost::apply_visitor(p, spec.getVariables());
		for (const auto& ptr : subrules) {
			ptr->run(server, spec);
		}
	}
}

void WhenRule::run(PseudoServer& server, GameSpec& spec)
{
	for(Case& current_case : cases) {
		if (current_case.condition.evaluate(spec.getVariables())) {
			for (const auto& ptr : current_case.subrules) {
				ptr->run(server, spec);
			}
			break;
		}
	}
}

void InputChoiceRule::run(PseudoServer& server, GameSpec& spec){

	List& players = boost::get<List>(boost::get<Map>(spec.getVariables())["players"]);
	Map& toplevel = boost::get<Map>(spec.getVariables());
	toplevel[to] = &players.front(); //first player
	const std::string& name = boost::get<std::string>(boost::get<Map>(players.front())["name"]); 
	server.send({spec.getConnectionByName(name), prompt.fill_with(spec.getVariables())});
	List& weapons = boost::get<List>(boost::get<Map>(spec.getVariables())["weapons"]);
	for(auto weapon:weapons){
		const std::string& weaponName = boost::get<std::string>(boost::get<Map>(weapon)["name"]);
		server.send({spec.getConnectionByName(name), weaponName});
	}
	//NEED more test on this
	std::string choice;	
	std::cin >> choice;
	server.send({spec.getConnectionByName(name), choice});
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



void AddRule::run(PseudoServer& server, GameSpec& spec)
{
	Getter getter(to, spec.getVariables());
	GetterResult result = getter.get();
	assert(!result.needs_to_be_saved);
	int& integer = boost::get<int>(result.result);
	integer += value;
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

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Usage:\n  " << argv[0] << "<server config>\n"
				<< "  e.g. " << argv[0] << " ../configs/server/congfig1.json\n";
		return 1;
	}

	// Variable a = "a";
	// boost::apply_visitor(TEST(), a);
	// a = Query{"b"};
	// boost::apply_visitor(TEST(), a);
	// return 0;

	std::ifstream serverconfig{argv[1]};
	if (serverconfig.fail())
    {
        std::cout << "cannot open the congiguration file" << std::endl;
        return 0;
    }
	nlohmann::json j = nlohmann::json::parse(serverconfig);

	std::vector<GameSpec> configurations;
	std::vector<Player> players;
	for (const std::string& name : {"a", "b"})
		players.emplace_back(name, Connection());
	configurations.reserve(j["games"].size());
	for ([[maybe_unused]] const auto& [ key, gamespecfile]: j["games"].items())
	{
		std::ifstream gamespecstream{gamespecfile};
		if (gamespecstream.fail())
		{
			std::cout << "cannot open the game configuration file " << gamespecfile << std::endl;
			return 0;
		}
		nlohmann::json gamespec = nlohmann::json::parse(gamespecstream);
		configurations.emplace_back(gamespec, players);
		std::cout << "\nTranslated game " << key << "\n\n";
    }

	// TEST
	Variable& variables = configurations.front().getVariables();
	PrintTheThing p;
	boost::apply_visitor(p, variables);

	std::cout << "\nStarting a test\n\n";
	PseudoServer server;
	std::thread t1 = configurations.front().launchGameDetached(server);
	// std::thread t2 = configurations.back().launchGameDetached(server);
	t1.join();
	// t2.join();
	//PseudoServer server;
	//configurations.back().launchGame(server);
	// for(GameSpec& c : configurations) {
	// 	std::cout << "\nGame " << c.getName() << "\n\n";
	// 	c.launchGame(server);
	// }
	std::cout << "\nFinished\n";

	return 0;
};












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
