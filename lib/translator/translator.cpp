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
void parseRule(const nlohmann::json& j, std::vector<nlohmann::json>& v){
	if(j.is_object()){
		for (const auto& item: j.items()){
			if (!item.key().compare("rule")){
				// std::cout << item.value() << "\n";
			} else if(!item.key().compare("rules") || item.value().is_array()){
				parseRule(item.value(),v);
			}
		}
	} else if (j.is_array()){
		for (const auto& item: j){
			parseRule(item,v);
			v.push_back(item);
		}
	}
}

RulesIdentity readRuleType(std::string s){
	if(s == "foreach"){
		return FOR_EACH;
	} else if (s == "loop"){
		return LOOP;
	} else if (s == "inparallel"){
		return IN_PARALLEL;
	} else if (s == "parallelfor"){
		return PARALLEL_FOR;
	} else if (s == "switch"){
		return SWITCH;
	} else if (s == "when"){
		return WHEN;
	} else if (s == "extend"){
		return EXTEND;
	} else if (s == "reverse"){
		return REVERSE;
	} else if (s == "shuffle"){
		return SHUFFLE;
	} else if (s == "sort"){
		return SORT;
	} else if (s == "deal"){
		return DEAL;
	} else if (s == "discard"){
		return DISCARD;
	} else if (s == "add"){
		return ADD;
	} else if (s == "timer"){
		return TIMER;
	} else if (s == "input-choice"){
		return INPUT_CHOICE;
	} else if (s == "input-text"){
		return INPUT_TEXT;
	} else if (s == "input-vote"){
		return INPUT_VOTE;
	} else if (s == "message"){
		return MESSAGE;
	} else if (s == "global-message"){
		return GLOBAL_MESSAGE;
	} else if (s == "scores"){
		return SCORES;
	}
	return INVALID;
}

void createObject(const nlohmann::json& j, ruleList& list){
	if (j.is_object()){
		RulesIdentity ruleIden;
		//Identify type of rule
		for (const auto& item: j.items()){
			if (!item.key().compare("rule")){
				ruleIden = readRuleType(item.value());
			}
		}

		//Creating object
		if (ruleIden == GLOBAL_MESSAGE){
			GlobalMessageRule temp;
			temp.setRule("global-message");
			for (const auto& item: j.items()){
				if(!item.key().compare("value")){
					temp.setValue(item.value());
				}
			}
			list.push_back(temp);
		} else if (ruleIden == SCORES){
			ScoresRule temp;
			temp.setRule("scores");
			for (const auto& item: j.items()){
				if(!item.key().compare("score")){
					temp.setScore(item.value());
				} else if (!item.key().compare("ascending")){
					temp.setAscending(item.value());
				}
			}
			list.push_back(temp);
		} else if (ruleIden == FOR_EACH){
			ForEachRule temp;
			temp.setRule("foreach");
			for(const auto& item: j.items()){
				if(!item.key().compare("list")){
					temp.setList(item.value());
				} else if (!item.key().compare("element")){
					temp.setElement(item.value());
				} else if (!item.key().compare("rules")){
					// auto size = item.value().size();
					// ruleList tempList(list.end()-size, list.end());
					// for(auto& item :tempList){
					// 	temp.insert(item);
					// }
					temp.insert(list.back());
				}
			}
			list.push_back(temp);
		} else if (ruleIden == PARALLEL_FOR){
			ParallelForRule temp;
			temp.setRule("parallelfor");
			for(const auto& item: j.items()){
				if(!item.key().compare("list")){
					temp.setList(item.value());
				} else if (!item.key().compare("element")){
					temp.setElement(item.value());
				} else if (!item.key().compare("rules")){
					// auto size = item.value().size();
					// ruleList tempList(list.end()-size, list.end());
					// for(auto& item :tempList){
					// 	temp.insert(item);
					// }
					temp.insert(list.back());
				}
			}
			list.push_back(temp);
		} else if (ruleIden == INPUT_CHOICE){
			InputChoiceRule temp;
			temp.setRule("input-choice");
			for(const auto& item: j.items()){
				if(!item.key().compare("to")){
					temp.setTo(item.value());
				} else if (!item.key().compare("prompt")){
					temp.setPrompt(item.value());
				} else if (!item.key().compare("choices")){
					temp.setChoices(item.value());
				} else if (!item.key().compare("result")){
					temp.setResult(item.value());
				} 
			}
		}
	}
	return;
}

void printRules(ruleType object){
	if (std::holds_alternative<GlobalMessageRule>(object)){
		auto temp = std::get<GlobalMessageRule>(object);
		std::cout << temp.getRule() << "\n";
		// std::cout << temp.getValue() << "\n\n";

	} else if (std::holds_alternative<ForEachRule>(object)){
		auto temp = std::get<ForEachRule>(object);
		std::cout << temp.getRule() << "\n";
		// std::cout << temp.getList() << "\n";
		// std::cout << temp.getElement() << "\n";
		ruleList subRules = temp.getSubrules();
		for(auto& item : subRules){
			std::cout <<"\t"<< "Subrules: ";
			printRules(item);
		}
	}else if (std::holds_alternative<ParallelForRule>(object)){
		auto temp = std::get<ParallelForRule>(object);
		std::cout << temp.getRule() << "\n";
		// std::cout << temp.getList() << "\n";
		// std::cout << temp.getElement() << "\n";
		ruleList subRules = temp.getSubrules();
		for(auto& item : subRules){
			std::cout <<"\t"<< "Subrules: ";
			printRules(item);
		}
	} else if (std::holds_alternative<InputChoiceRule>(object)){
		auto temp = std::get<InputChoiceRule>(object);
		std::cout << temp.getRule() << "\n";
		// std::cout << temp.getTo() << "\n";
		// std::cout << temp.getPrompt() << "\n";
		// std::cout << temp.getChoices() << "\n";
		// std::cout << temp.getResult() << "\n";

	} else if (std::holds_alternative<ScoresRule>(object)){
		auto temp = std::get<ScoresRule>(object);
		std::cout << temp.getRule() << "\n";
		// std::cout << temp.getScore() << "\n";
		// std::cout << temp.getAscending() << "\n";
	}
}
int main() {

	//std::istringstream iss("{\"json\": \"beta\"}"); // code manually create a small json file
	//nlohmann::json jsonObject = nlohmann::json::parse(iss);

    std::unique_ptr<std::vector<Rule>> ruleVector = std::make_unique<std::vector<Rule>>();
	
	std::ifstream jsonFileStream("../../configs/games/rock_paper_scissors.json"); // read file
	nlohmann::json jsonObject = nlohmann::json::parse(jsonFileStream);

	// To make sure you can open the file
    if (!jsonFileStream)
    {
        std::cout << "cannot open file" << std::endl;
        return 0;
    }
	//----------------------------TEST: Rule class--------------------------------------//
    // AddRule a("add", "winner.wins", "1");
    // ruleList list;
    // list.push_back(a);
    // // std::cout << std::get<AddRule>(list.front()).getRule() << "\n";

    // TimerRule t("timer","12", "exa ct", list);
    // std::cout << std::get<AddRule>(t.getSubRules().front()).getRule() << "\n";
    // std::cout << std::get<AddRule>(t.getSubRules().front()).getTo() << "\n";
    // std::cout << std::get<AddRule>(t.getSubRules().front()).getValue() << "\n";

	//----------------------------TEST: parseRule function---------------------------------
	std::vector<nlohmann::json> v;
	ruleList l;
	parseRule(jsonObject, v);
	for (auto item:v){
		std::cout << item << "\n";
	}
	for(auto& item: v){
		createObject(item, l);
	}

	for(auto& item: l){
		printRules(item);
	}



	// std::visit(output_visitor{}, l.front());

	// std::cout << l.size() << "\n\n";

	// std::cout << std::get<GlobalMessageRule>(l.at(0)).getRule() << "\n";
	// std::cout << std::get<GlobalMessageRule>(l.at(0)).getValue() << "\n\n";

	// std::cout << std::get<ForEachRule>(l.at(1)).getRule() << "\n";
	// std::cout << std::get<ForEachRule>(l.at(1)).getList() << "\n";
	// std::cout << std::get<ForEachRule>(l.at(1)).getElement() << "\n\n";

	// ruleList subRules = std::get<ForEachRule>(l.at(1)).getSubrules();
	// std::cout << "\t\t" << std::get<GlobalMessageRule>(subRules.front()).getRule() << "\n";
	// std::cout << "\t\t" << std::get<GlobalMessageRule>(subRules.front()).getValue() << "\n";

	// std::cout << std::get<ScoresRule>(l.at(2)).getRule() << "\n";
	// std::cout << std::get<ScoresRule>(l.at(2)).getScore() << "\n";
	// std::cout << std::get<ScoresRule>(l.at(2)).getAscending() << "\n";


	//----------------------------TEST:parseConstants function------------------------------
	// std::unique_ptr<Constants> constants;
	// constants = parseConstants(jsonObject);
	// for (auto& x : constants->getWeapons()) {
	// 	std::cout << x.first << " beats " << x.second << std::endl;
	// }
	//constants->getWeapons();
	// std::map<std::string, std::string>::iterator it;
	// for (it = constants->getWeapons().begin(); it != constants->getWeapons().end();++it) {
	// 	std::cout << it->first << " beats " << it->second << std::endl;
	// }
	
	//-------------------------------TEST:parseConfiguration function-----------------------
	// std::unique_ptr<Configuration> configuration;
	// configuration = parseConfiguration(jsonObject);
	// std::cout << "-------> Parsing configuration ------->" << std::endl;
	// std::cout << "Configuration Name: " << configuration->getName() << std::endl;

	//-------------------------------TEST:parseVariables function-----------------------
	// std::unique_ptr<Variables> variables;
	// variables = parseVariables(jsonObject);
	// std::cout << "-------> Parsing variables ------->";
	// To print



	return 0;
}

// under construction
// requires more group meetings to flesh out a more concrete structure