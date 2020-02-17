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
#include <boost/variant.hpp>

#include "common.h"

using networking::Message;
using networking::Connection;

// using String = std::string;
// using Integer = long;
// using Boolean = bool;
// using Key = std::string;

using Variable = boost::make_recursive_variant<
    bool,   
    int,
    std::string,
    std::vector<boost::recursive_variant_>,
    std::unordered_map<std::string, boost::recursive_variant_ >
>::type;

using List = std::vector<Variable>;
using Map = std::unordered_map<std::string, Variable>;

// Processes the query of the type 'configuration.Rounds.upfrom(1)'
// query is tokenized by the '.'
class SuperGetter : public boost::static_visitor<Variable&>
{
    thread_local static Variable returned;
    const std::vector<std::string>& query;
    size_t current_token;
public:
    SuperGetter(const std::vector<std::string>& query, size_t current_token):
        query(query), current_token(current_token) {}

    Variable& operator()(bool& boolean)
    {
        return (Variable&) boolean;
    }

    Variable& operator()(int& integer)
    {
        if (current_token == query.size())
            return (Variable&) integer;
        if (current_token == query.size() - 1)
        {
            const auto& current_query = query[current_token];
            if(current_query.compare(0, 6, "upfrom") == 0)
            {
                size_t opening_bracket = current_query.rfind('(');
                size_t closing_bracket = current_query.rfind(')');
                int from = std::stoi(current_query.substr(opening_bracket + 1, closing_bracket - opening_bracket - 1));
                returned = List();
                List& upfrom = boost::get<List>(returned);
                upfrom.reserve(integer - from + 1);
                std::iota(upfrom.begin(), upfrom.end(), from);
                return returned;
            }
            else
            {
                std::cout << "Unrecognized integer attribute" << std::endl;
                std::terminate();
            }
        }
        else
        {
            std::cout << "Ill-formed variable access" << std::endl;
            std::terminate();
        }
    }

    Variable& operator()(std::string& string)
    {
        return (Variable&) string;
    }

    Variable& operator()(List& list)
    {
        if (current_token == query.size())
            return (Variable&) list;
        const auto& current_query = query[current_token];
        if(current_query.compare(0, 4, "size") == 0)
        {
            // Ugly, but works
            // Might separate read-only and writable getters later
            returned = (int) list.size();
            return returned;
        }
        else if(current_query.compare(0, 8, "contains") == 0)
        {
            // TODO
            return (Variable&) list;
        }
        else if(current_query.compare(0, 7, "collect") == 0)
        {
            // TODO
            return (Variable&) list;
        }
        else if(current_query.compare(0, 8, "elements") == 0)
        {
            // TODO
            return (Variable&) list;
        }
        else
        {
            std::cout << "Unrecognized list attribute" << std::endl;
            std::terminate();
        }
    }

    Variable& operator()(Map& map)
    {
        return boost::apply_visitor(*this, map[query[current_token++]]);
    }
};

class PrintTheThing : public boost::static_visitor<>
{
    size_t current_offset = 0u;
public:
    //PrintTheThing(): current_offset(0u) {}

    void print_offset() const { for(size_t i = 0; i < current_offset; i++) std::cout << ' '; }

    void operator()(bool boolean) const
    {
        print_offset();
        std::cout << (boolean ? "true" : "false") << std::endl;
    }

    void operator()(int integer) const
    {
        print_offset();
        std::cout << integer << std::endl;
    }

    void operator()(const std::string& string) const
    {
        print_offset();
        std::cout << string << std::endl;
    }

    void operator()(const List& list)
    {
        current_offset++;
        for(const Variable& var : list) {
            boost::apply_visitor(*this, var);
        }
        current_offset--;
    }

    void operator()(const Map& map)
    {
        
        for(const auto&[key, var] : map) {
            print_offset();
            std::cout << key << std::endl;
            current_offset++;
            boost::apply_visitor(*this, var);
            current_offset--;
        }
    }
};

// query is a string like 'configuration.Rounds.upfrom(1)', tokenized by the '.'
// TODO: input validation is easy to add
// Variable& superGetter(Variable& variable, const std::vector<std::string>& query, size_t current_token = 0u)
// {
//     if (current_token == query.size())
//         return variable;
//     switch
// }

Variable buildVariables(const nlohmann::json& json)
{
    if (json.is_boolean()) {
        //std::cout << json << std::endl;
        return (Variable) (bool) json;
    }
    else if (json.is_number()) {
        //std::cout << json << std::endl;
        return (Variable) (int) json;
    }
    else if (json.is_string()) {
        //std::cout << json << std::endl;
        return (Variable) (std::string) json;
    }
    else if (json.is_object()) {
        Map map;
        for(const auto&[key, value]: json.items()) {
            //std::cout << key << std::endl;
            map[key] = buildVariables(value);
        }
        return (Variable) map;
    }
    else if (json.is_array()) {
        List list;
        for(const auto&[key, value]: json.items()) {
            //std::cout << key << std::endl;
            list.push_back(buildVariables(value));
        }
        return (Variable) list;
    }
    else {
        std::cout << "Invalid JSON variable type" << std::endl;
        std::terminate();
    }
}

class Configuration {
public:
	Configuration(const nlohmann::json& gamespec, const std::vector<Variable>& player_names):
        name(gamespec["configuration"]["name"]),
        playerCountMin(gamespec["configuration"]["player count"]["min"]),
        playerCountMax(gamespec["configuration"]["player count"]["max"]),
        //rules(gamespec["rules"]),
        variables(Map())
    {
        if (player_names.size() < playerCountMin) {
            std::cout << "Too few players" << std::endl;
            std::terminate();
        }
        if (player_names.size() > playerCountMax) {
            std::cout << "Too many players" << std::endl;
            std::terminate();
        }
        Map& map = boost::get<Map>(variables);
        // Put "setup" variables into "configuration" submap
        map["configuration"] = Map();
        Map& configuration = boost::get<Map>(map["configuration"]);
        for(const auto&[key, value]: gamespec["configuration"]["setup"].items()) {
            configuration[key] = buildVariables(value);
        }
        // Put variables into the top-level map
        for(const auto&[key, value]: gamespec["variables"].items()) {
            map[key] = buildVariables(value);
        }
        // Put constants into the top-level map
        for(const auto&[key, value]: gamespec["constants"].items()) {
            map[key] = buildVariables(value);
        }
        // Add players
        map["players"] = List();
        List& players = boost::get<List>(map["players"]);
        for(const Variable& name: player_names) {
            Map player = boost::get<Map>(buildVariables(gamespec["per-player"]));
            player["name"] = name;
            players.push_back(player);
        }
        // Who cares
        map["audience"] = List();
    }

	const std::string& getName() const { return name; }
	size_t getPlayerCountMin() const { return playerCountMin; }
	size_t getPlayerCountMax() const { return playerCountMax; }
    Variable& getVariables() { return variables; }

private:
	std::string name;
	size_t playerCountMin;
	size_t playerCountMax;
    Variable variables;
    //RuleTree rules;
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

