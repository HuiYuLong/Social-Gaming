#include "include/translator.h"

using namespace std;


template<class Key, class Value>
std::unique_ptr<Constants<Key, Value>> parseConstants(const nlohmann::json& constantsConfig) {
	std::unique_ptr<Constants<Key, Value>> constants = std::make_unique<Constants<Key, Value>>();
	for (auto& item : constantsConfig.items()) {
		for (auto& item2 : item.value().items()) {
			//std::cout << item2.value() << std::endl;
			Key k = item2.value()["name"];
			Value v = item2.value()["beats"];
			//std::cout << k << std::endl;
			//std::cout << v << std::endl;
			constants->insertToAssignments(k,v);

			// if (item2.value().is_array()) {
			// 	std::cout << item2.key() << std::endl;
			// 	}
			//  else {
			// 	Key k = item2.key();
			// 	Value v = item2.value();
			// 	constants->insertToAssignments(k,v);
			// }

		}
	}
	return constants;
}


nlohmann::json DivideSection(const nlohmann::json& j,std::string name){
	for(auto& item: j.items()){
		if((item.key() == name)){
			return item.value();
		}
		
	}
	return nullptr;
}

template<class Key, class Value>
std::unique_ptr<PerPlayer<Key,Value>> parsePerPlayer(const nlohmann::json& j) {
	std::unique_ptr<PerPlayer<Key,Value>> perPlayer = std::make_unique<PerPlayer<Key,Value>>();
	for(auto& item : j.items()){
		Key k = item.key();
		Value v = item.value();
		perPlayer->insertToPlayerMap(k,v);
	}
	return perPlayer;
}


std::unique_ptr<Configuration> parseConfiguration(const nlohmann::json& j) {
	std::unique_ptr<Configuration> configuration = std::make_unique<Configuration>();
	for (auto& item : j.items()) {
		if (item.key().compare("configuration") == 0) {
			configuration->setName(item.value()["name"]);
			std::cout << item.value()["name"] << std::endl;
			configuration->setPlayerCountMin(item.value()["player count"]["min"]);
			configuration->setPlayerCountMax(item.value()["player count"]["max"]);
			configuration->setAudience(item.value()["audience"]);
			configuration->setRound(item.value()["setup"]["Rounds"]);
		}
	}

	return configuration;

}

template<class Key, class Value>
std::unique_ptr<Variables<Key, Value>> parseVariables(const nlohmann::json& variablesConfig) {
	std::unique_ptr<Variables<Key, Value>> variables = std::make_unique<Variables<Key, Value>>();
	for (auto& item : variablesConfig.items()) {
		//std::cout << item.value() << std::endl;
		Key k = item.key();
		Value v = item.value();
		variables->insertToVariablesList(k,v);
	}
	//std::cout << "PV called!" << std::endl;
	return variables;
}


//parseRule function recursively searching for "rule" key and print out the value (name of the rule)
void parseRule(const nlohmann::json& j){
	if(j.is_object()){
		for (const auto& item: j.items()){
			if (!item.key().compare("rule")){
				std::cout << item.value() << "\n";
			} else if(!item.key().compare("rules") || item.value().is_array()){
				parseRule(item.value());
			}
		}
	} else if (j.is_array()){
		// std::cout << j.size() << "\n";
		for (const auto& item: j){
			parseRule(item);
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 2)
	{
		std::cout << "Pass the json file location as the first parameter" << std::endl;
		// ex) ../../configs/games/simple2.json
		return 1;
	}
	std::ifstream jsonFileStream(argv[1]); // read file
	// To make sure you can open the file
    if (jsonFileStream.fail())
    {
        std::cout << "cannot open file" << std::endl;
        return 0;
    }


	nlohmann::json gameConfig = nlohmann::json::parse(jsonFileStream);
	nlohmann::json perPlayerConfig = DivideSection(gameConfig,"per-player");
	nlohmann::json perAudience = DivideSection(gameConfig,"per-audience");
	nlohmann::json constants = DivideSection(gameConfig,"constants");
	nlohmann::json variableConfig = DivideSection(gameConfig,"variables");

	std::unique_ptr<PerPlayer<std::string,int>> player = parsePerPlayer<std::string,int>(perPlayerConfig);
	for(auto& item: player->getPerPlayer()){
		std::cout << item.first << " -> " << item.second << std::endl;
	}

	std::unique_ptr<Constants<std::string,std::string>> constant = parseConstants<std::string,std::string>(constants);
	for(auto& item: constant->getAssignments()){
		std::cout << item.first << " -> " << item.second << std::endl;
	}

	//std::cout << variableConfig << std::endl;
	//assuming the winner variable contain an array of strings "winners": ["playername1","playername2"] in json file
	//the first item is string, second is a string array
	std::unique_ptr<Variables<std::string,std::vector<std::string>>> var = parseVariables<std::string,std::vector<std::string>>(variableConfig);
	for(auto& item: var->getWinners()){
	 	std::cout << item.first << " -> ";
		//print every element in the string array
		for(auto& str:item.second){
			std::cout << str << " ";
		}
		std::cout << std::endl;
	}

	// RuleTree ruleTree(gameConfig);

	return 0;
}

// under construction
// requires more group meetings to flesh out a more concrete structure