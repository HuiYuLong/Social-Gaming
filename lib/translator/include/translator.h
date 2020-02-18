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
#include <boost/algorithm/string.hpp>
#include <thread>

#include "common.h"
#include "variables.h"

using networking::Message;
using networking::Connection;

// For testing the interpreter
#include <mutex>
class PseudoServer
{
	std::mutex lock;
public:
	void send(Message message)
	{
		std::lock_guard lg(lock);
		std::cout << message.text << std::endl;
	}

	Message receive(Connection connection)
	{
		std::lock_guard lg(lock);
		std::string input;
		std::cin >> input;
		return Message{connection, input};
	}
};

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


using ruleType = std::string;

class GameSpec;

class Rule {
public:
    virtual ~Rule() {};

    virtual void run(PseudoServer& server, GameSpec& spec) = 0;
};

using ruleList = std::vector<std::unique_ptr<Rule>>;

class RuleTree {
    ruleList rules;
public:
    RuleTree(RuleTree&&);
    RuleTree& operator=(RuleTree&&);
    RuleTree(const nlohmann::json& gameConfig);
    ruleList& getRules();

    // std::thread spawn(PseudoServer& server, GameSpec& spec)
    // {
    //     return std::thread([this, &server, &spec] {
    //         for (const auto& ptr : rules) {
    //             ptr->run(server, spec);
    //         }
    //     });
    // }

    void spawn(PseudoServer& server, GameSpec& spec)
    {
        for (const auto& ptr : rules) {
            ptr->run(server, spec);
        }
    }
};

struct Player
{
    std::string name;
    Connection connection;

    Player(const std::string& name, Connection connection): name(name), connection(connection) {}
};

class GameSpec {
public:
	GameSpec(const nlohmann::json& gamespec, const std::vector<Player>& players):
        name(gamespec["configuration"]["name"]),
        playerCountMin(gamespec["configuration"]["player count"]["min"]),
        playerCountMax(gamespec["configuration"]["player count"]["max"]),
        variables(Map()),
        rules(gamespec["rules"])
    {
        if (players.size() < playerCountMin) {
            std::cout << "Too few players" << std::endl;
            std::terminate();
        }
        if (players.size() > playerCountMax) {
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
        List& player_list = boost::get<List>(map["players"]);
        for(const Player& player: players) {
            Map player_map = boost::get<Map>(buildVariables(gamespec["per-player"]));
            player_map["name"] = player.name;
            playersMap[player.name] = player.connection;
            player_list.push_back(player_map);
        }
        // Who cares
        map["audience"] = List();
    }

	const std::string& getName() const { return name; }
	size_t getPlayerCountMin() const { return playerCountMin; }
	size_t getPlayerCountMax() const { return playerCountMax; }
    Variable& getVariables() { return variables; }
    Connection getConnectionByName(const std::string& name) { return playersMap[name]; }
    void launchGame(PseudoServer& server) { rules.spawn(server, *this); }

private:
	std::string name;
	size_t playerCountMin;
	size_t playerCountMax;
    Variable variables;
    RuleTree rules;
    using PlayerMap = std::unordered_map<std::string, Connection>;
    PlayerMap playersMap;
};

class Condition 
{
    // The clause will take in a top-level variable map and return a boolean value
    // Indicating whether the variables satisfy the clause
    std::function<bool(Variable&)> clause;
public:
    Condition(const nlohmann::json& condition)
    {
        if (condition.is_boolean())
        {
            if (condition)
                clause = [](Variable&) { return true; };
            else
                clause = [](Variable&) { return false; };
        }
        else
        {
            // Condition is given as string
            size_t pos;
            bool negated;
            std::string condition_str = condition;
            if (condition_str.at(0) == '!') {
                negated = true;
                condition_str = condition_str.substr(1);
            }
            if ((pos = condition_str.find("==")) != std::string::npos) {
                // Interpret as a comparison of two variables
                std::string first = (std::string) condition_str.substr(0, pos);
                boost::trim_right(first);
                std::string second = (std::string) condition_str.substr(pos+2);
                boost::trim_left(second);
                clause = [first, second, negated] (Variable& toplevel) {
                    SuperGetter getter(first);
                    Variable result1 = boost::apply_visitor(getter, toplevel);
                    getter = SuperGetter(second);
                    Variable result2 = boost::apply_visitor(getter, toplevel);
                    return negated ^ boost::apply_visitor(Equal(), result1, result2);
                };
            }
            else {
                // Interpret as a boolean variable
                clause = [condition_str, negated] (Variable& toplevel) {
                    SuperGetter getter(condition_str);
                    Variable result = boost::apply_visitor(getter, toplevel);
                    return negated ^ boost::get<bool>(result);
                };
            }
        }
        
    }

    // evaluate condition
    bool evaluate(Variable& toplevel) { return clause(toplevel); }
};

struct Case { 
    Case(const nlohmann::json& condition): condition(condition) {}

	Condition condition;
	ruleList subrules;
};


//-----------------------PETER'S CODE:---------------------------------

class AddRule : public Rule{
private:
    std::string to;
    int value;
public:
    AddRule(const nlohmann::json& rule);

    void run(PseudoServer& server, GameSpec& spec) override;

    ruleType getTo() const {return to;}
    int getValue() const {return value;}

    void setTo(const ruleType& to) {this->to = to;}
    void setValue(int value) {this->value = value;}
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

    void run(PseudoServer& server, GameSpec& spec) override;

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
    ruleType element_name;
    ruleList subrules;

public:
    ForEachRule(const nlohmann::json& rule);
    // ~ForEachRule() override;

    void run(PseudoServer& server, GameSpec& spec) override;

    ruleType getList() const {return list;}
    void setList(const ruleType& list) {this->list = list;}
    ruleType getElement() const {return element_name;}
    void setElement(const ruleType& element_name) {this->element_name = element_name;}
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

    void run(PseudoServer& server, GameSpec& spec) override;

    void setCount(const ruleType& list) {this->count = count;}
    ruleType getCount() const {return count;}

    std::vector<Case> const& getCases() const {return this->cases;}
    void setCases(std::vector<Case> cases) {this->cases=std::move(cases);}
};

std::unordered_map<std::string, std::function<std::unique_ptr<Rule>(const nlohmann::json&)>> rulemap = {
		{"foreach", [](const nlohmann::json& rule) { return std::make_unique<ForEachRule>(rule); }},
        {"global-message", [](const nlohmann::json& rule) { return std::make_unique<GlobalMessageRule>(rule); }},
        {"when", [](const nlohmann::json& rule) { return std::make_unique<WhenRule>(rule); }},
        {"add", [](const nlohmann::json& rule) {return std::make_unique<AddRule>(rule); }}
        // {"loop", [](const nlohmann::json&rule) {return std::make_unique<LoopRule>(rule);}},
        // {"inparallel", [](const nlohmann::json&rule) {return std::make_unique<InParallelRule>(rule);}},
        // {"parallelfor", [](const nlohmann::json&rule) {return std::make_unique<ParallelForRule>(rule);}},
        // {"extend", [](const nlohmann::json& rule) {return std::make_unique<ExtendRule>(rule); }}, 
        // {"reverse", [](const nlohmann::json& rule) {return std::make_unique<ReverseRule>(rule); }},
        // {"discard", [](const nlohmann::json& rule) {return std::make_unique<DiscardRule>(rule); }}, 
        // {"input-choice", [](const nlohmann::json& rule) {return std::make_unique<InputChoiceRule>(rule); }},
        // {"scores", [](const nlohmann::json& rule) {return std::make_unique<ScoresRule>(rule); }}
};




