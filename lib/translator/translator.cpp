#include "include/translator.h"



std::unique_ptr<Constants> parseConstants(const nlohmann::json& j) {
	std::unique_ptr<Constants> constants = std::make_unique<Constants>();
	for (auto& item : j.items()) {
		if (item.key().compare("constants") == 0) {
			for (auto& item : item.value()["weapons"].items()) {
				constants->insertToWeapons(item.value()["name"],item.value()["beats"]);
				//std::cout << item.value()["name"] << std::endl;
				//std::cout << item.value()["beats"] << std::endl;
			}
		}
	}
	return constants;
}

std::unique_ptr<PerPlayer> parsePerPlayer(const nlohmann::json& j) {
	std::unique_ptr<PerPlayer> perPlayer = std::make_unique<PerPlayer>();
	for (auto& item: j.items()) {
		if (item.key().compare("per-player") == 0) {
			perPlayer->setWins(item.value()["wins"]);
		}
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

std::unique_ptr<Variables> parseVariables(const nlohmann::json& j) {
	std::unique_ptr<Variables> variables = std::make_unique<Variables>();
	for (auto& item : j.items()) { 
		if (item.key().compare("variables") == 0) {
			variables->setWinners(item.value()[""]);
		}
	}
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

	RuleTree ruleTree(gameConfig);

	return 0;
}

// under construction
// requires more group meetings to flesh out a more concrete structure