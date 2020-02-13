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

template<class Key, class Value>
class Constants {
public:
    Constants(): assignments(){}
    Constants(std::unordered_map<Key, Value> assignments){
        this->assignments = assignments;
    }
    void setWeapons(std::unordered_map<Key, Value> assignments) {
        this->assignments = assignments;
    }
    std::unordered_map<Key, Value> getAssignments() const {
        return this->assignments;
    }
    void insertToAssignments(Key& key, Value& val) {
        this->assignments.emplace(key, val);
    }

private:
    std::unordered_map<Key, Value> assignments;
};


template<class Key, class Value>
class Variables {
public:
    Variables(): variablesList(){}
    Variables(std::unordered_map<Key, Value> variablesList){
        this->variablesList = variablesList;
    }
    std::unordered_map<Key, Value> getWinners() const {
        return this->variablesList;
    }
    void insertToVariablesList(Key& key, Value& val) {
        this->variablesList.emplace(key, val);
    }

private:
    std::unordered_map<Key, Value> variablesList;
};

template<class Key, class Value>
class PerPlayer {
public:
    PerPlayer(): playerMap(){}
    PerPlayer(const std::unordered_map<Key,Value>& playerMap): playerMap(playerMap){}
    std::unordered_map<Key,Value> getPerPlayer() const {return this->playerMap;}
    void insertToPlayerMap(const Key& k, const Value& v){
        this->playerMap.emplace(k,v);
    }
private:
    std::unordered_map<Key,Value> playerMap;
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
    virtual ~Rule() {};
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
public:
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
    bool ascending;

public:
    ScoresRule(const nlohmann::json& rule);

    ruleType getScore() const {return score;}
    bool getAscending() const {return ascending;}

    void setScore(const bool& score) {this->score = score;}
    void setAscending(const bool& ascending) {this->ascending = ascending;}
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
    ruleType until;
    ruleType whileCondition;
    ruleList subrules;
public:
    LoopRule(const nlohmann::json& rule);

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
    std::vector<Case> cases;
public:
    WhenRule(const nlohmann::json& rule);

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
        {"global-message", [](const nlohmann::json& rule) { return std::make_unique<GlobalMessageRule>(rule); }},
        {"loop", [](const nlohmann::json&rule) {return std::make_unique<LoopRule>(rule);}},
        {"inparallel", [](const nlohmann::json&rule) {return std::make_unique<InParallelRule>(rule);}},
        {"parallelfor", [](const nlohmann::json&rule) {return std::make_unique<ParallelForRule>(rule);}},
        {"extend", [](const nlohmann::json& rule) {return std::make_unique<ExtendRule>(rule); }}, 
        {"reverse", [](const nlohmann::json& rule) {return std::make_unique<ReverseRule>(rule); }},
        {"discard", [](const nlohmann::json& rule) {return std::make_unique<DiscardRule>(rule); }}, 
        {"input-choice", [](const nlohmann::json& rule) {return std::make_unique<InputChoiceRule>(rule); }},
        {"add", [](const nlohmann::json& rule) {return std::make_unique<AddRule>(rule); }},
        {"scores", [](const nlohmann::json& rule) {return std::make_unique<ScoresRule>(rule); }}
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

LoopRule::LoopRule(const nlohmann::json& rule): Rule(rule["rule"]), until(rule["until"]), whileCondition("placeholder") {
    std::cout << "Loop: " << until << " or " << whileCondition << std::endl;
    for (const auto& it : rule["rules"].items()) {
        subrules.push_back(rulemap[it.value()["rule"]](it.value()));
    }
}

InParallelRule::InParallelRule(const nlohmann::json& rule): Rule(rule["rule"]) {
    std::cout << "In Parallel: " << rule["rule"] << std::endl;
    for (const auto& it : rule["rules"].items()) {
        subrules.push_back(rulemap[it.value()["rule"]](it.value()));
    }
}

ParallelForRule::ParallelForRule(const nlohmann::json& rule): Rule(rule["rule"]), list(rule["list"]), element(rule["element"]) {
    std::cout << "ParallelFor: " << rule["list"] << std::endl;
    for (const auto& it : rule["rules"].items()) {
        subrules.push_back(rulemap[it.value()["rule"]](it.value()));
    }
}

// SwitchRule and WhenRule are under construction - needs to work on cases constructor

// SwitchRule::SwitchRule(const nlohmann::json& rule): Rule(rule["rule"]), list(rule["list"]), value(rule["value"]) {

//     // needs cases constructor first

//     std::cout << "Switch For: " << rule["rule"] << std::endl;
//     for (const auto& it : rule["cases"].items()) {
//         subrules.push_back(rulemap[it.value()["rule"]](it.value()));
//     }
// }

// WhenRule::WhenRule(const nlohmann::json& rule): Rule(rule["rule"]) {

//     // needs cases constructor first

//     std::cout << "When: " << rule["rule"] << std::endl;
//     for (const auto& it : rule["cases"].items()) {
//         subrules.push_back(rulemap[it.value()["rule"]](it.value()));
//     }
// }

//TODO:Implement constructor of LoopRule,ParallelForRule,etc classes
ExtendRule::ExtendRule(const nlohmann::json& rule) : Rule(rule["rule"]), target(rule["target"]), list(rule["list"]) {
    std::cout << "Extend list: " << rule["list"] << std::endl;
}

ReverseRule::ReverseRule(const nlohmann::json& rule) : Rule(rule["rule"]), list(rule["list"]) {
    std::cout << "Reserve list: " << list << std::endl;
    for(const auto& it : rule["rules"].items()) {
        subrules.push_back(rulemap[it.value()["rule"]](it.value()));
    }
}

DiscardRule::DiscardRule(const nlohmann::json& rule) : Rule(rule["rule"]), from(rule["from"]), count(rule["count"]) {
    std::cout << "Discard count: " << count << std::endl;
}

InputChoiceRule::InputChoiceRule(const nlohmann::json& rule) : Rule(rule["rule"]), to(rule["to"]), prompt(rule["prompt"]), choices(rule["choices"]), result(rule["result"]) {
    std::cout << "Input-Choice to: " << to << std::endl;
}

AddRule::AddRule(const nlohmann::json& rule) : Rule(rule["rule"]), to(rule["to"]), value(rule["rule"]) {
    std::cout << "Add to: " << to << std::endl; 
}

ScoresRule::ScoresRule(const nlohmann::json& rule) : Rule(rule["rule"]), score(rule["score"]), ascending(rule["ascending"]) {
    std::cout << "Score: " << score << std::endl;
}


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


