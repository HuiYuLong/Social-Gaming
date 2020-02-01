#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>




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
class Rule {
public: 
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
        ascending
    };


    // auto getScores(json jsonObject) {
    //     auto title = jsonObject["rules"][1]["rule"];
    //     scores::score newGame;
    // };

	//Graph rules; // a graph data structure to hold subrules ... implementation to be further discussed.
};
//----------------------------------------End Of Rule Class---------------------------------------//

class Player {
public:
};