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

using DataType = boost::variant<std::string, int, bool, unsigned>;
using networking::Message;
using networking::Connection;

// For testing the interpreter
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

using RuleList = std::vector<std::unique_ptr<Rule>>;

class RuleTree {
    RuleList rules;
public:
    RuleTree(RuleTree&&);
    RuleTree& operator=(RuleTree&&);
    RuleTree(const nlohmann::json& gameConfig);
    RuleList& getRules();

    std::thread spawn_detached(PseudoServer& server, GameSpec& spec)
    {
        return std::thread([this, &server, &spec] {
            for (const auto& ptr : rules) {
                ptr->run(server, spec);
            }
        });
    }

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
        map["audience"] = &map["players"];
    }

	const std::string& getName() const { return name; }
	size_t getPlayerCountMin() const { return playerCountMin; }
	size_t getPlayerCountMax() const { return playerCountMax; }
    Variable& getVariables() { return variables; }
    Connection getConnectionByName(const std::string& name) { return playersMap[name]; }
    void launchGame(PseudoServer& server) { rules.spawn(server, *this); }
    std::thread launchGameDetached(PseudoServer& server) { return rules.spawn_detached(server, *this); }

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
    static std::regex equality_regex;
    static std::regex decimal_regex;
    //static std::regex variable_regex;
    // struct Operand
    // {
    //     Variable variable;
    //     bool is_constant;

    //     Operand(const std::string& str)
    //     {
    //         if (std::regex_match(str, decimal_regex)) {
    //             variable = std::stoi(str);
    //             is_constant = true;
    //         }
    //         else if (str == "true") {
    //             variable = true;
    //             is_constant = true;
    //         }
    //         else if (str == "false") {
    //             variable = false;
    //             is_constant = true;
    //         }
    //         else {
    //             variable = str;
    //             is_constant = false;
    //         }
    //     }

    //     Variable get_from(Variable& toplevel) const
    //     {
    //         return is_constant ? variable : Getter(boost::get<std::string>(variable)).get_from(toplevel).result;
    //     }
    // };
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
    DataType duration;
    DataType mode;
    RuleList subrules;
public:
    TimerRule(const nlohmann::json& rule);

    DataType getDuration() const {return duration;}
    DataType getMode() const {return mode;}

    void setDuration(const DataType& duration) {this->duration = duration;}
    void setMode(const DataType& mode) {this->mode = mode;}

    RuleList const& getSubrules() const {return this->subrules;}
    void setSubrules(RuleList subrules) {this->subrules=std::move(subrules);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};

class InputChoiceRule : public Rule{
private:
    ruleType to;
    ruleType prompt;
    ruleType choices; 
    ruleType result;
public:
    InputChoiceRule(const nlohmann::json& rule);

    DataType getTo() const {return to;}
    DataType getChoices() const {return choices;}
    DataType getPrompt () const {return prompt;}
    DataType getResult() const {return result;}

    // void setTo(const DataType& to) {this->to = to;}
    // void setChoices(const DataType& choices) {this->choices = choices;}
    // void setPrompt(const DataType& prompt) {this->prompt = prompt;}
    // void setResult(const DataType& result) {this->result = result;}
    
    // std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
    //     std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
    //     return outputs;
    // }
};

class InputTextRule : public Rule{
private:
    DataType to; 
    DataType prompt;
    DataType result; 
public:
    InputTextRule(const nlohmann::json& rule);

    DataType getTo() const {return to;}
    DataType getPrompt () const {return prompt;}
    DataType getResult() const {return result;}

    void setTo(const DataType& to) {this->to = to;}
    void setPrompt(const DataType& prompt) {this->prompt = prompt;}
    void setResult(const DataType& result) {this->result = result;}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};

class InputVoteRule : public Rule{
private:
    DataType to; 
    DataType prompt; 
    DataType choices;
    DataType result;
public:
    InputVoteRule(const nlohmann::json& rule);

    DataType getTo() const {return to;}
    DataType getChoices() const {return choices;}
    DataType getPrompt () const {return prompt;}
    DataType getResult() const {return result;}

    void setTo(const DataType& to) {this->to = to;}
    void setChoices(const DataType& choices) {this->choices = choices;}
    void setPrompt(const DataType& prompt) {this->prompt = prompt;}
    void setResult(const DataType& result) {this->result = result;}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};

class MessageRule : public Rule{
private:
    DataType to; 
    DataType value;

public:
    MessageRule(const nlohmann::json& rule);

    DataType getTo() const {return to;}
    DataType getValue() const {return value;}

    void setTo(const DataType& to) {this->to = to;}
    void setValue(const DataType& value) {this->value = value;}

    // std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
	// 	std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
	// 	for (auto msg : incoming) {
	// 		Message output;
	// 		output.connection = msg.connection;
	// 		output.text = this->getValue();
	// 		std::cout << "we are running global-message: " << this->getValue() << std::endl;
	// 		outputs->push_back(output);
	// 	}
	// 	return outputs;
	// }

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }

};

class GlobalMessageRule : public Rule{
private:
    Text value;

public:
    GlobalMessageRule(const nlohmann::json& rule);

    // ~GlobalMessageRule() override;
    // std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
    // 	std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
    // 	for (auto msg : incoming) {
    // 		Message output;
    // 		output.connection = msg.connection;
    // 		output.text = boost::lexical_cast<std::string>(this->getValue());
    // 		std::cout << "we are running global-message: " << this->getValue() << std::endl;
    // 		outputs->push_back(output);
    // 	}
    // 	return outputs;
    // }

    void run(PseudoServer& server, GameSpec& spec) override;
};

class ScoresRule: public Rule{
private:
    ruleType score;
    bool ascending;

public:
    ScoresRule(const nlohmann::json& rule);

    void run(PseudoServer& server, GameSpec& spec) override;

};


//-------------------------------Sophia's Code------------------------------//

class ExtendRule : public Rule {
private:
    DataType target;
    DataType list;
    RuleList subrules;
public:
    ExtendRule(const nlohmann::json& rule);
    DataType getTarget() const{return target;}
    DataType getList() const{return list;}
    
    void setTarget(const DataType& target){this->target=target;}
    void setList(const DataType & list){this->list=list;}

    RuleList const& getSubrules() const {return this->subrules;}
    void setSubrules(RuleList subrules) {this->subrules=std::move(subrules);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};

  
class ReverseRule : public Rule{
private:
    DataType list;
    RuleList subrules;
public:
    ReverseRule(const nlohmann::json& rule);
    DataType getList() const{return list;}
    void setList(const DataType & list){this->list=list;}

    RuleList const& getSubrules() const {return this->subrules;}
    void setSubrules(RuleList subrules) {this->subrules=std::move(subrules);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};

class ShuffleRule : public Rule{
private:
    DataType list;
    RuleList subrules;
public:
    ShuffleRule(const nlohmann::json& rule);
    DataType getList() const{return list;}    
    void setList(const DataType & list){this->list=list;}

    RuleList const& getSubrules() const {return this->subrules;}
    void setSubrules(RuleList subrules) {this->subrules=std::move(subrules);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};

// Sorts a list in ascending order
class SortRule : public Rule {
private:
    DataType list;
    DataType key;
    RuleList subrules;
public:
    SortRule(const nlohmann::json& rule);
    DataType getList() const{return list;}
    DataType getKey() const{return key;}
    
    void setList(const DataType & list){this->list=list;}
    void setKey(const DataType & key){this->key=key;}

    RuleList const& getSubrules() const {return this->subrules;}
    void setSubrules(RuleList subrules) {this->subrules=std::move(subrules);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};

class DealRule : public Rule {
private:
    DataType from;
    DataType to;
    DataType count;
    RuleList subrules;
public:
    DealRule(const nlohmann::json& rule);

    DataType getFrom() const{return from;}
    DataType getTo() const{return to;}
    DataType getCount() const{return count;}
    
    void setFrom(const DataType & from){this->from=from;}
    void setTo(const DataType & to){this->to=to;}
    void setCount(const DataType& count){this->count=count;}

    RuleList const& getSubrules() const {return this->subrules;}
    void setSubrules(RuleList subrules) {this->subrules=std::move(subrules);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};

class DiscardRule : public Rule {
private:
    DataType from;
    DataType count;
    RuleList subrules;
public:
    DiscardRule(const nlohmann::json& rule);

    DataType getFrom() const{return from;}
    DataType getCount() const{return count;}

    void setFrom(const DataType & from){this->from=from;}
    void setCount(const DataType& count){this->count=count;}

    RuleList const& getSubrules() const {return this->subrules;}
    void setSubrules(RuleList subrules) {this->subrules=std::move(subrules);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};

class ListAttributesRule : public Rule {
private:
    DataType roles;
public:
    ListAttributesRule(const nlohmann::json& rule);
    
    DataType getRoles() const{return roles;}

    void setRoles(const DataType& roles) {this->roles=roles;}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }

};

//-------------------------------Junho's Code------------------------------//

class ForEachRule : public Rule {
private:
    ruleType list;
    ruleType element_name;
    RuleList subrules;

public:
    ForEachRule(const nlohmann::json& rule);
    // ~ForEachRule() override;

    void run(PseudoServer& server, GameSpec& spec) override;

    ruleType getList() const {return list;}
    void setList(const ruleType& list) {this->list = list;}
    ruleType getElement() const {return element_name;}
    void setElement(const ruleType& element_name) {this->element_name = element_name;}
    RuleList const& getSubrules() const {return this->subrules;}
    void setSubrules(RuleList subrules) {this->subrules=std::move(subrules);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }

};

class LoopRule : public Rule {
private:
    DataType until;
    DataType whileCondition;
    RuleList subrules;
public:
    LoopRule(const nlohmann::json& rule);

    DataType getUntil() const {return this->until;}
    void setUntil(const DataType& until) {this->until = until;}
    DataType getWhile() const {return this->whileCondition;}
    void setWhile(const DataType& whileCondition) {this->whileCondition = whileCondition;}

    RuleList const& getSubrules() const {return this->subrules;}
    void setSubrules(RuleList subrules) {this->subrules=std::move(subrules);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};
  
class InParallelRule : public Rule {
private:
    RuleList subrules;
public:
    InParallelRule(const nlohmann::json& rule);

    RuleList const& getSubrules() const {return this->subrules;}
    void setSubrules(RuleList subrules) {this->subrules=std::move(subrules);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};

class ParallelForRule : public Rule {
private:
    DataType list;
    DataType element;
    RuleList subrules;
public:
    ParallelForRule(const nlohmann::json& rule);

    DataType getList() const {return list;}
    void setList(const DataType& list) {this->list = list;}
    DataType getElement() const {return element;}
    void setElement(const DataType& element) {this->element = element;}

    RuleList const& getSubrules() const {return this->subrules;}
    void setSubrules(RuleList subrules) {this->subrules=std::move(subrules);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }

};

// Sorts a list in ascending order
class SwitchRule : public Rule {
private:
    DataType list;
    DataType value;
    std::vector<Case> cases;
public:
    SwitchRule(const nlohmann::json& rule);

    DataType getList() const {return list;}
    void setList(const DataType& list) {this->list = list;}
    DataType getValue() const {return this->value;}
    void setValue(const DataType& value) {this->value = value;}

    std::vector<Case> const& getCases() const {return this->cases;}
    void setCases(std::vector<Case> cases) {this->cases=std::move(cases);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};

class WhenRule : public Rule {
private:
    std::vector<Case> cases;
public:
    WhenRule(const nlohmann::json& rule);

    void run(PseudoServer& server, GameSpec& spec) override;

    std::vector<Case> const& getCases() const {return this->cases;}
    void setCases(std::vector<Case> cases) {this->cases=std::move(cases);}

    std::unique_ptr<std::deque<Message>> run(const std::deque<Message>& incoming) {
        std::unique_ptr<std::deque<Message>> outputs = std::make_unique<std::deque<Message>>(); 
        return outputs;
    }
};

std::unordered_map<std::string, std::function<std::unique_ptr<Rule>(const nlohmann::json&)>> rulemap = {
		{"foreach", [](const nlohmann::json& rule) { return std::make_unique<ForEachRule>(rule); }},
        {"global-message", [](const nlohmann::json& rule) { return std::make_unique<GlobalMessageRule>(rule); }},
        {"when", [](const nlohmann::json& rule) { return std::make_unique<WhenRule>(rule); }},
        {"add", [](const nlohmann::json& rule) {return std::make_unique<AddRule>(rule); }},
        // {"loop", [](const nlohmann::json&rule) {return std::make_unique<LoopRule>(rule);}},
        // {"inparallel", [](const nlohmann::json&rule) {return std::make_unique<InParallelRule>(rule);}},
        // {"parallelfor", [](const nlohmann::json&rule) {return std::make_unique<ParallelForRule>(rule);}},
        // {"extend", [](const nlohmann::json& rule) {return std::make_unique<ExtendRule>(rule); }}, 
        // {"reverse", [](const nlohmann::json& rule) {return std::make_unique<ReverseRule>(rule); }},
        // {"discard", [](const nlohmann::json& rule) {return std::make_unique<DiscardRule>(rule); }}, 
        // {"input-choice", [](const nlohmann::json& rule) {return std::make_unique<InputChoiceRule>(rule); }},
         {"scores", [](const nlohmann::json& rule) {return std::make_unique<ScoresRule>(rule); }}
};




