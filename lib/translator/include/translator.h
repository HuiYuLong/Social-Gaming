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
	Constants():weapons() {}
	Constants(std::map<std::string, std::string> weapons){
        this->weapons = weapons;
    }
	void setWeapons(std::map<std::string, std::string> weapons) {
        this->weapons = weapons;
    }
    std::map<std::string, std::string> getWeapons() const {
        return this->weapons;
    }
    void insertToWeapons(std::string s1, std::string s2) {
        this->weapons.emplace(s1, s2);
    }

private:
	std::map<std::string, std::string> weapons;
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
// enum class ruleType { string, list, json };
using ruleType = std::string;
class Rule {
private:
    ruleType rule;
public:
    Rule(const ruleType& rule): rule(rule){}
    ruleType getRule() const {return rule;}
    void setRule(const ruleType& rule) {this->rule = rule;}
};

class Case {
	std::string caseString;
	std::vector<Rule> rules;
};

//-----------------------PETER'S CODE:---------------------------------

using ruleList = std::vector<Rule>;
class Add : public Rule{
private:
    ruleType to;
    ruleType value;
public:
    Add(const ruleType& rule, const ruleType& to, const ruleType&value): Rule{rule}, to(to), value(value) {}

    ruleType getTo() const {return to;}
    ruleType getValue() const {return value;}

    void setTo(const ruleType& to) {this->to = to;}
    void setValue(const ruleType& value) {this->value = value;}
};

class Timer : public Rule{
private:
    ruleType duration;
    ruleType mode;
    ruleList subrules;
public:
    Timer(const ruleType& rule, const ruleType& duration, const ruleType& mode, const ruleList& subrules): Rule{rule}, duration(duration), mode(mode), subrules(subrules) {}

    ruleType getDuration() const {return duration;}
    ruleType getMode() const {return mode;}
    ruleList getSubRules() const {return subrules;}

    void setDuration(const ruleType& duration) {this->duration = duration;}
    void setMode(const ruleType& mode) {this->mode = mode;}
    void setSubRules(const ruleList subrules) {this->subrules = subrules;}
};

class InputChoice : public Rule{
private:
    ruleType to;//TODO --- A single player/audience member. Just leave as std::string data ruleType for now
    ruleType prompt;
    ruleType choices; //TODO --- list or name of a list to choose from. Just leave as std::string data ruleType for now
    ruleType result; //TODO --- list of variable name in which to store the response. Just leave as std::string data ruleType for now
public:
    InputChoice(const ruleType& rule, const ruleType& to, const ruleType& choices, const ruleType& result): Rule{rule}, to(to), choices(choices), result(result) {}

    ruleType getTo() const {return to;}
    ruleType getChoices() const {return choices;}
    ruleType getPrompt () const {return prompt;}
    ruleType getResult() const {return result;}

    void setTo(const ruleType& to) {this->to = to;}
    void setChoices(const ruleType& choices) {this->choices = choices;}
    void setPrompt(const ruleType& prompt) {this->prompt = prompt;}
    void setResult(const ruleType& result) {this->result = result;}
};

class InputText : public Rule{
private:
    ruleType to; //TODO --- list of players. Just leave as std::string data ruleType for now
    ruleType prompt;
    ruleType result; //TODO --- list of variable name in which to store the response. Just leave as std::string data ruleType for now
public:
    InputText(const ruleType& rule, const ruleType& to, const ruleType& prompt, const ruleType& result): Rule{rule}, to(to), prompt(prompt), result(result) {}

    ruleType getTo() const {return to;}
    ruleType getPrompt () const {return prompt;}
    ruleType getResult() const {return result;}

    void setTo(const ruleType& to) {this->to = to;}
    void setPrompt(const ruleType& prompt) {this->prompt = prompt;}
    void setResult(const ruleType& result) {this->result = result;}
};

class InputVote : public Rule{
private:
    ruleType to; //TODO --- list of players and/or audience members. Just leave as std::string data ruleType for now
    ruleType prompt; 
    ruleType choices;
    ruleType result;
public:
    InputVote(const ruleType& rule, const ruleType& to, const ruleType& choices, const ruleType& result): Rule{rule}, to(to), choices(choices), result(result) {}

    ruleType getTo() const {return to;}
    ruleType getChoices() const {return choices;}
    ruleType getPrompt () const {return prompt;}
    ruleType getResult() const {return result;}

    void setTo(const ruleType& to) {this->to = to;}
    void setChoices(const ruleType& choices) {this->choices = choices;}
    void setPrompt(const ruleType& prompt) {this->prompt = prompt;}
    void setResult(const ruleType& result) {this->result = result;}
};

class Message : public Rule{
private:
    ruleType to; //TODO --- list of players. Just leave as std::string data ruleType for now
    ruleType value;

public:
    Message(const ruleType& rule,const ruleType& to, const ruleType& value): Rule{rule}, to(to), value(value) {}

    ruleType getTo() const {return to;}
    ruleType getValue() const {return value;}

    void setTo(const ruleType& to) {this->to = to;}
    void setValue(const ruleType& value) {this->value = value;}
};

class GlobalMessage : public Rule{
private:
    ruleType value;

public:
    GlobalMessage(const ruleType& rule, const ruleType& value): Rule{rule}, value(value){}

    ruleType getValue() const {return value;}

    void setValue(const ruleType& value) {this->value = value;}
};

class Scores: public Rule{
private:
    ruleType score;
    ruleType ascending;

public:
    Scores(const ruleType& rule, const ruleType& score, const ruleType& ascending): Rule{rule}, score(score), ascending(ascending){}

    ruleType getScore() const {return score;}
    ruleType getAscending() const {return ascending;}

    void setScore(const ruleType& score) {this->score = score;}
    void setAscending(const ruleType& ascending) {this->ascending = ascending;}
};

//-------------------------------Sophia's Code------------------------------//

class Extend : public Rule {
private:
    ruleType target;
    ruleType list;
    std::vector<Rule> subrules;
public:
    Extend(const ruleType& rule,const ruleType& target,const ruleType & list,const std::vector<Rule>& subrules):Rule{rule},target(target),list(list),subrules(subrules){}
    ruleType getTarget() const{return target;}
    ruleType getList() const{return list;}
    std::vector<Rule> getSubrules() const {return subrules;}
    
    void setTarget(const ruleType& target){this->target=target;}
    void setList(const ruleType & list){this->list=list;}
    void setSubrules(const std::vector<Rule>& subrules){this->subrules=subrules;}
};

  
class Reverse : public Rule{
private:
    ruleType list;
    std::vector<Rule> subrules;
public:
    Reverse(const ruleType& rule, const ruleType& list, const std::vector<Rule>& subrules):Rule{rule},list(list),subrules(subrules){}
    ruleType getList() const{return list;}
    std::vector<Rule> getSubrules() const {return subrules;}
    
    void setList(const ruleType & list){this->list=list;}
    void setSubrules(const std::vector<Rule>& subrules){this->subrules=subrules;}
};

class Shuffle : public Rule{
private:
    ruleType list;
    std::vector<Rule> subrules;
};

// Sorts a list in ascending order
class Sort : public Rule {
private:
    ruleType list;
    ruleType key;
    std::vector<Rule> subrules;
};

class Deal : public Rule {
private:
    ruleType from;
    ruleType to;
    ruleType count;
    std::vector<Rule> subrules;
};

class Discard : public Rule {
private:
    ruleType from;
    ruleType count;
    std::vector<Rule> subrules;
};

class ListAttributes : public Rule {
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
// Not completed
class Player {

};


