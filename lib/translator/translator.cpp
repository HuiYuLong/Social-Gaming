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

std::unique_ptr<Configuration> parseConfiguration(const nlohmann::json& j) {
	// Configuration* configuration;
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

	std::unique_ptr<Constants> constants;
	constants = parseConstants(jsonObject);

	for (auto& x : constants->getWeapons()) {
		std::cout << x.first << " beats " << x.second << std::endl;
	}

	//constants->getWeapons();

	// std::map<std::string, std::string>::iterator it;
	// for (it = constants->getWeapons().begin(); it != constants->getWeapons().end();++it) {
	// 	std::cout << it->first << " beats " << it->second << std::endl;
	// }
	
	//*** Parse Configuration ***//
	std::unique_ptr<Configuration> configuration;
	configuration = parseConfiguration(jsonObject);
	// std::cout << "Parsing: " << std::endl;
	// std::cout << configuration->getName() << std::endl;

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

    AddRule a("add", "winner.wins", "1");
    ruleList list;
    list.push_back(static_cast<Rule>(a));
    TimerRule t("timer","12", "exact", list);
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