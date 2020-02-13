#include "include/translator.h"
#include <unordered_map>
#include "boost/variant.hpp"
#include <iostream>
using namespace std;

// code from group a


class my_visitor : public boost::static_visitor<int>
{
public:
    int operator()(int i) const
    {
        return i;
    }
    
    int operator()(const std::string & str) const
    {
        return str.length();
    }
};
using MapValue  = std::unordered_map<std::string, boost::recursive_variant_>;

enum DataKind {
INTEGER,
STRING,
BOOLEAN,
QUESTION_ANSWER,
MULTIPLE_CHOICE
};

struct MapSetupValue {
DataKind kind;
std::string prompt;
};

using MapValue  = std::unordered_map<std::string, boost::recursive_variant_>;
using ValueList = std::vector<boost::recursive_variant_>;


using SetupValue = boost::make_recursive_variant<
	MapSetupValue,
	std::string,
	int, 
	bool
>::type;

using EnvValue = boost::make_recursive_variant<
    MapValue,
    ValueList, 
    std::string,
    int, 
    bool
  >::type;




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

// }
// //--------------------------------------------------------------------------------------------
// std::unique_ptr<Configuration> parseConfiguration(const nlohmann::json& j) {
// 	std::unique_ptr<Configuration> configuration = std::make_unique<Configuration>();
// 	for (auto& item : j.items()) {
// 		if (item.key().compare("constants") == 0) {
// 			for (auto& item : item.value()["weapons"].items()) {
// 				constants->insertToWeapons(item.value()["name"],item.value()["beats"]);
// 				//std::cout << item.value()["name"] << std::endl;
// 				//std::cout << item.value()["beats"] << std::endl;
// 			}
// 		}
// 	}
// 	return constants;
// }

template<class Key, class Value>
std::unique_ptr<PerPlayer<Key,Value>> parsePerPlayer(const nlohmann::json& pp) {
	std::unique_ptr<PerPlayer<Key,Value>> perPlayer = std::make_unique<PerPlayer<Key,Value>>();
	for(auto& item : pp.items()){
		Key k = item.key();
		Value v = item.value();
		perPlayer->insertToPlayerMap(k,v);
	}
	return perPlayer;
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
	// nlohmann::json gameConfig = nlohmann::json::parse(jsonFileStream);
	// nlohmann::json perPlayerConfig = DivideSection(gameConfig,"per-player");

	// nlohmann::json ConfigurationConfig = DivideSection(gameConfig,"configuration");
	// cout<<ConfigurationConfig<<endl;

    // using dataType =  boost::variant< std::string,bool,int >;
	// dataType stock=100;
	// dataType pen=("pen");

	// cout <<pen<<" "<<stock<<endl;
	// typedef boost::variant<int, string, bool> dataType;
	// dataType u="hello world";
	// dataType storck=4;  

	// string s=boost::get<string>(u);
    // std::cout << s<<" "<<storck<<endl; // output: hello world
	


	unordered_map<std::string, dataType> m;
	cout<<"test\n";
	m.emplace("hi", true);
	m.emplace("hello", 2);
	m.emplace("third", "hello world");
	m["third"]="hello word";

	for (auto&item :m){
		std::cout << item.first<< " " << item.second << "\n";

	}
	// m["name"]= "Rock, Paper, Scissors";

	// m["bl"]= true;
	
	

	// std::unique_ptr<PerPlayer<std::string,int>> player = parsePerPlayer<std::string,int>(perPlayerConfig);

   std::unique_ptr<Configuration<std::string,dataType>> Configuration = parseConfiguration<std::string,dataType>(ConfigurationConfig);


	// output: hello world

	// for(auto& item: player->getPerPlayer()){
	// 	std::cout << item.first << " -> " << item.second << std::endl;
	// }

	
	// for(auto& item: Configuration->getConfigurationMap()){
	// 	std::cout << item.first << " -> " << item.second << std::endl;
	// }


	


	// RuleTree ruleTree(gameConfig);

	return 0;
};

// under construction
// requires more group meetings to flesh out a more concrete structure