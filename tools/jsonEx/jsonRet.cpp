#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>

class Configuration {
public:
	std::string name;
	int playerCountMin;
	int playerCountMax;
	bool audience;
	nlohmann::json setup;
	Configuration(std::string name) {
		this->name = name;
	}
};

class Constants {
public:
};

class Variables {
public:
};

class PerPlayer {
public:
};

// maybe we should construct separate class for the different rule classes?
// ex) OutputRule, InputRule, ArithmeticRule, ListOpsRule, ControlStructRule ...

class Rule {
public:
	std::string type; // "foreach", "scores" -> the field under "rule": in the json
	int list;
	std::string element;
	//Graph rules; // a graph data structure to hold subrules ... implementation to be further discussed.
};

class Player {
public:
};

class Interpreter {
public:
	Configuration* configuration;
	Constants* constants;
	Interpreter(nlohmann::json gameConfig) {
		// filling out the Configuration object for the interpreter
		// example of iterating over the jsonObject
		for (auto& item : gameConfig.items()) {
			if (item.key().compare("configuration") == 0) {
				configuration = new Configuration(item.value()["name"]);
				std::cout << "We will be playing: " << item.value()["name"] << std::endl;
			}
		}

	}

	void createGame() { // more parameters to be added
		std::cout << "just cheking what we will be playing: " << configuration->name << std::endl;
	}

	void destoryGame() {
		delete this->configuration;
	}
};

int main(int argc, char **argv)
{
    nlohmann::json j;
    std::cout << "Import nlohmann succesful" << std::endl;

    std::ifstream jsonFileStream("../../configs/games/rock_paper_scissors.json"); // read file
	nlohmann::json jsonObject = nlohmann::json::parse(jsonFileStream);

	// auto name = jsonObject["configuration"]["name"].get<std::string>(); // both methods work just fine
	auto name = jsonObject["configuration"]["name"];
	auto playerCountMin = jsonObject["configuration"]["player count"]["min"];
	auto playerCountMax = jsonObject["configuration"]["player count"]["max"];
	auto audience = jsonObject["configuration"]["audience"];
	auto setup = jsonObject["configuration"]["setup"];
	auto constants = jsonObject["constants"];
	auto variables = jsonObject["variables"];
	auto perPlayer = jsonObject["per-player"];
	auto perAudience = jsonObject["per-audience"];
	auto rules = jsonObject["rules"];

	std::cout << name << std::endl;
	std::cout << playerCountMin << std::endl;
	std::cout << playerCountMax << std::endl;
	std::cout << audience << std::endl;
	std::cout << setup << std::endl;
	std::cout << constants << std::endl;
	std::cout << variables << std::endl;
	std::cout << perPlayer << std::endl;
	std::cout << perAudience << std::endl;
	std::cout << rules << std::endl;

	Interpreter* interpreter = new Interpreter(jsonObject);
	interpreter->createGame();
	interpreter->destoryGame();

    return 0;
}