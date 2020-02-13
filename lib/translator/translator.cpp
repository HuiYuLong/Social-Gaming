#include "include/translator.h"
#include <unordered_map>
#include "boost/variant.hpp"
#include <iostream>
using namespace std;


//Helper functions
nlohmann::json DivideSection(const nlohmann::json& j,std::string name){
	for(auto& item: j.items()){
		if((item.key() == name)){
			return item.value();
		}
		
	}
	return nullptr;
}

void definingDataType( const nlohmann::basic_json<> &item, DataType& value){
	using type = nlohmann::json::value_t;	
	if (item.type() == type::number_unsigned){
		unsigned temp = item;
		value = temp;
	} else if (item.type() == type::number_integer){
		int temp = item;
		value = temp;
	} else if (item.type() == type::boolean){
		bool temp = item;
		value = temp;
	} else if (item.type() == type::string){
		std::string temp = item;
		value = temp;
	}
}

std::unique_ptr<Configuration> parseConfiguration(const nlohmann::json& ConfigurationConfig){
	
	//to do something
	
}

//---------------------------------------------------------------------------------------------------------------------

std::unique_ptr<Constants> parseConstants(const nlohmann::json& constantsConfig) {
	std::unique_ptr<Constants> constants = std::make_unique<Constants>();
	for (auto& item : constantsConfig.items()) {
		for (auto& item2 : item.value().items()) {
			string k = item2.value()["name"];
			string v = item2.value()["beats"];
			constants->insertToConstantsMap(k,v);
		}
	}
	return constants;
}

void parseConstantsTest(const nlohmann::json& gameConfig){
	nlohmann::json constants = DivideSection(gameConfig,"constants");
	std::unique_ptr<Constants> constant = parseConstants(constants);
	for(auto& item: constant->getConstantsMap()){
		std::cout << item.first << " -> " << item.second << std::endl;
	}
}

//----------------------------------------------------------------------------------------------

std::unique_ptr<PerPlayer> parsePerPlayer(const nlohmann::json& j) {
	std::unique_ptr<PerPlayer> perPlayer = std::make_unique<PerPlayer>();

	for(auto& item : j.items()){
		std::string ruleName = item.key();
		DataType value;
		definingDataType(item.value(),value);

		perPlayer->insertToPlayerMap(ruleName,value);
	}
	return perPlayer;
}

void parsePerPlayerTest(const nlohmann::json& gameConfig){
	nlohmann::json perPlayerConfig = DivideSection(gameConfig,"per-player");
	std::unique_ptr<PerPlayer> player = parsePerPlayer(perPlayerConfig);

	for(auto& item: player->getPerPlayer()){
		std::cout << item.first << " -> " << item.second << std::endl;
	}

}

//---------------------------------------------------------------------------------------------
std::unique_ptr<Variables> parseVariables(const nlohmann::json& j) {
	std::unique_ptr<Variables> variables = std::make_unique<Variables>();
	for (auto& item : j.items()) { 
		if (item.key().compare("variables") == 0) {
			variables->setWinners(item.value()[""]);
		}
	}
	return variables;
}

void parseVariablesTest(const nlohmann::json& gameConfig){
	nlohmann::json parseVariableConfig = DivideSection(gameConfig,"per-player");
	std::unique_ptr<PerPlayer> Variables = parsePerPlayer(parseVariableConfig);

	for(auto& item: Variables->getPerPlayer()){
		std::cout << item.first << " -> " << item.second << std::endl;
	}

}


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
	// nlohmann::json ConfigurationConfig = DivideSection(gameConfig,"configuration");
	// cout<<ConfigurationConfig<<endl;

	parsePerPlayerTest(gameConfig);
	parseConstantsTest(gameConfig);

	

	// RuleTree ruleTree(gameConfig);

	return 0;
};

// under construction
// requires more group meetings to flesh out a more concrete structure