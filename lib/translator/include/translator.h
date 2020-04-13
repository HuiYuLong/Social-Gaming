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

class Timer
{
	std::chrono::time_point<std::chrono::steady_clock> expected_end;
public:
	Timer(int timeout);

	bool hasnt_expired() const;
};

class GameState;

// The base class for all the rules in the game configuration
// The derived rules only need to define the run method that implements all their logic
class Rule {
public:
    virtual ~Rule() {};

    virtual void run(Server& server, GameState& state) = 0;
};

class RuleList : public Rule
{
    std::vector<std::unique_ptr<Rule>> rules;

public:

    RuleList(RuleList&&) = default;

    RuleList(const nlohmann::json& json_rules);

    std::vector<std::unique_ptr<Rule>>& get();

    void run(Server& server, GameState& state) override;
};

class SetupRule : public Rule
{
    using Parameter = boost::variant<bool, int, std::string, Map>;

    std::vector<std::pair<std::string, Parameter>> parameters;

public:
    SetupRule(const nlohmann::json& setup);

    void run(Server& server, GameState& state) override;
};

// Contains the game configuration independent of a particular game instance
class Configuration {
public:
	Configuration(const nlohmann::json& config):
        name(config["configuration"]["name"]),
        player_count_min(config["configuration"]["player count"]["min"]),
        player_count_max(config["configuration"]["player count"]["max"]),
        core_variables(Map()),
        rules(config["rules"]),
        setup(config["configuration"]["setup"])
    {
        Map& map = boost::get<Map>(core_variables);        
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
    void launchGame(Server& server, GameState& state) { setup.run(server, state); rules.run(server, state); }
    //std::thread launchGameDetached(Server& server) { return rules.spawnDetached(server, *this); }

private:
	std::string name;
	size_t player_count_min;
	size_t player_count_max;
    Variable core_variables;
    Variable per_player;
    Variable per_audience;
    RuleList rules;
    SetupRule setup;
};

struct CallbackResult
{
    bool should_stop;
    bool will_be_resumed;
};

class Callback
{
public:
    // a pair indicating whether the rule should stop and whether it is going to be continued
    virtual CallbackResult check(GameState&, Rule*) = 0;
};

class RuleState
{
public:
    virtual ~RuleState() {};
};

// Each game session's private game state that holds the variable tree and the mapping of in-game names to connections
class GameState {
public:
    GameState(const Configuration& conf, const Name2Connection& name2connection, Connection game_owner)
    :   toplevel(conf.getVariables()),      // copy
        name2connection(name2connection),   // copy
        game_owner(game_owner),
        parallel_world_number(reinterpret_cast<uintptr_t>(nullptr))
    {
        Map& toplevelmap = boost::get<Map>(toplevel);
        List& players = boost::get<List>(toplevelmap["players"]);
        for ([[maybe_unused]] const auto& [name, _]: name2connection) {
            players.emplace_back(conf.getPerPlayer());
            Map& playermap = boost::get<Map>(players.back());
            playermap["name"] = name;
        }
        PrintTheThing p;
        boost::apply_visitor(p, toplevel);
    }

    Variable& getVariables() { return toplevel; }
    Connection getConnectionByName(const std::string& name) { return name2connection.at(name); }
    Connection getGameOwnerConnection() { return game_owner; }
    void registerCallback(Callback* callback) { callbacks.push_back(callback); }
    void deregisterCallback(Callback* callback) { callbacks.erase(std::remove(callbacks.begin(), callbacks.end(), callback), callbacks.end()); }
    CallbackResult checkCallbacks(Rule *rule)
    {
        for (auto it = callbacks.rbegin(); it != callbacks.rend(); ++it) {
            CallbackResult result = (*it)->check(*this, rule);
            if (result.should_stop) {
                return result;
            }
        }
        return CallbackResult{false, false};
    }
    std::unique_ptr<RuleState>& getState(Rule* rule) { return rule_states[reinterpret_cast<uintptr_t>(rule) | parallel_world_number]; }
    std::unique_ptr<RuleState>& getPureState(Rule* rule) { return rule_states[reinterpret_cast<uintptr_t>(rule)]; }
    void deleteState(Rule* rule) { rule_states.erase(reinterpret_cast<uintptr_t>(rule) | parallel_world_number); }
    void setParallelWorldNumber(uint16_t number) { uintptr_t extended = number; parallel_world_number = extended << 48; }
private:
    Variable toplevel;
    Name2Connection name2connection;
    Connection game_owner;
    std::vector<Callback*> callbacks;   // used by timers
    std::unordered_map<uintptr_t, std::unique_ptr<RuleState>> rule_states;
    uintptr_t parallel_world_number;    // almost crazy
};

struct Case
{ 
    Case(const nlohmann::json& json_case);

    Case(const nlohmann::json& json_case, const Query& value);

	Condition condition;
	RuleList subrules;
};

class Cases
{
	std::vector<Case> cases;
public:
	Cases(const nlohmann::json& json_cases);

    Cases(const nlohmann::json& json_cases, const Query& value);

	void run(Server&, GameState&);
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
        std::regex r("\\{(.+?)\\}");
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
                Getter getter(Query(value.text), toplevel);
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

class AddRule : public Rule{
private:
    Query to;
    int value;
public:
    AddRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class TimerRuleState : public RuleState
{
public:
	TimerRuleState(int duration);

	Timer timer;
};

class TimerRuleImplementation : public Rule
{
protected:
    int duration;
    RuleList subrules;
public:
    TimerRuleImplementation(int duration, const nlohmann::json& subrules);

    virtual void run(Server& server, GameState& state) = 0;
};

class AtMostTimer : public TimerRuleImplementation, public Callback
{
public:
    AtMostTimer(int duration, const nlohmann::json& subrules);

    void run(Server& server, GameState& state) override;

    CallbackResult check(GameState&, Rule*) override;
};

class ExactTimer : public TimerRuleImplementation, public Callback
{
public:
    ExactTimer(int duration, const nlohmann::json& subrules);

    void run(Server& server, GameState& state) override;

    CallbackResult check(GameState&, Rule*) override;
};

class TrackTimer : public TimerRuleImplementation, public Callback
{
    Query flag;
public:
    TrackTimer(int duration, const nlohmann::json& subrules, const std::string& flag);

    void run(Server& server, GameState& state) override;

    CallbackResult check(GameState&, Rule*) override;
};

class TimerRule : public Rule {
private:
    std::unique_ptr<TimerRuleImplementation> impl;
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
    Query to;
    Text prompt;
    boost::variant<List, Query> choices;
    Query result;
    std::optional<int> timeout;
public:
    InputChoiceRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class InputTextRule : public Rule{
private:
    Query to; 
    Text prompt;
    Query result; 
    std::optional<int> timeout;
public:
    InputTextRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;
};

class InputVoteRule : public Rule{
private:
    Query to; 
    Text prompt; 
    std::string choices;
    Query result;
public:
    InputVoteRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class MessageRule : public Rule{
private:
    Query to; 
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
    std::string score_attribute;
    bool ascending;

public:
    ScoresRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};



class ExtendRule : public Rule {
private:
    Query target;
    Query list;
public:
    ExtendRule(const nlohmann::json& rule);
    
    void run(Server& server, GameState& state) override;

};

  
class ReverseRule : public Rule{
private:
    std::string list;
public:
    ReverseRule(const nlohmann::json& rule);
    
    void run(Server& server, GameState& state) override;

};

class ShuffleRule : public Rule{
private:
    std::string list;
public:
    ShuffleRule(const nlohmann::json& rule);
    
    void run(Server& server, GameState& state) override;

};

// Sorts a list in ascending order
class SortRule : public Rule {
private:
    std::string list;
    // Variable key;
public:
    SortRule(const nlohmann::json& rule);
    
    void run(Server& server, GameState& state) override;

};

class DealRule : public Rule {
private:
    std::string from;
    std::string to;
    int count;
public:
    DealRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class DiscardRule : public Rule {
private:
    Query from;
    boost::variant<int, Query> count;
public:
    DiscardRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;
};

class ForEachRule : public Rule {
private:
    Query list;
    std::string element_name;
    RuleList subrules;

public:
    ForEachRule(const nlohmann::json& rule);
    // ~ForEachRule() override;

    void run(Server& server, GameState& state) override;

};

class LoopRule : public Rule {
private:
    bool untilLoop;
    Condition failCondition;
    RuleList subrules;
public:
    LoopRule(const nlohmann::json& rule);
    void run(Server& server, GameState& state) override;

};
  
class InParallelRule : public Rule, public Callback {
private:
    RuleList  subrules;
public:
    InParallelRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

    CallbackResult check(GameState&, Rule*) override;
};

class ParallelForRule : public Rule, public Callback {
private:
    Query list;
    std::string element_name;
    RuleList subrules;
public:
    ParallelForRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;    

    CallbackResult check(GameState&, Rule*) override;
};

// Sorts a list in ascending order
class SwitchRule : public Rule {
private:
    Cases cases;
public:
    SwitchRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;

};

class WhenRule : public Rule {
private:
    Cases cases;
public:
    WhenRule(const nlohmann::json& rule);

    void run(Server& server, GameState& state) override;
};

#endif // TRANLATOR_H


