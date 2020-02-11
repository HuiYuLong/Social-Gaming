#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <deque>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <variant>
#include <cassert>
#include <iomanip>
#include <type_traits>
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
// enum class dataType { string, list, json };
// using dataType = std::variant<std::string, int, bool>;
using dataType = std::string;

class Rule {

private:
    dataType rule;
public:
    Rule(): rule(""){}
    Rule(const dataType& rule): rule(rule){}
    dataType getRule() const {return rule;}
    void setRule(const dataType& rule) {this->rule = rule;}
};

enum RulesIdentity {
    ADD,
    TIMER,
    INPUT_CHOICE,
    INPUT_TEXT,
    INPUT_VOTE,
    MESSAGE,
    GLOBAL_MESSAGE,
    SCORES,
    EXTEND,
    REVERSE,
    DISCARD,
    FOR_EACH,
    LOOP,
    IN_PARALLEL,
    PARALLEL_FOR,
    SWITCH,
    WHEN,
    SHUFFLE,
    SORT,
    DEAL,
    INVALID
};
//Class forward declaration
class AddRule;
class TimerRule;
class InputChoiceRule;
class InputTextRule;
class InputVoteRule;
class MessageRule;
class GlobalMessageRule;
class ScoresRule;
class ExtendRule;
class ReverseRule;
class ShuffleRule;
class SortRule;
class DealRule;
class DiscardRule;
class ListAttributesRule;
class ForEachRule;
class LoopRule;
class InParallelRule;
class ParallelForRule;
class SwitchRule;
class WhenRule;

using ruleType = std::variant<AddRule, TimerRule, InputChoiceRule,
                              InputTextRule, InputVoteRule, MessageRule, 
                              GlobalMessageRule, ScoresRule, ExtendRule, 
                              ReverseRule, ShuffleRule, SortRule, 
                              DealRule, DiscardRule, ListAttributesRule, 
                              ForEachRule, LoopRule, InParallelRule, 
                              ParallelForRule, SwitchRule, WhenRule>;
using ruleList = std::vector<ruleType>;
class Case { 
	std::string caseString;
	ruleList rules;
};

//-----------------------PETER'S CODE:---------------------------------

class AddRule : public Rule{
private:
    dataType to;
    dataType value;
public:
    AddRule(): Rule{} {}
    AddRule(const dataType& rule, const dataType& to, const dataType&value): Rule{rule}, to(to), value(value) {}

    dataType getTo() const {return to;}
    dataType getValue() const {return value;}

    void setTo(const dataType& to) {this->to = to;}
    void setValue(const dataType& value) {this->value = value;}
};

class TimerRule : public Rule{
private:
    dataType duration;
    dataType mode;
    ruleList subrules;
public:
    TimerRule(): Rule{} {}
    TimerRule(const dataType& rule, const dataType& duration, const dataType& mode, const ruleList& subrules): Rule{rule}, duration(duration), mode(mode), subrules(subrules) {}

    dataType getDuration() const {return duration;}
    dataType getMode() const {return mode;}
    ruleList getSubRules() const {return subrules;}

    void setDuration(const dataType& duration) {this->duration = duration;}
    void setMode(const dataType& mode) {this->mode = mode;}
    void setSubRules(const ruleList subrules) {this->subrules = subrules;}
};

class InputChoiceRule : public Rule{
private:
    dataType to;//TODO --- A single player/audience member. Just leave as std::string data dataType for now
    dataType prompt;
    dataType choices; //TODO --- list or name of a list to choose from. Just leave as std::string data dataType for now
    dataType result; //TODO --- list of variable name in which to store the response. Just leave as std::string data dataType for now
public:
    InputChoiceRule(): Rule{} {}
    InputChoiceRule(const dataType& rule, const dataType& to, const dataType& choices, const dataType& result): Rule{rule}, to(to), choices(choices), result(result) {}

    dataType getTo() const {return to;}
    dataType getChoices() const {return choices;}
    dataType getPrompt () const {return prompt;}
    dataType getResult() const {return result;}

    void setTo(const dataType& to) {this->to = to;}
    void setChoices(const dataType& choices) {this->choices = choices;}
    void setPrompt(const dataType& prompt) {this->prompt = prompt;}
    void setResult(const dataType& result) {this->result = result;}
};

class InputTextRule : public Rule{
private:
    dataType to; //TODO --- list of players. Just leave as std::string data dataType for now
    dataType prompt;
    dataType result; //TODO --- list of variable name in which to store the response. Just leave as std::string data dataType for now
public:
    InputTextRule(): Rule{} {}
    InputTextRule(const dataType& rule, const dataType& to, const dataType& prompt, const dataType& result): Rule{rule}, to(to), prompt(prompt), result(result) {}

    dataType getTo() const {return to;}
    dataType getPrompt () const {return prompt;}
    dataType getResult() const {return result;}

    void setTo(const dataType& to) {this->to = to;}
    void setPrompt(const dataType& prompt) {this->prompt = prompt;}
    void setResult(const dataType& result) {this->result = result;}
};

class InputVoteRule : public Rule{
private:
    dataType to; //TODO --- list of players and/or audience members. Just leave as std::string data dataType for now
    dataType prompt; 
    dataType choices;
    dataType result;
public:
    InputVoteRule(): Rule{} {}
    InputVoteRule(const dataType& rule, const dataType& to, const dataType& choices, const dataType& result): Rule{rule}, to(to), choices(choices), result(result) {}

    dataType getTo() const {return to;}
    dataType getChoices() const {return choices;}
    dataType getPrompt () const {return prompt;}
    dataType getResult() const {return result;}

    void setTo(const dataType& to) {this->to = to;}
    void setChoices(const dataType& choices) {this->choices = choices;}
    void setPrompt(const dataType& prompt) {this->prompt = prompt;}
    void setResult(const dataType& result) {this->result = result;}
};

class MessageRule : public Rule{
private:
    dataType to; //TODO --- list of players. Just leave as std::string data dataType for now
    dataType value;

public:
    MessageRule(): Rule{} {}
    MessageRule(const dataType& rule,const dataType& to, const dataType& value): Rule{rule}, to(to), value(value) {}

    dataType getTo() const {return to;}
    dataType getValue() const {return value;}

    void setTo(const dataType& to) {this->to = to;}
    void setValue(const dataType& value) {this->value = value;}
};

class GlobalMessageRule : public Rule{
private:
    dataType value;

public:
    GlobalMessageRule(): Rule{} {}

    GlobalMessageRule(const dataType& rule, const dataType& value): Rule{rule}, value(value){}

    dataType getValue() const {return value;}

    void setValue(const dataType& value) {this->value = value;}
};

class ScoresRule: public Rule{
private:
    dataType score;
    bool ascending;

public:
    ScoresRule(): Rule{} {}
    ScoresRule(const dataType& rule, const dataType& score, const bool& ascending): Rule{rule}, score(score), ascending(ascending){}

    dataType getScore() const {return score;}
    bool getAscending() const {return ascending;}

    void setScore(const dataType& score) {this->score = score;}
    void setAscending(const bool& ascending) {this->ascending = ascending;}
};

//-------------------------------Sophia's Code------------------------------//

class ExtendRule : public Rule {
private:
    dataType target;
    dataType list;
    ruleList subrules;
public:
    ExtendRule(): Rule{} {}
    ExtendRule(const dataType& rule,const dataType& target,const dataType & list,const ruleList& subrules):Rule{rule},target(target),list(list),subrules(subrules){}
    dataType getTarget() const{return target;}
    dataType getList() const{return list;}
    ruleList getSubrules() const {return subrules;}
    
    void setTarget(const dataType& target){this->target=target;}
    void setList(const dataType & list){this->list=list;}
    void setSubrules(const ruleList& subrules){this->subrules=subrules;}
};

  
class ReverseRule : public Rule{
private:
    dataType list;
    ruleList subrules;
public:
    ReverseRule(): Rule{} {}
    ReverseRule(const dataType& rule, const dataType& list,const ruleList& subrules):Rule{rule},list(list),subrules(subrules){}
    dataType getList() const{return list;}
    ruleList getSubrules() const {return subrules;}
    
    void setList(const dataType & list){this->list=list;}
    void setSubrules(const ruleList& subrules){this->subrules=subrules;}
};

class ShuffleRule : public Rule{
private:
    dataType list;
    ruleList subrules;
public:
    ShuffleRule(): Rule{} {}
    ShuffleRule(const dataType& rule, const dataType& list, const ruleList& subrules):Rule{rule},list(list),subrules(subrules){}
    dataType getList() const{return list;}
    ruleList getSubrules() const {return subrules;}
    
    void setList(const dataType & list){this->list=list;}
    void setSubrules(const ruleList& subrules){this->subrules=subrules;}
};

// Sorts a list in ascending order
class SortRule : public Rule {
private:
    dataType list;
    dataType key;
    ruleList subrules;
public:
    SortRule(): Rule{} {}
    SortRule(const dataType& rule, const dataType& list, const dataType& key,const ruleList& subrules):Rule{rule},list(list),key(key),subrules(subrules){}
    dataType getList() const{return list;}
    dataType getKey() const{return key;}
    ruleList getSubrules() const {return subrules;}
    
    void setList(const dataType & list){this->list=list;}
    void setKey(const dataType & key){this->key=key;}
    void setSubrules(const ruleList& subrules){this->subrules=subrules;}
};

class DealRule : public Rule {
private:
    dataType from;
    dataType to;
    dataType count;
    ruleList subrules;
public:
    DealRule(): Rule{} {}
    DealRule(const dataType& rule, const dataType& from, const dataType& to,const dataType& count,const ruleList& subrules):
    Rule{rule}, from(from),to(to),count(count),subrules(subrules){}

    dataType getFrom() const{return from;}
    dataType getTo() const{return to;}
    dataType getCount() const{return count;}
    ruleList getSubrules() const {return subrules;}
    
    void setFrom(const dataType & from){this->from=from;}
    void setTo(const dataType & to){this->from=from;}
    void setCount(const dataType& count){this->count=count;}
    void setSubrules(const ruleList& subrules){this->subrules=subrules;}

};
//todo: geter and steter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class DiscardRule : public Rule {
private:
    dataType from;
    dataType count;
    ruleList subrules;
public:
    DiscardRule(): Rule{} {}
    DiscardRule(const dataType& rule, const dataType& from, const dataType& count, const ruleList& subrules ):
    Rule{rule},from(from),count(count),subrules(subrules){}

    dataType getFrom() const{return from;}
    dataType getCount() const{return count;}
    ruleList getSubrules() const {return subrules;}

    void setFrom(const dataType & from){this->from=from;}
    void setCount(const dataType& count){this->count=count;}
    void setSubrules(const ruleList& subrules){this->subrules=subrules;}

};

class ListAttributesRule : public Rule {
private:
    dataType roles;
public:
    ListAttributesRule():Rule{} {}
    ListAttributesRule(const dataType& rule, const dataType& roles) :
    Rule{rule},roles(roles){}
    
    dataType getRoles() const{return roles;}

    void setRoles(const dataType& roles) {this->roles=roles;}

};

//-------------------------------Junho's Code------------------------------//

class ForEachRule : public Rule {
private:
    dataType list;
    dataType element;
    ruleList subrules;

public:
    ForEachRule(): Rule{} {}
    ForEachRule(const dataType& rule,const dataType& list, const dataType& element, const ruleList& subrules):
    Rule{rule},list(list),element(element),subrules(subrules){}

    dataType getList() const {return list;}
    void setList(const dataType& list) {this->list = list;}
    dataType getElement() const {return element;}
    void setElement(const dataType& element) {this->element = element;}
    ruleList getSubrules() const {return this->subrules;}
    void setSubrules(const ruleList& subrules) {this->subrules=subrules;}
    void insert(const ruleType& rule){ this->subrules.push_back(rule);}
};

class LoopRule : public Rule {
private:
    dataType target;
    dataType until;
    dataType whileCondition;
    ruleList subrules;
public:
    LoopRule():Rule{} {}
    LoopRule(const dataType& rule,const dataType& target,const dataType& until,const dataType& whileCondition,const ruleList& subrules):
    Rule{rule},target(target),until(until),whileCondition(whileCondition),subrules(subrules){}

    dataType getTarget() const {return this->target;}
    void setTarget(const dataType& target) {this->target = target;}
    dataType getUntil() const {return this->until;}
    void setUntil(const dataType& until) {this->until = until;}
    dataType getWhile() const {return this->whileCondition;}
    void setWhile(const dataType& whileCondition) {this->whileCondition = whileCondition;}
    ruleList getSubrules() const {return this->subrules;}
    void setSubrules(const ruleList& subrules) {this->subrules=subrules;}
};
  
class InParallelRule : public Rule {
private:
    ruleList subrules;
public:
    InParallelRule():Rule{} {}
    InParallelRule(const dataType& rule,const ruleList subrules):
    Rule{rule},subrules(subrules){}

    ruleList getSubrules() const {return this->subrules;}
    void setSubrules(const ruleList& subrules) {this->subrules=subrules;}
};

class ParallelForRule : public Rule {
private:
    dataType list;
    dataType element;
    ruleList subrules;
public:
    ParallelForRule():Rule{} {}
    ParallelForRule(const dataType& rule,const dataType& list,const dataType& element,const ruleList subrules):
    Rule{rule},list(list),element(element),subrules(subrules){}

    dataType getList() const {return list;}
    void setList(const dataType& list) {this->list = list;}
    dataType getElement() const {return element;}
    void setElement(const dataType& element) {this->element = element;}
    ruleList getSubrules() const {return this->subrules;}
    void setSubrules(const ruleList& subrules) {this->subrules=subrules;}
    void insert(const ruleType& rule){ this->subrules.push_back(rule);}

};

// Sorts a list in ascending order
class SwitchRule : public Rule {
private:
    dataType list;
    dataType value;
    std::vector<Case> cases;
public:
    SwitchRule(): Rule{} {}
    SwitchRule(const dataType& rule,const dataType& list,const dataType& value,const  std::vector<Case> cases):
    Rule{rule},list(list),value(value),cases(cases){}

    dataType getList() const {return list;}
    void setList(const dataType& list) {this->list = list;}
    dataType getValue() const {return this->value;}
    void setValue(const dataType& value) {this->value = value;}
    std::vector<Case> getCases() const {return this->cases;}
    void setCases(const std::vector<Case>& cases) {this->cases=cases;}
};

class WhenRule : public Rule {
private:
    dataType count;
    std::vector<Case> cases;
public:
    WhenRule():Rule{} {}
    WhenRule(const dataType& rule,const dataType& count,const std::vector<Case>& cases):
    Rule{rule},count(count),cases(cases){}

    void setCount(const dataType& list) {this->count = count;}
    dataType getCount() const {return count;}
    std::vector<Case> getCases() const {return this->cases;}
    void setCases(const std::vector<Case>& cases) {this->cases=cases;}
};

//----------------------------------------End Of Rule Class---------------------------------------//
// Not completed
class Player {

};


