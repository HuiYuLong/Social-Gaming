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

using ruleList = std::vector<std::variant<AddRule, TimerRule, InputChoiceRule,
                                          InputTextRule, InputVoteRule, MessageRule, 
                                          GlobalMessageRule, ScoresRule, ExtendRule, 
                                          ReverseRule, ShuffleRule, SortRule, 
                                          DealRule, DiscardRule, ListAttributesRule, 
                                          ForEachRule, LoopRule, InParallelRule, 
                                          ParallelForRule, SwitchRule, WhenRule>>;
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
    AddRule(const ruleType& rule, const ruleType& to, const ruleType&value): Rule{rule}, to(to), value(value) {}

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
    TimerRule(const ruleType& rule, const ruleType& duration, const ruleType& mode, const ruleList& subrules): Rule{rule}, duration(duration), mode(mode), subrules(subrules) {}

    ruleType getDuration() const {return duration;}
    ruleType getMode() const {return mode;}
    ruleList getSubRules() const {return subrules;}

    void setDuration(const ruleType& duration) {this->duration = duration;}
    void setMode(const ruleType& mode) {this->mode = mode;}
    void setSubRules(const ruleList subrules) {this->subrules = subrules;}
};

class InputChoiceRule : public Rule{
private:
    ruleType to;//TODO --- A single player/audience member. Just leave as std::string data ruleType for now
    ruleType prompt;
    ruleType choices; //TODO --- list or name of a list to choose from. Just leave as std::string data ruleType for now
    ruleType result; //TODO --- list of variable name in which to store the response. Just leave as std::string data ruleType for now
public:
    InputChoiceRule(const ruleType& rule, const ruleType& to, const ruleType& choices, const ruleType& result): Rule{rule}, to(to), choices(choices), result(result) {}

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
    ruleType to; //TODO --- list of players. Just leave as std::string data ruleType for now
    ruleType prompt;
    ruleType result; //TODO --- list of variable name in which to store the response. Just leave as std::string data ruleType for now
public:
    InputTextRule(const ruleType& rule, const ruleType& to, const ruleType& prompt, const ruleType& result): Rule{rule}, to(to), prompt(prompt), result(result) {}

    ruleType getTo() const {return to;}
    ruleType getPrompt () const {return prompt;}
    ruleType getResult() const {return result;}

    void setTo(const ruleType& to) {this->to = to;}
    void setPrompt(const ruleType& prompt) {this->prompt = prompt;}
    void setResult(const ruleType& result) {this->result = result;}
};

class InputVoteRule : public Rule{
private:
    ruleType to; //TODO --- list of players and/or audience members. Just leave as std::string data ruleType for now
    ruleType prompt; 
    ruleType choices;
    ruleType result;
public:
    InputVoteRule(const ruleType& rule, const ruleType& to, const ruleType& choices, const ruleType& result): Rule{rule}, to(to), choices(choices), result(result) {}

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
    ruleType to; //TODO --- list of players. Just leave as std::string data ruleType for now
    ruleType value;

public:
    MessageRule(const ruleType& rule,const ruleType& to, const ruleType& value): Rule{rule}, to(to), value(value) {}

    ruleType getTo() const {return to;}
    ruleType getValue() const {return value;}

    void setTo(const ruleType& to) {this->to = to;}
    void setValue(const ruleType& value) {this->value = value;}
};

class GlobalMessageRule : public Rule{
private:
    ruleType value;

public:
    GlobalMessageRule(const ruleType& rule, const ruleType& value): Rule{rule}, value(value){}

    ruleType getValue() const {return value;}

    void setValue(const ruleType& value) {this->value = value;}
};

class ScoresRule: public Rule{
private:
    ruleType score;
    ruleType ascending;

public:
    ScoresRule(const ruleType& rule, const ruleType& score, const ruleType& ascending): Rule{rule}, score(score), ascending(ascending){}

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
    ExtendRule(const ruleType& rule,const ruleType& target,const ruleType & list,const ruleList& subrules):Rule{rule},target(target),list(list),subrules(subrules){}
    ruleType getTarget() const{return target;}
    ruleType getList() const{return list;}
    ruleList getSubrules() const {return subrules;}
    
    void setTarget(const ruleType& target){this->target=target;}
    void setList(const ruleType & list){this->list=list;}
    void setSubrules(const ruleList& subrules){this->subrules=subrules;}
};

  
class ReverseRule : public Rule{
private:
    ruleType list;
    ruleList subrules;
public:
    ReverseRule(const ruleType& rule, const ruleType& list,const ruleList& subrules):Rule{rule},list(list),subrules(subrules){}
    ruleType getList() const{return list;}
    ruleList getSubrules() const {return subrules;}
    
    void setList(const ruleType & list){this->list=list;}
    void setSubrules(const ruleList& subrules){this->subrules=subrules;}
};

class ShuffleRule : public Rule{
private:
    ruleType list;
    ruleList subrules;
public:
    ShuffleRule(const ruleType& rule, const ruleType& list, const ruleList& subrules):Rule{rule},list(list),subrules(subrules){}
    ruleType getList() const{return list;}
    ruleList getSubrules() const {return subrules;}
    
    void setList(const ruleType & list){this->list=list;}
    void setSubrules(const ruleList& subrules){this->subrules=subrules;}
};

// Sorts a list in ascending order
class SortRule : public Rule {
private:
    ruleType list;
    ruleType key;
    ruleList subrules;
public:
    SortRule(const ruleType& rule, const ruleType& list, const ruleType& key,const ruleList& subrules):Rule{rule},list(list),key(key),subrules(subrules){}
    ruleType getList() const{return list;}
    ruleType getKey() const{return key;}
    ruleList getSubrules() const {return subrules;}
    
    void setList(const ruleType & list){this->list=list;}
    void setKey(const ruleType & key){this->key=key;}
    void setSubrules(const ruleList& subrules){this->subrules=subrules;}
};

class DealRule : public Rule {
private:
    ruleType from;
    ruleType to;
    ruleType count;
    ruleList subrules;
public:
    DealRule(const ruleType& rule, const ruleType& from, const ruleType& to,const ruleType& count,const ruleList& subrules):
    Rule{rule}, from(from),to(to),count(count),subrules(subrules){}

    ruleType getFrom() const{return from;}
    ruleType getTo() const{return to;}
    ruleType getCount() const{return count;}
    ruleList getSubrules() const {return subrules;}
    
    void setFrom(const ruleType & from){this->from=from;}
    void setTo(const ruleType & to){this->from=from;}
    void setCount(const ruleType& count){this->count=count;}
    void setSubrules(const ruleList& subrules){this->subrules=subrules;}

};
//todo: geter and steter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class DiscardRule : public Rule {
private:
    ruleType from;
    ruleType count;
    ruleList subrules;
public:
    DiscardRule(const ruleType& rule, const ruleType& from, const ruleType& count, const ruleList& subrules ):
    Rule{rule},from(from),count(count),subrules(subrules){}

    ruleType getFrom() const{return from;}
    ruleType getCount() const{return count;}
    ruleList getSubrules() const {return subrules;}

    void setFrom(const ruleType & from){this->from=from;}
    void setCount(const ruleType& count){this->count=count;}
    void setSubrules(const ruleList& subrules){this->subrules=subrules;}

};

class ListAttributesRule : public Rule {
private:
    ruleType roles;
public:
    ListAttributesRule(const ruleType& rule, const ruleType& roles) :
    Rule{rule},roles(roles){}
    
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
    ForEachRule(const ruleType& rule,const ruleType& list, const ruleType& element, const ruleList& subrules):
    Rule{rule},list(list),element(element),subrules(subrules){}

    ruleType getList() const {return list;}
    void setList(const ruleType& list) {this->list = list;}
    ruleType getElement() const {return element;}
    void setElement(const ruleType& element) {this->element = element;}
    ruleList getSubrules() const {return this->subrules;}
    void setSubrules(const ruleList& subrules) {this->subrules=subrules;}

};

class LoopRule : public Rule {
private:
    ruleType target;
    ruleType until;
    ruleType whileCondition;
    ruleList subrules;
public:
    LoopRule(const ruleType& rule,const ruleType& target,const ruleType& until,const ruleType& whileCondition,const ruleList& subrules):
    Rule{rule},target(target),until(until),whileCondition(whileCondition),subrules(subrules){}

    ruleType getTarget() const {return this->target;}
    void setTarget(const ruleType& target) {this->target = target;}
    ruleType getUntil() const {return this->until;}
    void setUntil(const ruleType& until) {this->until = until;}
    ruleType getWhile() const {return this->whileCondition;}
    void setWhile(const ruleType& whileCondition) {this->whileCondition = whileCondition;}
    ruleList getSubrules() const {return this->subrules;}
    void setSubrules(const ruleList& subrules) {this->subrules=subrules;}
};
  
class InParallelRule : public Rule {
private:
    ruleList subrules;
public:
    InParallelRule(const ruleType& rule,const ruleList subrules):
    Rule{rule},subrules(subrules){}

    ruleList getSubrules() const {return this->subrules;}
    void setSubrules(const ruleList& subrules) {this->subrules=subrules;}
};

class ParallelForRule : public Rule {
private:
    ruleType list;
    ruleType element;
    ruleList subrules;
public:
    ParallelForRule(const ruleType& rule,const ruleType& list,const ruleType& element,const ruleList subrules):
    Rule{rule},list(list),element(element),subrules(subrules){}

    ruleType getList() const {return list;}
    void setList(const ruleType& list) {this->list = list;}
    ruleType getElement() const {return element;}
    void setElement(const ruleType& element) {this->element = element;}
    ruleList getSubrules() const {return this->subrules;}
    void setSubrules(const ruleList& subrules) {this->subrules=subrules;}

};

// Sorts a list in ascending order
class SwitchRule : public Rule {
private:
    ruleType list;
    ruleType value;
    std::vector<Case> cases;
public:
    SwitchRule(const ruleType& rule,const ruleType& list,const ruleType& value,const  std::vector<Case> cases):
    Rule{rule},list(list),value(value),cases(cases){}

    ruleType getList() const {return list;}
    void setList(const ruleType& list) {this->list = list;}
    ruleType getValue() const {return this->value;}
    void setValue(const ruleType& value) {this->value = value;}
    std::vector<Case> getCases() const {return this->cases;}
    void setCases(const std::vector<Case>& cases) {this->cases=cases;}
};

class WhenRule : public Rule {
private:
    ruleType count;
    std::vector<Case> cases;
public:
    WhenRule(const ruleType& rule,const ruleType& count,const std::vector<Case>& cases):
    Rule{rule},count(count),cases(cases){}

    void setCount(const ruleType& list) {this->count = count;}
    ruleType getCount() const {return count;}
    std::vector<Case> getCases() const {return this->cases;}
    void setCases(const std::vector<Case>& cases) {this->cases=cases;}
};

//----------------------------------------End Of Rule Class---------------------------------------//
// Not completed
class Player {

};


