#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
//#include "../../lib/interpreter/include/interpreter.h"


class Configuration {
public:
	Configuration():name(""), playerCountMin(0), playerCountMax(0), audience(false), round(0) {}
	Configuration(std::string name, int playerCountMin, int playerCountMax, bool audience, int round):
					name(name), playerCountMin(playerCountMin), playerCountMax(playerCountMax),
					audience(audience), round(round) {}

	std::string getName() const {return name;}
	int getPlayerCountMin() const {return playerCountMin;}
	int getPlayerCountMax() const {return playerCountMax;}
	bool getAudience() const {return audience;}
	int getRound() const {return round;}

	void setName(const std::string& name) {this->name = name;}
	void setPlayerCountMin(const int& playerCountMin) {this->playerCountMin = playerCountMin;}
	void setPlayerCountMax(const int& playerCountMax) {this->playerCountMax = playerCountMax;}
	void setAudience(const bool& audience) {this->audience = audience;}
	void setRound(const int& round) {this->round = round;}

	void print(){
		std::cout << "Configuration: " << "\n";
		std::cout << "\tName: " << this->getName() << "\n";
		std::cout << "\tMin Player: " << this->getPlayerCountMin() << "\n";
		std::cout << "\tMax Player: " << this->getPlayerCountMax() << "\n";
		std::cout << "\tAudience: " << this->getAudience() << "\n";
		std::cout << "\tRound: " << this->getRound() << "\n";
	}
private:
	std::string name;
	int playerCountMin;
	int playerCountMax;
	bool audience;
	int round;
};

class Constants {
public:
	Constants(): name(""), beats("") {}
	Constants(std::vector<std::pair<std::string, std::string> > weapens, std::string name, std::string beats){}
	std::string getBeats() const;
	std::string getName() const;
	std::string getWeapens() const;
	void setWeapens(const std::vector<std::pair<std::string, std::string>>& weapens);
	void setBeats(const std::string& beats);
	void setName(const std::string& name);

private:
	std::vector<std::pair<std::string, std::string> > weapens;
	std::string name;
	std::string beats;
	nlohmann::json setup;
	// Not sure since it is JSON Array string pairs Need helps for fixing 
	Constants(std::vector<std::pair<std::string, std::string> > weapens) {
		this -> weapens = weapens;
	}
};

class Variables {
public:
	Variables(): winnersName(""){}
	Variables(std::vector<std::string> winners, std::string winnersName);
	std::string getWinnersName() const;
	std::string getWinners() const;
	void setBeats(const std::string& winnersName) ;
	void setName(const std::vector<std::string>& winners) ;
private:
	std::vector<std::string> winners;
	std::string winnersName;
	nlohmann::json setup;
	// Not sure since it is JSON Array string pairs Need helps for fixing 
	Variables(std::vector<std::string> winners) {
		this -> winners = winners;
	}
};

class PerPlayer {
public:
};

// maybe we should construct separate class for the different rule classes?
// ex) OutputRule, InputRule, ArithmeticRule, ListOpsRule, ControlStructRule ...

//-------------------------------------------Rule Class---------------------------------------//
namespace Rule {

    template<typename scores> struct EnumTraits;
    enum class type {  // "foreach", "scores" -> the field under "rule": in the json
        forEach,
        globalMsg,
        parallelFor,
        inputChoice,
        Discard,
        When,
        Extend,
        add,
        scores
    };
	enum class forEach {
        list,
        element,
        rules
    };
    enum class globalMsg {
        value
    };
    enum class parallelFor {
        list,
        element,
        rules
    };
    enum class inputChoice {
        to,
        prompt,
        choices,
        result,
        timeout
    };
    enum class Discard {
        from,
        count
    };
    enum class When {
        
    };
    enum class Extend {
        target,
        list
    };
    enum class add {
        to,
        value
    };
    enum class scores {
        score, 
        ascending,
    };
    // reference: https://en.cppreference.com/w/cpp/language/enum
    //enumeration types (both scoped and unscoped) can have overloaded operators
    std::ostream& operator<<(std::ostream& os, scores c)
    {
        switch(c)
        {
            case scores::score    : os << "scores";    break;
            case scores::ascending: os << "ascending";    break;
            default    : os.setstate(std::ios_base::failbit);
        }
        return os;
    }
 
    template<> struct EnumTraits<scores> { static constexpr scores LAST = scores::ascending; };
    // auto getScores(nlohmann::json jsonObject) {    
    //         auto items = EnumTraits<scores>::LAST;
    //         std::cout << items << std::endl;
    //     // Rule::scores newRule = Rule::scores::score;
    //     // newRule = jsonObject["rules"][1]["rule"];
    // };


	//Graph rules; // a graph data structure to hold subrules ... implementation to be further discussed.
};
//----------------------------------------End Of Rule Class---------------------------------------//

class Player {
public:
};


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
//end for interpreter.h
void foreach(Interpreter *interpreter){
	auto x = interpreter->configuration->getRound();
	//std::string message = interpreter["rules"][0]["rules"][0]["value"];
	//std::cout << message << std::endl;
	//std::string messageB = boost::replace_all(message, "{round}", x);
	//message.replace(message.find('{'), message.find('{')+1, std::to_string(x));
	//std::cout << jsonObject["rules"][0]["rules"][0]["value"]<< x << std::endl;
	std::cout << x<< std::endl;

}

int main(int argc, char **argv){
    std::ifstream jsonFileStream("../../configs/games/rock_paper_scissors.json");
    nlohmann::json jsonObject = nlohmann::json::parse(jsonFileStream);

	auto name = jsonObject["configuration"]["name"];
	auto rules = jsonObject["rules"];

	std::cout << name << std::endl;

	//std::cout << " " << jsonObject["rules"][0]["rules"][2]["rule"] << std::endl;
	//print the rule "discard" in rule "foreach" 

	Interpreter* interpreter = new Interpreter(jsonObject);
	interpreter->createGame();
	//test game rules here:
	//try to call "foreach" -> "global message" -> "value"
	//
	if (jsonObject["rules"][0]["rule"] == "foreach"){
		auto x = interpreter->configuration->getRound();
		std::string message = jsonObject["rules"][0]["rules"][0]["value"];
		std::cout << message << std::endl;
		//std::string messageB = boost::replace_all(message, "{round}", x);
		message.replace(message.find('{'), message.find('{')+1, std::to_string(x));
		//std::cout << jsonObject["rules"][0]["rules"][0]["value"]<< x << std::endl;
		std::cout << message << std::endl;
	}

	foreach(interpreter);


	interpreter->destoryGame();

    nlohmann::json j;
    std::cout << "Import nlohmann succesful" << std::endl;
    return 0;
}
