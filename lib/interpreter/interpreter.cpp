#include <interpreter.h>

class Interpreter {
public:
	Configuration* configuration;
	Constants* constants;
	Variables* variables;
	PerPlayer* perPlayer;
//	PerAudience* perAudience;
	// Rule* rules;

	Interpreter(nlohmann::json gameConfig) {
		// filling out the Configuration object for the interpreter
		// example of iterating over the jsonObject
		for (auto& item : gameConfig.items()) {
			if (item.key().compare("configuration") == 0) {
				configuration = new Configuration(item.value()["name"],item.value()["player count"]["min"], item.value()["player count"]["max"], item.value()["audience"], item.value()["setup"]["Rounds"]);
				// configuration->print();
			}
		}

	}

	void createGame() { // more parameters to be added
		// std::cout << "just cheking what we will be playing: " << configuration->getName() << std::endl;
	}

	void destoryGame() {
		delete this->configuration;
	}

};


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

int main() {

	//std::istringstream iss("{\"json\": \"beta\"}"); // code manually create a small json file
	//nlohmann::json jsonObject = nlohmann::json::parse(iss);

	std::ifstream jsonFileStream("../../configs/games/rock_paper_scissors.json"); // read file
	nlohmann::json jsonObject = nlohmann::json::parse(jsonFileStream);

	// To make sure you can open the file
    if (!jsonFileStream)
    {
        std::cout << "cannot open file" << std::endl;
        return 0;
    }
	parseRule(jsonObject);

//-----------------------------------------Rule Tests--------------------------------------//
    // auto rule1 = jsonObject["rules"][0]["rule"];
    // auto rule2 = jsonObject["rules"][1]["rule"];
    // auto rule3 = jsonObject["rules"][0]["rules"];
    // auto Global_msg = jsonObject["rules"][0]["rules"][0];
    // auto Rules_choices = jsonObject["rules"][0]["rules"][1];
    // auto choices = jsonObject["rules"][0]["rules"][1]["rules"];
    // // auto inputChoice = jsonObject["rules"][0]["rules"][1]["rules"][1];
    // auto parallelFor = jsonObject["rules"][0]["rules"][1];    
    // auto inputChoice = jsonObject["rules"][0]["rules"][1]["rules"];    
    // auto Discard = jsonObject["rules"][0]["rules"][2]; 
    // auto forEach = jsonObject["rules"][0]["rules"][3];
    // // auto when = jsonObject["rules"][0]["rules"][3]["rules"];
    // auto when = jsonObject["rules"][0]["rules"][4];
    // std::cout << rule2 << std::endl;
    // cout << rule3 << endl;
    // cout << Global_msg << endl;
    // cout << inputChoice << endl;
    // cout << inputChoice << endl;
    // cout << when << endl; 
    // Rule::getScores(jsonObject);

    Add a("add", "winner.wins", "1");
    ruleList list;
    list.push_back(static_cast<Rule>(a));
    Timer t("timer","12", "exact", list);
    std::cout<< t.getSubRules().front().getRule() << "\n";

    //--------------------------------------End of rule Tests------------------------------------//

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

	// Interpreter* interpreter = new Interpreter(jsonObject);
	// interpreter->createGame();
	// interpreter->destoryGame();

	return 0;
}

// under construction
// requires more group meetings to flesh out a more concrete structure