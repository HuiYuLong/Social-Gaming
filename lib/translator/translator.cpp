#include "include/translator.h"

using namespace std;


nlohmann::json DivideSection(const nlohmann::json& j,std::string name){
	for(auto& item: j.items()){
		if((item.key() == name)){
			return item.value();
		}
		
	}
	return nullptr;
}


//---------------------------------------------------------------------------------------------------------------------
template<class Key, class Value>
std::unique_ptr<Constants<Key, Value>> parseConstants(const nlohmann::json& constantsConfig) {
	std::unique_ptr<Constants<Key, Value>> constants = std::make_unique<Constants<Key, Value>>();
	for (auto& item : constantsConfig.items()) {
		for (auto& item2 : item.value().items()) {
			Key k = item2.value()["name"];
			Value v = item2.value()["beats"];
			constants->insertToAssignments(k,v);
		}
	}
	return constants;
}

void parseConstantsTest(const nlohmann::json& gameConfig){
	nlohmann::json constants = DivideSection(gameConfig,"constants");
	std::unique_ptr<Constants<std::string,std::string>> constant = parseConstants<std::string,std::string>(constants);
	for(auto& item: constant->getAssignments()){
		std::cout << item.first << " -> " << item.second << std::endl;
	}
}

//----------------------------------------------------------------------------------------------

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
//--------------------------------------------------------------------------------------------
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

    // parsePerPlayerTest(gameConfig);
    // parseConstantsTest(gameConfig);
	// nlohmann::json perAudience = DivideSection(gameConfig,"per-audience");

	RuleTree ruleTree(gameConfig);

	return 0;
}

// under construction
// requires more group meetings to flesh out a more concrete structure