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
using type = std::string;
class Rule {
private:
    type rule;
public:
    Rule(const type& rule): rule(rule){}
    type getRule() const {return rule;}
    void setRule(const type& rule) {this->rule = rule;}
};

class Case {
	std::string caseString;
	std::vector<Rule> rules;
};

//-----------------------PETER'S CODE:---------------------------------
using ruleList = std::vector<Rule>;
class Add : public Rule{
private:
    type to;
    type value;
public:
    Add(const type& rule, const type& to, const type&value): Rule{rule}, to(to), value(value) {}

    type getTo() const {return to;}
    type getValue() const {return value;}

    void setTo(const type& to) {this->to = to;}
    void setValue(const type& value) {this->value = value;}
};

class Timer : public Rule{
private:
    type duration;
    type mode;
    ruleList subrules;
public:
    Timer(const type& rule, const type& duration, const type& mode, const ruleList& subrules): Rule{rule}, duration(duration), mode(mode), subrules(subrules) {}

    type getDuration() const {return duration;}
    type getMode() const {return mode;}
    ruleList getSubRules() const {return subrules;}

    void setDuration(const type& duration) {this->duration = duration;}
    void setMode(const type& mode) {this->mode = mode;}
    void setSubRules(const ruleList subrules) {this->subrules = subrules;}
};

class InputChoice : public Rule{
private:
    type to;//TODO --- A single player/audience member. Just leave as std::string data type for now
    type prompt;
    type choices; //TODO --- list or name of a list to choose from. Just leave as std::string data type for now
    type result; //TODO --- list of variable name in which to store the response. Just leave as std::string data type for now
public:
    InputChoice(const type& rule, const type& to, const type& choices, const type& result): Rule{rule}, to(to), choices(choices), result(result) {}

    type getTo() const {return to;}
    type getChoices() const {return choices;}
    type getPrompt () const {return prompt;}
    type getResult() const {return result;}

    void setTo(const type& to) {this->to = to;}
    void setChoices(const type& choices) {this->choices = choices;}
    void setPrompt(const type& prompt) {this->prompt = prompt;}
    void setResult(const type& result) {this->result = result;}
};

class InputText : public Rule{
private:
    type to; //TODO --- list of players. Just leave as std::string data type for now
    type prompt;
    type result; //TODO --- list of variable name in which to store the response. Just leave as std::string data type for now
public:
    InputText(const type& rule, const type& to, const type& prompt, const type& result): Rule{rule}, to(to), prompt(prompt), result(result) {}

    type getTo() const {return to;}
    type getPrompt () const {return prompt;}
    type getResult() const {return result;}

    void setTo(const type& to) {this->to = to;}
    void setPrompt(const type& prompt) {this->prompt = prompt;}
    void setResult(const type& result) {this->result = result;}
};

class InputVote : public Rule{
private:
    type to; //TODO --- list of players and/or audience members. Just leave as std::string data type for now
    type prompt; 
    type choices;
    type result;
public:
    InputVote(const type& rule, const type& to, const type& choices, const type& result): Rule{rule}, to(to), choices(choices), result(result) {}

    type getTo() const {return to;}
    type getChoices() const {return choices;}
    type getPrompt () const {return prompt;}
    type getResult() const {return result;}

    void setTo(const type& to) {this->to = to;}
    void setChoices(const type& choices) {this->choices = choices;}
    void setPrompt(const type& prompt) {this->prompt = prompt;}
    void setResult(const type& result) {this->result = result;}
};

class Message : public Rule{
private:
    type to; //TODO --- list of players. Just leave as std::string data type for now
    type value;

public:
    Message(const type& rule,const type& to, const type& value): Rule{rule}, to(to), value(value) {}

    type getTo() const {return to;}
    type getValue() const {return value;}

    void setTo(const type& to) {this->to = to;}
    void setValue(const type& value) {this->value = value;}
};

class GlobalMessage : public Rule{
private:
    type value;

public:
    GlobalMessage(const type& rule, const type& value): Rule{rule}, value(value){}

    type getValue() const {return value;}

    void setValue(const type& value) {this->value = value;}
};

class Scores: public Rule{
private:
    type score;
    type ascending;

public:
    Scores(const type& rule, const type& score, const type& ascending): Rule{rule}, score(score), ascending(ascending){}

    type getScore() const {return score;}
    type getAscending() const {return ascending;}

    void setScore(const type& score) {this->score = score;}
    void setAscending(const type& ascending) {this->ascending = ascending;}
};

//-------------------------------Sophia's Code------------------------------//
enum class ruleType { string, list, json };

class extendRule : public Rule {
private:
    ruleType target;
    ruleType list;
    std::vector<Rule> subrules;
};
  
class reverseRule : public Rule{
private:
    ruleType list;
    std::vector<Rule> subrules;
};

class shuffleRule : public Rule{
private:
    ruleType list;
    std::vector<Rule> subrules;
};

// Sorts a list in ascending order
class sortRule : public Rule {
private:
    ruleType list;
    ruleType key;
    std::vector<Rule> subrules;
};

class dealRule : public Rule {
private:
    ruleType from;
    ruleType to;
    ruleType count;
    std::vector<Rule> subrules;
};

class discard : public Rule {
private:
    ruleType from;
    ruleType count;
    std::vector<Rule> subrules;
};

class listAttributesRule : public Rule {
    ruleType roles;
};

//-------------------------------Junho's Code------------------------------//

class ForEachRule : public Rule {
private:
    ruleType list;
    ruleType element;
    std::vector<Rule> subrules;
};

class LoopRule : public Rule {
private:
    ruleType target;
    ruleType until;
    ruleType whileCondition;
    std::vector<Rule> subrules;
};
  
class InParallelRule : public Rule {
private:
    std::vector<Rule> subrules;
};

class ParallelForRule : public Rule {
private:
    ruleType list;
    ruleType element;
    std::vector<Rule> subrules;
};

// Sorts a list in ascending order
class SwitchRule : public Rule {
private:
    ruleType list;
    ruleType value;
    std::vector<Case> cases;
};

class WhenRule : public Rule {
private:
    ruleType count;
    std::vector<Case> cases;
};

//----------------------------------------End Of Rule Class---------------------------------------//

class Player {
public:
};
