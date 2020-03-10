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

#include "common.h"
#include "variables.h"

using networking::Message;
using networking::Connection;

// For testing the interpreter
// It will be replaced by an actual server implementation in the future
// which will have the same interface
#include <mutex>
class PseudoServer
{
	static std::mutex lock;
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

std::mutex PseudoServer::lock;

// Converts JSON objects into Variable objects
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


class Configuration;

// The base class for all the rules in the game configuration
// The derived rules only need to define the run method that implements all their logic
class Rule {
public:
    virtual ~Rule() {};

    virtual void run(PseudoServer& server, Configuration& spec) = 0;
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

    // Run the rules in a separate thread
    std::thread spawnDetached(PseudoServer& server, Configuration& spec)
    {
        return std::thread([this, &server, &spec] {
            for (const auto& ptr : rules) {
                ptr->run(server, spec);
            }
        });
    }

    // Just run the rules (kept for debugging purposes)
    void spawn(PseudoServer& server, Configuration& spec)
    {
        for (const auto& ptr : rules) {
            ptr->run(server, spec);
        }
    }
};

// This class is only used in the Configuration constructor to pass the information about the players
struct Player
{
    // The in-game name of a player that can be referenced from the game spec
    std::string name;

    // A unique identifier of each player needed by the Server to send and receive messages
    Connection connection;

    Player(const std::string& name, Connection connection): name(name), connection(connection) {}
};

// Contains the game configuration
class Configuration {
public:
	Configuration(const nlohmann::json& config):
        name(config["configuration"]["name"]),
        playerCountMin(config["configuration"]["player count"]["min"]),
        playerCountMax(config["configuration"]["player count"]["max"]),
        variables(Map()),
        rules(config["rules"])
    {
        // no way to know if there are how many ppl are in at the beginning
        // if (players.size() < playerCountMin) {
        //     std::cout << "Too few players" << std::endl;
        //     std::terminate();
        // }
        // if (players.size() > playerCountMax) {
        //     std::cout << "Too many players" << std::endl;
        //     std::terminate();
        // }

        Map& map = boost::get<Map>(variables);
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
        // map["players"] = List();
        // List& player_list = boost::get<List>(map["players"]);
        // for(const Player& player: players) {
        //     Map player_map = boost::get<Map>(buildVariables(config["per-player"]));
        //     player_map["name"] = player.name;
        //     name2conection[player.name] = player.connection;
        //     player_list.push_back(player_map);
        // }
        // Add audience list
        // if (config["configuration"]["audience"])
        // {
        //     map["audience"] = List();
        // }
    }

	const std::string& getName() const { return name; }
	size_t getPlayerCountMin() const { return playerCountMin; }
	size_t getPlayerCountMax() const { return playerCountMax; }
    Variable& getVariables() { return variables; }
    Connection getConnectionByName(const std::string& name) { return name2conection.at(name); }
    void launchGame(PseudoServer& server) { rules.spawn(server, *this); }
    std::thread launchGameDetached(PseudoServer& server) { return rules.spawnDetached(server, *this); }

private:
	std::string name;
	size_t playerCountMin;
	size_t playerCountMax;
    Variable variables;
    RuleTree rules;
    using Name2Connection = std::unordered_map<std::string, Connection>;
    Name2Connection name2conection;
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

std::regex Condition::equality_regex("\\s*(\\S+)\\s*==\\s*(\\S+)\\s*");
std::regex Condition::decimal_regex("\\d+");
//std::regex Condition::variable_regex("(\\w+(\\(\\w+\\)))?\\w+(\\(\\w+\\)))?");

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

    void run(PseudoServer& server, Configuration& spec) override;

};

class TimerRule : public Rule{
private:
    int duration;
    ruleType mode;
    RuleList subrules;
public:
    TimerRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;

};

class InputChoiceRule : public Rule{
private:
    ruleType to;
    Text prompt;
    ruleType choices; 
    Text result;
public:
    InputChoiceRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;

};

class InputTextRule : public Rule{
private:
    ruleType to; 
    Text prompt;
    Text result; 
public:
    InputTextRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;
};

class InputVoteRule : public Rule{
private:
    ruleType to; 
    Text prompt; 
    ruleType choices;
    Text result;
public:
    InputVoteRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;

};

class MessageRule : public Rule{
private:
    ruleType to; 
    Text value;

public:
    MessageRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;

};

class GlobalMessageRule : public Rule{
private:
    Text value;

public:
    GlobalMessageRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;
};

class ScoresRule: public Rule{
private:
    ruleType score;
    bool ascending;

public:
    ScoresRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;

};



class ExtendRule : public Rule {
private:
    ruleType target;
    ruleType list;
public:
    ExtendRule(const nlohmann::json& rule);
    
    void run(PseudoServer& server, Configuration& spec) override;

};

  
class ReverseRule : public Rule{
private:
    ruleType list;
public:
    ReverseRule(const nlohmann::json& rule);
    
    void run(PseudoServer& server, Configuration& spec) override;

};

class ShuffleRule : public Rule{
private:
    ruleType list;
public:
    ShuffleRule(const nlohmann::json& rule);
    
    void run(PseudoServer& server, Configuration& spec) override;

};

// Sorts a list in ascending order
class SortRule : public Rule {
private:
    ruleType list;
    // Variable key;
public:
    SortRule(const nlohmann::json& rule);
    
    void run(PseudoServer& server, Configuration& spec) override;

};

class DealRule : public Rule {
private:
    ruleType from;
    ruleType to;
    int count;
public:
    DealRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;

};

class DiscardRule : public Rule {
private:
    ruleType from;
    int count;
public:
    DiscardRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;
};

class ListAttributesRule : public Rule {
private:
    ruleType list;
public:
    ListAttributesRule(const nlohmann::json& rule);
    
    void run(PseudoServer& server, Configuration& spec) override;

};

class ForEachRule : public Rule {
private:
    ruleType list;
    ruleType element_name;
    RuleList subrules;

public:
    ForEachRule(const nlohmann::json& rule);
    // ~ForEachRule() override;

    void run(PseudoServer& server, Configuration& spec) override;

};

class LoopRule : public Rule {
private:
    Condition failCondition;
    RuleList subrules;
public:
    LoopRule(const nlohmann::json& rule);
    void run(PseudoServer& server, Configuration& spec) override;

};
  
class InParallelRule : public Rule {
private:
    RuleList subrules;
public:
    InParallelRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;

};

class ParallelForRule : public Rule {
private:
    ruleType list;
    ruleType element;
    RuleList subrules;
public:
    ParallelForRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;    

};

// Sorts a list in ascending order
class SwitchRule : public Rule {
private:
    ruleType list;
    ruleType value;
    std::vector<Case> cases;
public:
    SwitchRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;

};

class WhenRule : public Rule {
private:
    std::vector<Case> cases;
public:
    WhenRule(const nlohmann::json& rule);

    void run(PseudoServer& server, Configuration& spec) override;
};

std::unordered_map<std::string, std::function<std::unique_ptr<Rule>(const nlohmann::json&)>> rulemap = {

        //Control Structures
		{"foreach", [](const nlohmann::json& rule) { return std::make_unique<ForEachRule>(rule); }},
        {"loop", [](const nlohmann::json&rule) {return std::make_unique<LoopRule>(rule);}},
        // {"inparallel", [](const nlohmann::json&rule) {return std::make_unique<InParallelRule>(rule);}},
        // {"parallelfor", [](const nlohmann::json&rule) {return std::make_unique<ParallelForRule>(rule);}},
        //{"switch", [](const nlohmann::json&rule) {return std::make_unique<SwitchRule>(rule);}},
        {"when", [](const nlohmann::json& rule) { return std::make_unique<WhenRule>(rule); }},

        //List Operations
        {"extend", [](const nlohmann::json& rule) {return std::make_unique<ExtendRule>(rule); }}, 
        {"reverse", [](const nlohmann::json& rule) {return std::make_unique<ReverseRule>(rule); }},
        {"shuffle", [](const nlohmann::json& rule) {return std::make_unique<ShuffleRule>(rule); }},
        {"sort",[](const nlohmann::json& rule) {return std::make_unique<SortRule>(rule);}},
        //{"deal",[](const nlohmann::json& rule) {return std::make_unique<DealRule>(rule);}},
        {"discard", [](const nlohmann::json& rule) {return std::make_unique<DiscardRule>(rule); }}, 
        {"deal",[](const nlohmann::json& rule) {return std::make_unique<DealRule>(rule);}},
        //{"discard", [](const nlohmann::json& rule) {return std::make_unique<DiscardRule>(rule); }}, 

        //Arithmetic Operations
        {"add", [](const nlohmann::json& rule) {return std::make_unique<AddRule>(rule); }},

        //Timing
        {"timer", [](const nlohmann::json& rule) {return std::make_unique<TimerRule>(rule); }},

        //Human Input 
        {"input-choice", [](const nlohmann::json& rule) {return std::make_unique<InputChoiceRule>(rule); }},
        {"input-text", [](const nlohmann::json& rule) {return std::make_unique<InputTextRule>(rule); }},
        {"input-vote", [](const nlohmann::json& rule) {return std::make_unique<InputVoteRule>(rule); }},

        //Output
        {"message", [](const nlohmann::json& rule) { return std::make_unique<MessageRule>(rule); }},
        {"global-message", [](const nlohmann::json& rule) { return std::make_unique<GlobalMessageRule>(rule); }},
        {"scores", [](const nlohmann::json& rule) {return std::make_unique<ScoresRule>(rule); }}
        
      
};




