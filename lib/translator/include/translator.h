#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <deque>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <memory>
#include <utility>

#include "common.h"

using networking::Message;


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
	Variables(): winners() {}
    // reconsider the type
	Variables(std::vector<std::string> winners) {}
	std::vector<std::string> getWinners() const {
        return this->winners;
    }
	void setWinners(const std::vector<std::string>& winners) {
        this->winners = winners;
    }

private:
	std::vector<std::string> winners;
};

class PerPlayer{
public:
    PerPlayer(): wins(0) {}
    int getWins() {return this->wins;}
    void setWins(int wins) {this->wins = wins;}

private:
    int wins;
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
    // virtual ~Rule() {};
};

using ruleList = std::vector<std::unique_ptr<Rule>>;

class Case { 
	std::string caseString;
	ruleList rules;
};

//-----------------------PETER'S CODE:---------------------------------

class AddRule : public Rule{
private:
    ruleType to;
    ruleType value;
public:
    AddRule(const nlohmann::json& rule);

    ruleType getTo() const {return to;}
    ruleType getValue() const {return value;}

    void setTo(const ruleType& to) {this->to = to;}
    void setValue(const ruleType& value) {this->value = value;}
};

class TimerRule : public Rule{
private:
    ruleType duration;
    ruleType mode;
    ruleList subrules;
public:
    TimerRule(const nlohmann::json& rule);

    ruleType getDuration() const {return duration;}
    ruleType getMode() const {return mode;}

    void setDuration(const ruleType& duration) {this->duration = duration;}
    void setMode(const ruleType& mode) {this->mode = mode;}

    ruleList const& getSubrules() const {return this->subrules;}
    void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
};

class InputChoiceRule : public Rule{
private:
    ruleType to;
    ruleType prompt;
    ruleType choices; 
    ruleType result; 
    InputChoiceRule(const nlohmann::json& rule);

    ruleType getTo() const {return to;}
    ruleType getChoices() const {return choices;}
    ruleType getPrompt () const {return prompt;}
    ruleType getResult() const {return result;}

    void setTo(const ruleType& to) {this->to = to;}
    void setChoices(const ruleType& choices) {this->choices = choices;}
    void setPrompt(const ruleType& prompt) {this->prompt = prompt;}
    void setResult(const ruleType& result) {this->result = result;}
};

class InputTextRule : public Rule{
private:
    ruleType to; 
    ruleType prompt;
    ruleType result; 
public:
    InputTextRule(const nlohmann::json& rule);

    ruleType getTo() const {return to;}
    ruleType getPrompt () const {return prompt;}
    ruleType getResult() const {return result;}

    void setTo(const ruleType& to) {this->to = to;}
    void setPrompt(const ruleType& prompt) {this->prompt = prompt;}
    void setResult(const ruleType& result) {this->result = result;}
};

class InputVoteRule : public Rule{
private:
    ruleType to; 
    ruleType prompt; 
    ruleType choices;
    ruleType result;
public:
    InputVoteRule(const nlohmann::json& rule);

    ruleType getTo() const {return to;}
    ruleType getChoices() const {return choices;}
    ruleType getPrompt () const {return prompt;}
    ruleType getResult() const {return result;}

    void setTo(const ruleType& to) {this->to = to;}
    void setChoices(const ruleType& choices) {this->choices = choices;}
    void setPrompt(const ruleType& prompt) {this->prompt = prompt;}
    void setResult(const ruleType& result) {this->result = result;}
};

class MessageRule : public Rule{
private:
    ruleType to; 
    ruleType value;

public:
    MessageRule(const nlohmann::json& rule);

    ruleType getTo() const {return to;}
    ruleType getValue() const {return value;}

    void setTo(const ruleType& to) {this->to = to;}
    void setValue(const ruleType& value) {this->value = value;}
};

class GlobalMessageRule : public Rule{
private:
    ruleType value;

public:
    GlobalMessageRule(const nlohmann::json& rule);
    // ~GlobalMessageRule() override;

    ruleType getValue() const {return value;}

    void setValue(const ruleType& value) {this->value = value;}
};

class ScoresRule: public Rule{
private:
    ruleType score;
    ruleType ascending;

public:
    ScoresRule(const nlohmann::json& rule);

    ruleType getScore() const {return score;}
    ruleType getAscending() const {return ascending;}

    void setScore(const ruleType& score) {this->score = score;}
    void setAscending(const ruleType& ascending) {this->ascending = ascending;}
};

//-------------------------------Sophia's Code------------------------------//

class ExtendRule : public Rule {
private:
    ruleType target;
    ruleType list;
    ruleList subrules;
public:
    ExtendRule(const nlohmann::json& rule);
    ruleType getTarget() const{return target;}
    ruleType getList() const{return list;}
    
    void setTarget(const ruleType& target){this->target=target;}
    void setList(const ruleType & list){this->list=list;}

    ruleList const& getSubrules() const {return this->subrules;}
    void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
};

  
class ReverseRule : public Rule{
private:
    ruleType list;
    ruleList subrules;
public:
    ReverseRule(const nlohmann::json& rule);
    ruleType getList() const{return list;}
    void setList(const ruleType & list){this->list=list;}

    ruleList const& getSubrules() const {return this->subrules;}
    void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
};

class ShuffleRule : public Rule{
private:
    ruleType list;
    ruleList subrules;
public:
    ShuffleRule(const nlohmann::json& rule);
    ruleType getList() const{return list;}    
    void setList(const ruleType & list){this->list=list;}

    ruleList const& getSubrules() const {return this->subrules;}
    void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
};

// Sorts a list in ascending order
class SortRule : public Rule {
private:
    ruleType list;
    ruleType key;
    ruleList subrules;
public:
    SortRule(const nlohmann::json& rule);
    ruleType getList() const{return list;}
    ruleType getKey() const{return key;}
    
    void setList(const ruleType & list){this->list=list;}
    void setKey(const ruleType & key){this->key=key;}

    ruleList const& getSubrules() const {return this->subrules;}
    void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
};

class DealRule : public Rule {
private:
    ruleType from;
    ruleType to;
    ruleType count;
    ruleList subrules;
public:
    DealRule(const nlohmann::json& rule);

    ruleType getFrom() const{return from;}
    ruleType getTo() const{return to;}
    ruleType getCount() const{return count;}
    
    void setFrom(const ruleType & from){this->from=from;}
    void setTo(const ruleType & to){this->from=from;}
    void setCount(const ruleType& count){this->count=count;}

    ruleList const& getSubrules() const {return this->subrules;}
    void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
};

class DiscardRule : public Rule {
private:
    ruleType from;
    ruleType count;
    ruleList subrules;
public:
    DiscardRule(const nlohmann::json& rule);

    ruleType getFrom() const{return from;}
    ruleType getCount() const{return count;}

    void setFrom(const ruleType & from){this->from=from;}
    void setCount(const ruleType& count){this->count=count;}

    ruleList const& getSubrules() const {return this->subrules;}
    void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
};

class ListAttributesRule : public Rule {
private:
    ruleType roles;
public:
    ListAttributesRule(const nlohmann::json& rule);
    
    ruleType getRoles() const{return roles;}

    void setRoles(const ruleType& roles) {this->roles=roles;}

};

//-------------------------------Junho's Code------------------------------//

class ForEachRule : public Rule {
private:
    ruleType list;
    ruleType element;
    ruleList subrules;

public:
    ForEachRule(const nlohmann::json& rule);
    // ~ForEachRule() override;

    ruleType getList() const {return list;}
    void setList(const ruleType& list) {this->list = list;}
    ruleType getElement() const {return element;}
    void setElement(const ruleType& element) {this->element = element;}
    ruleList const& getSubrules() const {return this->subrules;}
    void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}

};

class LoopRule : public Rule {
private:
    ruleType target;
    ruleType until;
    ruleType whileCondition;
    ruleList subrules;
public:
    LoopRule(const nlohmann::json& rule);

    ruleType getTarget() const {return this->target;}
    void setTarget(const ruleType& target) {this->target = target;}
    ruleType getUntil() const {return this->until;}
    void setUntil(const ruleType& until) {this->until = until;}
    ruleType getWhile() const {return this->whileCondition;}
    void setWhile(const ruleType& whileCondition) {this->whileCondition = whileCondition;}

    ruleList const& getSubrules() const {return this->subrules;}
    void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
};
  
class InParallelRule : public Rule {
private:
    ruleList subrules;
public:
    InParallelRule(const nlohmann::json& rule);

    ruleList const& getSubrules() const {return this->subrules;}
    void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
};

class ParallelForRule : public Rule {
private:
    ruleType list;
    ruleType element;
    ruleList subrules;
public:
    ParallelForRule(const nlohmann::json& rule);

    ruleType getList() const {return list;}
    void setList(const ruleType& list) {this->list = list;}
    ruleType getElement() const {return element;}
    void setElement(const ruleType& element) {this->element = element;}

    ruleList const& getSubrules() const {return this->subrules;}
    void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}

};

// Sorts a list in ascending order
class SwitchRule : public Rule {
private:
    ruleType list;
    ruleType value;
    std::vector<Case> cases;
public:
    SwitchRule(const nlohmann::json& rule);

    ruleType getList() const {return list;}
    void setList(const ruleType& list) {this->list = list;}
    ruleType getValue() const {return this->value;}
    void setValue(const ruleType& value) {this->value = value;}

    std::vector<Case> const& getCases() const {return this->cases;}
    void setCases(std::vector<Case> cases) {this->cases=std::move(cases);}
};

class WhenRule : public Rule {
private:
    ruleType count;
    std::vector<Case> cases;
public:
    WhenRule(const nlohmann::json& rule);

    void setCount(const ruleType& list) {this->count = count;}
    ruleType getCount() const {return count;}

    std::vector<Case> const& getCases() const {return this->cases;}
    void setCases(std::vector<Case> cases) {this->cases=std::move(cases);}
};

class RuleTree {
    ruleList ruleTree;
public:
    RuleTree(const nlohmann::json& gameConfig);

    // ~RuleTree();
};

std::unordered_map<std::string, std::function<std::unique_ptr<Rule>(const nlohmann::json&)>> rulemap = {
		{"foreach", [](const nlohmann::json& rule) { return std::make_unique<ForEachRule>(rule); }},
        {"global-message", [](const nlohmann::json& rule) { return std::make_unique<GlobalMessageRule>(rule); }}
};

//----------------------------------------Constructor implementation-------------------------------------------------------------------------------------------------
GlobalMessageRule::GlobalMessageRule(const nlohmann::json& rule): Rule{rule["rule"]}, value(rule["value"]) { std::cout << "Global message: " << value << std::endl; }

ForEachRule::ForEachRule(const nlohmann::json& rule): Rule(rule["rule"]), list(rule["list"]), element(rule["element"])
{
    std::cout << "For each: " << element << std::endl;
    for (const auto& it : rule["rules"].items())
    {
		subrules.push_back(rulemap[it.value()["rule"]](it.value()));
    }
}

//TODO:Implement constructor of LoopRule,ParallelForRule,etc classes

RuleTree::RuleTree(const nlohmann::json& gameConfig)
{
    for (const auto& it: gameConfig["rules"].items())
    {
        const nlohmann::json& rule = it.value();
        const std::string& rulename = rule["rule"];
        ruleTree.push_back(rulemap[rulename](rule));
    }
}

//-----------------------------------End of constructor implementation-----------------------------

// ForEachRule::~ForEachRule()
// {
//     for (auto ruleptr : subrules)
//         delete ruleptr;
// }

// GlobalMessageRule::~GlobalMessageRule() {}

// RuleTree::~RuleTree()
// {
//     for (auto ruleptr : ruleTree)
// 		delete ruleptr;
// }

//----------------------------------------End Of Rule Class---------------------------------------//
// Not completed
class Player {

};


