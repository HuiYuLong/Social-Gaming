#include <interpreter.h>

class Interpreter {
public:
	Configuration* configuration;
	Constants* constants;
	Variables* variables;
	PerPlayer* perPlayer;
//	PerAudience* perAudience;
	Rule* rules;

	Interpreter(nlohmann::json gameConfig) {
		// filling out the Configuration object for the interpreter
		// example of iterating over the jsonObject
		for (auto& item : gameConfig.items()) {
			if (item.key().compare("configuration") == 0) {
				configuration = new Configuration(item.value()["name"],item.value()["player count"]["min"], item.value()["player count"]["max"], item.value()["audience"], item.value()["setup"]["Rounds"]);
				configuration->print();
			}
		}

	}

	void createGame() { // more parameters to be added
		std::cout << "just cheking what we will be playing: " << configuration->getName() << std::endl;
	}

	void destoryGame() {
		delete this->configuration;
	}

};

int main() {

	//std::istringstream iss("{\"json\": \"beta\"}"); // code manually create a small json file
	//nlohmann::json jsonObject = nlohmann::json::parse(iss);

	std::ifstream jsonFileStream("../../configs/games/rock_paper_scissors.json"); // read file
	nlohmann::json jsonObject = nlohmann::json::parse(jsonFileStream);

	// auto name = jsonObject["configuration"]["name"].get<std::string>(); // both methods work just fine
	// auto name = jsonObject["configuration"]["name"];
	// auto playerCountMin = jsonObject["configuration"]["player count"]["min"];
	// auto playerCountMax = jsonObject["configuration"]["player count"]["max"];
	// auto audience = jsonObject["configuration"]["audience"];
	// auto setup = jsonObject["configuration"]["setup"];
	// auto constants = jsonObject["constants"];
	// auto variables = jsonObject["variables"];
	// auto perPlayer = jsonObject["per-player"];
	// auto perAudience = jsonObject["per-audience"];
	// auto rules = jsonObject["rules"];

	// std::cout << name << std::endl;
	// std::cout << playerCountMin << std::endl;
	// std::cout << playerCountMax << std::endl;
	// std::cout << audience << std::endl;
	// std::cout << setup << std::endl;
	// std::cout << constants << std::endl;
	// std::cout << variables << std::endl;
	// std::cout << perPlayer << std::endl;
	// std::cout << perAudience << std::endl;
	// std::cout << rules << std::endl;

	Interpreter* interpreter = new Interpreter(jsonObject);
	interpreter->createGame();
	interpreter->destoryGame();

	return 0;
}

// under construction
// requires more group meetings to flesh out a more concrete structure