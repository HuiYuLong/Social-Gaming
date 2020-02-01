#include <interpreter.h>

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
		std::cout << "just checking what we will be playing: " << configuration->name << std::endl;
	}

	void destoryGame() {
		delete this->configuration;
	}

private:
};

int main() {

	//std::istringstream iss("{\"json\": \"beta\"}"); // code manually create a small json file
	//nlohmann::json jsonObject = nlohmann::json::parse(iss);

	std::cout << "Testing basic functionalities of interpreter class" << std::endl;
	std::cout << "Reading in a json file, and displaying all its fields ... " << std::endl;

	std::ifstream jsonFileStream("./rock_paper_scissors.json"); // read file
	nlohmann::json jsonObject = nlohmann::json::parse(jsonFileStream);

	//auto name = jsonObject["configuration"]["name"].get<std::string>(); // both methods work just fine
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

	std::cout << "Testing interpreter constructor ... " << std::endl;
	Interpreter* interpreter = new Interpreter(jsonObject);
	std::cout << "Testing createGame() method ... " << std::endl;
	interpreter->createGame();
	interpreter->destoryGame();

	return 0;
}

// under construction
// requires more group meetings to flesh out a more concrete structure