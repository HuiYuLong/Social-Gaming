#include "include/translator.h"
#include <unordered_map>
#include "boost/variant.hpp"
#include <iostream>
using namespace std;

using json = nlohmann::json;


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

//Define type of value in item of json.xs
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

	nlohmann::json config = CropSection(gameConfig,"configuration");

	nlohmann::json variable = CropSection(gameConfig,"variable");

	Top_levelMap topMap(config);

	

// 	  // print values
//    13     cout << object << '\n';
//    14     cout << null << '\n';
//    15  
//    16     // add values
//    17     object.push_back(json::object_t::value_type("three", 3));
//    18     object += json::object_t::value_type("four", 4);
//    19     null += json::object_t::value_type("A", "a");
//    20     null += json::object_t::value_type("B", "b");
//    21  
//    22     // print values
//    23     std::cout << object << '\n';
//    24     std::cout << null << '\n';
//    25 }


	//cout<<config.dump(4).push_back(variable)<<endl;




	// nlohmann::json perPlayerConfig = CropSection(gameConfig,"per-player");
	// nlohmann::json ConfigurationConfig = DivideSection(gameConfig,"configuration");
	// cout<<ConfigurationConfig<<endl;

	


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
