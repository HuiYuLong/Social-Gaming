#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <deque>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <memory>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <utility>
#include <boost/algorithm/string.hpp>
#include <thread>
#include <regex>
#include "Server.h"
#include "variables.h"

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

using networking::Connection;
using networking::Name2Connection;
using networking::Server;

// For testing the interpreter
// It will be replaced by an actual server implementation in the future
// which will have the same interface
// #include <mutex>
// using networking::Message;
// using networking::Connection;
// class PseudoServer
// {
// 	static std::mutex lock;
// public:
// 	void send(Message message)
// 	{
// 		std::lock_guard lg(lock);
// 		std::cout << message.text << std::endl;
// 	}

// 	Message receive(Connection connection)
// 	{
// 		std::lock_guard lg(lock);
// 		std::string input;
// 		std::cin >> input;
// 		return Message{connection, input};
// 	}
// };

// Converts JSON objects into Variable objects
Variable buildVariables(const nlohmann::json& json);

class GameState;

// The base class for all the rules in the game configuration
// The derived rules only need to define the run method that implements all their logic
class Rule {
public:
    virtual ~Rule() {};

    virtual void run(Server& server, GameState& state) = 0;
};

using RuleList = std::vector<std::unique_ptr<Rule>>;

// A holder for all the rules in some game configuration
class RuleTree {
    RuleList rules;
public:
    RuleTree(RuleTree&&);
    RuleTree& operator=(RuleTree&&);
    RuleTree(const nlohmann::json& gameConfig);
    RuleList& getRules();

    // Just run the rules (kept for debugging purposes)
    void launchGame(Server& server, GameState& state);
};


// Contains the game configuration independent of a particular game instance
class Configuration {
public:
	Configuration(const nlohmann::json& config):
        name(config["configuration"]["name"]),
        player_count_min(config["configuration"]["player count"]["min"]),
        player_count_max(config["configuration"]["player count"]["max"]),
        core_variables(Map()),
        rules(config["rules"])
    {
        Map& map = boost::get<Map>(core_variables);
        // Put "setup" variables into "configuration" submap
        map["configuration"] = Map();
        Map& configuration = boost::get<Map>(map["configuration"]);
        for(const auto&[key, value]: config["configuration"]["setup"].items()) {
            configuration[key] = buildVariables(value);
        }
        // Put variables into the top-level map
        for(const auto&[key, value]: config["variables"].items()) {
            map[key] = buildVariables(value);
        }
        // Put constants into the top-level map
        for(const auto&[key, value]: config["constants"].items()) {
            map[key] = buildVariables(value);
        }
        // Add players
        // We should leave the map creation here for now, just populate later
        map["players"] = List();
        // List& player_list = boost::get<List>(map["players"]);
        // for(const Player& player: players) {
        //     Map player_map = boost::get<Map>(buildVariables(config["per-player"]));
        //     player_map["name"] = player.name;
        //     name2conection[player.name] = player.connection;
        //     player_list.push_back(player_map);
        // }
        per_player = buildVariables(config["per-player"]);
        // Add the audience list
        if (config["configuration"]["audience"])
        {
            map["audience"] = List();
            per_audience = buildVariables(config["per-audience"]);
        }
    }

	const std::string& getName() const { return name; }
	size_t getPlayerCountMin() const { return player_count_min; }
	size_t getPlayerCountMax() const { return player_count_max; }
    const Variable& getVariables() const { return core_variables; } 
    const Variable& getPerPlayer() const { return per_player; }
    const Variable& getPerAudience() const { return per_audience; }
    void launchGame(Server& server, GameState& state) { rules.launchGame(server, state); }
    //std::thread launchGameDetached(Server& server) { return rules.spawnDetached(server, *this); }

private:
	std::string name;
	size_t player_count_min;
	size_t player_count_max;
    Variable core_variables;
    Variable per_player;
    Variable per_audience;
    RuleTree rules;
};

class GameState {
public:
    GameState(const Configuration& conf, const Name2Connection& name2connection)
    :   toplevel(conf.getVariables()),      // copy
        name2connection(name2connection)    // copy
    {
        Map& toplevelmap = boost::get<Map>(toplevel);
        List& players = boost::get<List>(toplevelmap["players"]);
        for ([[maybe_unused]] const auto& [name, connection]: name2connection) {
            players.emplace_back(conf.getPerPlayer());
            Map& playermap = boost::get<Map>(players.back());
            playermap["name"] = name;
        }
        PrintTheThing p;
        boost::apply_visitor(p, toplevel);
    }

    Variable& getVariables() { return toplevel; }
    Connection getConnectionByName(const std::string& name) { return name2connection.at(name); }
    // getIterators() ?
private:
    Variable toplevel;
    Name2Connection name2connection;
};

// The job of the Condition class is to prepare a function that corresponds
// to a logical expression inside the game specification (like players.wins == 0)
// During the game, you just need to call the evaluate method with the current variable tree
// that automatically accesses all variables involved in the logical expression
// and returns its result as a boolean variable
class Condition 
{
    // The clause will take in a top-level variable map and return a boolean value
    // Indicating whether the variables satisfy the clause
    std::function<bool(Variable&)> clause;
    static std::regex equality_regex;
    static std::regex decimal_regex;
  
    Variable getOperand(const std::string& str)
    {
        if (std::regex_match(str, decimal_regex)) {
            return std::stoi(str);
        }
        else {
            // Assume it's a variable name
            return Query(str);
        }
    }
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
            bool negated;
            std::string condition_str = condition;
            if (condition_str.at(0) == '!') {
                negated = true;
                condition_str = condition_str.substr(1);
            }
            std::smatch match;
            if (std::regex_match(condition_str, match, equality_regex)) {
                // Interpret as a comparison of two variables
                clause = [first=getOperand(match.str(1)), second=getOperand(match.str(2)), negated] (Variable& toplevel) {
                    Equal equal(toplevel);
                    return negated ^ boost::apply_visitor(equal, first, second);
                };
            }
            else {
                // Interpret as a boolean variable
                Query query = Query(condition_str);
                clause = [query, negated] (Variable& toplevel) {
                    Getter getter(query.query, toplevel);
                    return negated ^ boost::get<bool>(getter.get().result);
                };
            }
        }
        
    }

    

    // evaluate condition
    bool evaluate(Variable& toplevel) { return clause(toplevel); }
};

struct Case
{ 
    Case(const nlohmann::json& case_);

	Condition condition;
	RuleList subrules;
};

// Builds a string with all {...} variable references replaced by their values
class Text
{
    struct Value
    {
        std::string text;
        bool needs_to_be_replaced;

        Value(const std::string text, bool replace): text(text), needs_to_be_replaced(replace) {}
    };
    std::vector<Value> values;
public:
    Text(const std::string& value)
    {
        size_t previous_match = 0u;
        std::regex r("\\{([a-z.]+)\\}");
        for(std::sregex_iterator i = std::sregex_iterator(value.begin(), value.end(), r);
            i != std::sregex_iterator();
            ++i)
        {
            std::smatch m = *i;
            values.emplace_back(value.substr(previous_match, m.position() - previous_match), false);
            values.emplace_back(m[1], true);
            previous_match = m.position() + m.length();
        }
        values.emplace_back(value.substr(previous_match), false);
    }

    std::string fill_with(Variable& toplevel)
    {
        std::ostringstream out;
        for (const Value& value : values) {
            if (value.needs_to_be_replaced) {
                Getter getter(value.text, toplevel);
                Variable& result = getter.get().result;
                out << boost::apply_visitor(StringConverter(), result);
            }
            else {
                out << value.text;
            }
        }
        out << "\n\n";
        return out.str();
    }
};

using ruleType = std::string;


class AddRule : public Rule{
private:
    std::string to;
    int value;
public:
    AddRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class TimerRule : public Rule{
private:
    int duration;
    ruleType mode;
    RuleList subrules;
public:
    TimerRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class PauseRule : public Rule {
private:
    int duration;
public:
    PauseRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;
};

class InputChoiceRule : public Rule{
private:
    ruleType to;
    Text prompt;
    ruleType choices; 
    ruleType result;
public:
    InputChoiceRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class InputTextRule : public Rule{
private:
    ruleType to; 
    Text prompt;
    ruleType result; 
public:
    InputTextRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;
};

class InputVoteRule : public Rule{
private:
    ruleType to; 
    Text prompt; 
    ruleType choices;
    ruleType result;
public:
    InputVoteRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class MessageRule : public Rule{
private:
    ruleType to; 
    Text value;

public:
    MessageRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class GlobalMessageRule : public Rule{
private:
    Text value;

public:
    GlobalMessageRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;
};

class ScoresRule: public Rule{
private:
    ruleType score;
    bool ascending;

public:
    ScoresRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};



class ExtendRule : public Rule {
private:
    ruleType target;
    ruleType list;
public:
    ExtendRule(const nlohmann::json& rule);
    
    void run(Server& server, GameState& state) override;

};

  
class ReverseRule : public Rule{
private:
    ruleType list;
public:
    ReverseRule(const nlohmann::json& rule);
    
    void run(Server& server, GameState& state) override;

};

class ShuffleRule : public Rule{
private:
    ruleType list;
public:
    ShuffleRule(const nlohmann::json& rule);
    
    void run(Server& server, GameState& state) override;

};

// Sorts a list in ascending order
class SortRule : public Rule {
private:
    ruleType list;
    // Variable key;
public:
    SortRule(const nlohmann::json& rule);
    
    void run(Server& server, GameState& state) override;

};

class DealRule : public Rule {
private:
    ruleType from;
    ruleType to;
    int count;
public:
    DealRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class DiscardRule : public Rule {
private:
    ruleType from;
    int count;
public:
    DiscardRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;
};

class ListAttributesRule : public Rule {
private:
    ruleType list;
public:
    ListAttributesRule(const nlohmann::json& rule);
    
    void run(Server& server, GameState& state) override;

};

class ForEachRule : public Rule {
private:
    ruleType list;
    ruleType element_name;
    RuleList subrules;

public:
    ForEachRule(const nlohmann::json& rule);
    // ~ForEachRule() override;

    void run(Server& server, GameState& state) override;

};

class LoopRule : public Rule {
private:
    Condition failCondition;
    RuleList subrules;
public:
    LoopRule(const nlohmann::json& rule);
    void run(Server& server, GameState& state) override;

};
  
class InParallelRule : public Rule {
private:
    RuleList subrules;
public:
    InParallelRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class ParallelForRule : public Rule {
private:
    ruleType list;
    ruleType element;
    RuleList subrules;
public:
    ParallelForRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;    

};

// Sorts a list in ascending order
class SwitchRule : public Rule {
private:
    ruleType list;
    ruleType value;
    std::vector<Case> cases;
public:
    SwitchRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class WhenRule : public Rule {
private:
    std::vector<Case> cases;
public:
    WhenRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;
};

#endif // TRANLATOR_H


