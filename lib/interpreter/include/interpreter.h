#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>




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

class Rule {
public:
	std::string type; // "foreach", "scores" -> the field under "rule": in the json
	int list;
	std::string element;
	//Graph rules; // a graph data structure to hold subrules ... implementation to be further discussed.
};


//
class Player {
public:
	bool IsAdminPlayer;
	bool IsActive;
	std::string playerName;
	std::string Playermotto;

	//getter
	std::string getUsername()const;
	std::string getPlayeID()const;
	std::string getPassword()const;

	std::string getplayerEmail()const;
	std::string getplayerBirthday()const;
	std::string getplayerFirstDayInGame()const;
	std::string printPlayerInfo()const;

	//setter
	void setUsername(std::string username)const;
	void setPlayeID(std::string playerID)const;
	void setPassword(std::string password)const;

	void setplayerEmail(std::string playerEmail)const;
	void setplayerBirthday(std::string playerBirthday)const;
	void setplayerFirstDayInGame(std::string playerFirstDayInGame)const;

	//
	



private:


	std::string playerID;
	std::string username;  //unique
	std::string password;

	std::string playerEmail;
	std::string playerBirthday;
	std::string playerFirstDayInGame;
	nlohmann::json setup;

};