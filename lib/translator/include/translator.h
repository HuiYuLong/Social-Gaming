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
#include <utility>

#include "common.h"
using namespace std;

using DataType = boost::variant<std::string, int, bool, unsigned>;

using networking::Message;

using json = nlohmann::json;
using Variable = boost::make_recursive_variant<
    bool,
    int,
    std::string,
    std::vector<boost::recursive_variant_>,
    std::unordered_map<std::string, boost::recursive_variant_ >
    >::type;

void definingDataType( const nlohmann::basic_json<> &item, DataType& value);






//-------------------------------------------Configuration class ---------------------------------------//
std::unordered_map<std::string, std::function<Variable>(json&)>configurationMap = {
         {"configuration"}, [](json& config) {return std::make_unique<Configuration>(config);},
        // {"name",[](const nlohmann::json& config){
        //     std::string value = config["configuration"]["name"];
        //     return value;
        // }}
        // {"player count"}, [](const nlohmann::json& config){config["player count"]["min"];},
        // {"min"}, [](const nlohmann::json& config){return config["min"];},
        // {"max"}, [](const nlohmann::json& config){return config["max"];}
        // {"setup"}, [](const nlohmann::json& config){return map???}
};
class Top_levelMap {
public:
    Top_levelMap(const nlohmann::json& j){
        this->setVariables(j);
    }

    void setVariables(const nlohmann::json& j){
        // for (const auto& it: j["Configuration"].items()){
        //     const nlohmann::json& name = it.value();
        //     const std::string& temp = j["name"];

        // }

    //    this->variables.insert(configurationMap["name"](j));
    //     for(auto& item:this->variables){
    //         std::cout << item.first << " " << item.second;
    //     }

        cout<<"hello  i am back\n";
    }
    

private:
    std::unordered_map<std::string, DataType > variables;  //predeterminded variable in congiguration 
    DataType audience;  //map of each audience' name and state
    DataType players;  //map of each players' name and state
};
			




//-------------------------------------------Rule Class---------------------------------------//


// class Rule {

// private:
//     std::string rule;
// public:
    
//     Rule(const std::string& rule): rule(rule){}
//     std::string getRule() const {return rule;}
//     void setRule(const std::string& rule) {this->rule = rule;}
//     virtual ~Rule() {};
// };

// using ruleList = std::vector<std::unique_ptr<Rule>>;

// class Integer
// {
//     public:
//         list upfrom(int a); // add a from current
//     private:
//         int a;
// };

// // class Value
// // {
// //     public:
// //         Value(const std::string& condition ){ }
    
// //     operator const std::string (){return "something"};
// // };

// class List{
//     public:
//     std::vector<Variable> list;
//     List collect();

//     bool contains();
// };

// class Condition{
//     public:
//         Condition(const std::string & condition){}

// };

// class Case { 
// 	std::string caseString;
// 	ruleList rules;
// };


// //-----------------------PETER'S CODE:---------------------------------

// class AddRule : public Rule{
// private:
//     DataType to;
//     DataType value;
// public:
//     AddRule(const nlohmann::json& rule);

//     DataType getTo() const {return to;}
//     DataType getValue() const {return value;}

//     void setTo(const DataType& to) {this->to = to;}
//     void setValue(const DataType& value) {this->value = value;}
// };

// class TimerRule : public Rule{
// private:
//     DataType duration;
//     DataType mode;
//     ruleList subrules;
// public:
//     TimerRule(const nlohmann::json& rule);

//     DataType getDuration() const {return duration;}
//     DataType getMode() const {return mode;}

//     void setDuration(const DataType& duration) {this->duration = duration;}
//     void setMode(const DataType& mode) {this->mode = mode;}

//     ruleList const& getSubrules() const {return this->subrules;}
//     void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
// };

// class InputChoiceRule : public Rule{
// private:
//     DataType to;
//     DataType prompt;
//     DataType choices; 
//     DataType result; 
// public:
//     InputChoiceRule(const nlohmann::json& rule);

//     DataType getTo() const {return to;}
//     DataType getChoices() const {return choices;}
//     DataType getPrompt () const {return prompt;}
//     DataType getResult() const {return result;}

//     void setTo(const DataType& to) {this->to = to;}
//     void setChoices(const DataType& choices) {this->choices = choices;}
//     void setPrompt(const DataType& prompt) {this->prompt = prompt;}
//     void setResult(const DataType& result) {this->result = result;}
// };

// class InputTextRule : public Rule{
// private:
//     DataType to; 
//     DataType prompt;
//     DataType result; 
// public:
//     InputTextRule(const nlohmann::json& rule);

//     DataType getTo() const {return to;}
//     DataType getPrompt () const {return prompt;}
//     DataType getResult() const {return result;}

//     void setTo(const DataType& to) {this->to = to;}
//     void setPrompt(const DataType& prompt) {this->prompt = prompt;}
//     void setResult(const DataType& result) {this->result = result;}
// };

// class InputVoteRule : public Rule{
// private:
//     DataType to; 
//     DataType prompt; 
//     DataType choices;
//     DataType result;
// public:
//     InputVoteRule(const nlohmann::json& rule);

//     DataType getTo() const {return to;}
//     DataType getChoices() const {return choices;}
//     DataType getPrompt () const {return prompt;}
//     DataType getResult() const {return result;}

//     void setTo(const DataType& to) {this->to = to;}
//     void setChoices(const DataType& choices) {this->choices = choices;}
//     void setPrompt(const DataType& prompt) {this->prompt = prompt;}
//     void setResult(const DataType& result) {this->result = result;}
// };

// class MessageRule : public Rule{
// private:
//     DataType to; 
//     DataType value;

// public:
//     MessageRule(const nlohmann::json& rule);

//     DataType getTo() const {return to;}
//     DataType getValue() const {return value;}

//     void setTo(const DataType& to) {this->to = to;}
//     void setValue(const DataType& value) {this->value = value;}
// };

// class GlobalMessageRule : public Rule{
// private:
//     DataType value;

// public:
//     GlobalMessageRule(const nlohmann::json& rule);
//     // ~GlobalMessageRule() override;

//     DataType getValue() const {return value;}

//     void setValue(const DataType& value) {this->value = value;}
// };

// class ScoresRule: public Rule{
// private:
//     DataType score;

//     DataType ascending;

// public:
//     ScoresRule(const nlohmann::json& rule);

//     DataType getScore() const {return score;}
//     DataType getAscending() const {return ascending;}

//     void setScore(const bool& score) {this->score = score;}
//     void setAscending(const bool& ascending) {this->ascending = ascending;}
// };

// //-------------------------------Sophia's Code------------------------------//

// class ExtendRule : public Rule {
// private:
//     DataType target;
//     DataType list;
//     ruleList subrules;
// public:
//     ExtendRule(const nlohmann::json& rule);
//     DataType getTarget() const{return target;}
//     DataType getList() const{return list;}
    
//     void setTarget(const DataType& target){this->target=target;}
//     void setList(const DataType & list){this->list=list;}

//     ruleList const& getSubrules() const {return this->subrules;}
//     void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
// };

  
// class ReverseRule : public Rule{
// private:
//     DataType list;
//     ruleList subrules;
// public:
//     ReverseRule(const nlohmann::json& rule);
//     DataType getList() const{return list;}
//     void setList(const DataType & list){this->list=list;}

//     ruleList const& getSubrules() const {return this->subrules;}
//     void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
// };

// class ShuffleRule : public Rule{
// private:
//     DataType list;
//     ruleList subrules;
// public:
//     ShuffleRule(const nlohmann::json& rule);
//     DataType getList() const{return list;}    
//     void setList(const DataType & list){this->list=list;}

//     ruleList const& getSubrules() const {return this->subrules;}
//     void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
// };

// // Sorts a list in ascending order
// class SortRule : public Rule {
// private:
//     DataType list;
//     DataType key;
//     ruleList subrules;
// public:
//     SortRule(const nlohmann::json& rule);
//     DataType getList() const{return list;}
//     DataType getKey() const{return key;}
    
//     void setList(const DataType & list){this->list=list;}
//     void setKey(const DataType & key){this->key=key;}

//     ruleList const& getSubrules() const {return this->subrules;}
//     void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
// };

// class DealRule : public Rule {
// private:
//     DataType from;
//     DataType to;
//     DataType count;
//     ruleList subrules;
// public:
//     DealRule(const nlohmann::json& rule);

//     DataType getFrom() const{return from;}
//     DataType getTo() const{return to;}
//     DataType getCount() const{return count;}
    
//     void setFrom(const DataType & from){this->from=from;}
//     void setTo(const DataType & to){this->to=to;}
//     void setCount(const DataType& count){this->count=count;}

//     ruleList const& getSubrules() const {return this->subrules;}
//     void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
// };

// class DiscardRule : public Rule {
// private:
//     DataType from;
//     DataType count;
//     ruleList subrules;
// public:
//     DiscardRule(const nlohmann::json& rule);

//     DataType getFrom() const{return from;}
//     DataType getCount() const{return count;}

//     void setFrom(const DataType & from){this->from=from;}
//     void setCount(const DataType& count){this->count=count;}

//     ruleList const& getSubrules() const {return this->subrules;}
//     void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
// };

// class ListAttributesRule : public Rule {
// private:
//     DataType roles;
// public:
//     ListAttributesRule(const nlohmann::json& rule);
    
//     DataType getRoles() const{return roles;}

//     void setRoles(const DataType& roles) {this->roles=roles;}

// };

// //-------------------------------Junho's Code------------------------------//

// class ForEachRule : public Rule {
// private:
//     DataType list;
//     DataType element;
//     ruleList subrules;

// public:
//     ForEachRule(const nlohmann::json& rule);
//     // ~ForEachRule() override;

//     DataType getList() const {return list;}
//     void setList(const DataType& list) {this->list = list;}
//     DataType getElement() const {return element;}
//     void setElement(const DataType& element) {this->element = element;}
//     ruleList const& getSubrules() const {return this->subrules;}
//     void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}

// };

// class LoopRule : public Rule {
// private:
//     DataType until;
//     DataType whileCondition;
//     ruleList subrules;
// public:
//     LoopRule(const nlohmann::json& rule);

//     DataType getUntil() const {return this->until;}
//     void setUntil(const DataType& until) {this->until = until;}
//     DataType getWhile() const {return this->whileCondition;}
//     void setWhile(const DataType& whileCondition) {this->whileCondition = whileCondition;}

//     ruleList const& getSubrules() const {return this->subrules;}
//     void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
// };
  
// class InParallelRule : public Rule {
// private:
//     ruleList subrules;
// public:
//     InParallelRule(const nlohmann::json& rule);

//     ruleList const& getSubrules() const {return this->subrules;}
//     void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}
// };

// class ParallelForRule : public Rule {
// private:
//     DataType list;
//     DataType element;
//     ruleList subrules;
// public:
//     ParallelForRule(const nlohmann::json& rule);

//     DataType getList() const {return list;}
//     void setList(const DataType& list) {this->list = list;}
//     DataType getElement() const {return element;}
//     void setElement(const DataType& element) {this->element = element;}

//     ruleList const& getSubrules() const {return this->subrules;}
//     void setSubrules(ruleList subrules) {this->subrules=std::move(subrules);}

// };

// // Sorts a list in ascending order
// class SwitchRule : public Rule {
// private:
//     DataType list;
//     DataType value;
//     std::vector<Case> cases;
// public:
//     SwitchRule(const nlohmann::json& rule);

//     DataType getList() const {return list;}
//     void setList(const DataType& list) {this->list = list;}
//     DataType getValue() const {return this->value;}
//     void setValue(const DataType& value) {this->value = value;}

//     std::vector<Case> const& getCases() const {return this->cases;}
//     void setCases(std::vector<Case> cases) {this->cases=std::move(cases);}
// };

// class WhenRule : public Rule {
// private:
//     std::vector<Case> cases;
// public:
//     WhenRule(const nlohmann::json& rule);

//     std::vector<Case> const& getCases() const {return this->cases;}
//     void setCases(std::vector<Case> cases) {this->cases=std::move(cases);}
// };

// class RuleTree {
//     ruleList ruleTree;
// public:
//     RuleTree(const nlohmann::json& gameConfig);

//     // ~RuleTree();
// };

// std::unordered_map<std::string, std::function<std::unique_ptr<Rule>(const nlohmann::json&)>> rulemap = {
// 		{"foreach", [](const nlohmann::json& rule) { return std::make_unique<ForEachRule>(rule); }},
//         {"global-message", [](const nlohmann::json& rule) { return std::make_unique<GlobalMessageRule>(rule); }},
//         {"loop", [](const nlohmann::json&rule) {return std::make_unique<LoopRule>(rule);}},
//         {"inparallel", [](const nlohmann::json&rule) {return std::make_unique<InParallelRule>(rule);}},
//         {"parallelfor", [](const nlohmann::json&rule) {return std::make_unique<ParallelForRule>(rule);}},
//         {"extend", [](const nlohmann::json& rule) {return std::make_unique<ExtendRule>(rule); }}, 
//         {"reverse", [](const nlohmann::json& rule) {return std::make_unique<ReverseRule>(rule); }},
//         {"discard", [](const nlohmann::json& rule) {return std::make_unique<DiscardRule>(rule); }}, 
//         {"input-choice", [](const nlohmann::json& rule) {return std::make_unique<InputChoiceRule>(rule); }},
//         {"add", [](const nlohmann::json& rule) {return std::make_unique<AddRule>(rule); }},
//         {"scores", [](const nlohmann::json& rule) {return std::make_unique<ScoresRule>(rule); }}
// };

// //----------------------------------------Constructor implementation-------------------------------------------------------------------------------------------------
// GlobalMessageRule::GlobalMessageRule(const nlohmann::json& rule): Rule{rule["rule"]}{ 
//     definingDataType(rule["value"],value);
//     std::cout << "Global message: " << value << std::endl; 
// }

// ForEachRule::ForEachRule(const nlohmann::json& rule): Rule(rule["rule"]){
//     definingDataType(rule["list"],list);
//     definingDataType(rule["element"],element);
//     std::cout << "For each: " << element << std::endl;
//     for (const auto& it : rule["rules"].items())
//     {
// 		subrules.push_back(rulemap[it.value()["rule"]](it.value()));
//     }
// }

// LoopRule::LoopRule(const nlohmann::json& rule): Rule(rule["rule"]){
//     auto isUntil = rule.find("until");
//     if(isUntil != rule.end()){
//         definingDataType(rule["until"],until);
//         std::cout << "Loop until: " << until << std::endl;
//     } else {
//         definingDataType(rule["while"],whileCondition);
//         std::cout << "Loop while: " << whileCondition << std::endl;
//     }

//     for (const auto& it : rule["rules"].items()) {
//         subrules.push_back(rulemap[it.value()["rule"]](it.value()));
//     }
// }

// InParallelRule::InParallelRule(const nlohmann::json& rule): Rule(rule["rule"]) {
//     std::cout << "In Parallel: " << rule["rule"] << std::endl;
//     for (const auto& it : rule["rules"].items()) {
//         subrules.push_back(rulemap[it.value()["rule"]](it.value()));
//     }
// }

// ParallelForRule::ParallelForRule(const nlohmann::json& rule): Rule(rule["rule"]){
//     definingDataType(rule["list"],list);
//     definingDataType(rule["element"],element);

//     std::cout << "ParallelFor: " << rule["list"] << std::endl;
//     for (const auto& it : rule["rules"].items()) {
//         subrules.push_back(rulemap[it.value()["rule"]](it.value()));
//     }
// }

// // SwitchRule and WhenRule are under construction - needs to work on cases constructor

// // SwitchRule::SwitchRule(const nlohmann::json& rule): Rule(rule["rule"]), list(rule["list"]), value(rule["value"]) {

// //     // needs cases constructor first

// //     std::cout << "Switch For: " << rule["rule"] << std::endl;
// //     for (const auto& it : rule["cases"].items()) {
// //         subrules.push_back(rulemap[it.value()["rule"]](it.value()));
// //     }
// // }

// // WhenRule::WhenRule(const nlohmann::json& rule): Rule(rule["rule"]) {

// //     // needs cases constructor first

// //     std::cout << "When: " << rule["rule"] << std::endl;
// //     for (const auto& it : rule["cases"].items()) {
// //         subrules.push_back(rulemap[it.value()["rule"]](it.value()));
// //     }
// // }

// //TODO:Implement constructor of LoopRule,ParallelForRule,etc classes
// ExtendRule::ExtendRule(const nlohmann::json& rule) : Rule(rule["rule"]){
//     definingDataType(rule["target"],target);
//     definingDataType(rule["list"],list);

//     std::cout << "Extend list: " << rule["list"] << std::endl;
// }

// ReverseRule::ReverseRule(const nlohmann::json& rule) : Rule(rule["rule"]){
//     definingDataType(rule["list"],list);
//     std::cout << "Reserve list: " << list << std::endl;
//     for(const auto& it : rule["rules"].items()) {
//         subrules.push_back(rulemap[it.value()["rule"]](it.value()));
//     }
// }

// DiscardRule::DiscardRule(const nlohmann::json& rule) : Rule(rule["rule"]){
//     definingDataType(rule["from"],from);
//     definingDataType(rule["count"],count);
//     std::cout << "Discard count: " << count << std::endl;
// }

// InputChoiceRule::InputChoiceRule(const nlohmann::json& rule) : Rule(rule["rule"]) {
//     definingDataType(rule["to"],to);
//     definingDataType(rule["prompt"],prompt);
//     definingDataType(rule["choices"],choices);
//     definingDataType(rule["result"],result);

//     std::cout << "Input-Choice to: " << to << std::endl;
// }

// AddRule::AddRule(const nlohmann::json& rule) : Rule(rule["rule"]){
//     definingDataType(rule["to"],to);
//     definingDataType(rule["value"],value);
//     std::cout << "Add to: " << to << std::endl; 
// }

// ScoresRule::ScoresRule(const nlohmann::json& rule) : Rule(rule["rule"]){
//     definingDataType(rule["score"],score);
//     definingDataType(rule["ascending"],ascending);
//     std::cout << "Score: " << score << std::endl;
// }


 


// RuleTree::RuleTree(const nlohmann::json& gameConfig)
// {
//     for (const auto& it: gameConfig["rules"].items())
//     {
//         const nlohmann::json& rule = it.value();
//         const std::string& rulename = rule["rule"];
//         ruleTree.push_back(rulemap[rulename](rule));
//     }
// }

// // //-----------------------------------End of constructor implementation-----------------------------

// // // ForEachRule::~ForEachRule()
// // // {
// // //     for (auto ruleptr : subrules)
// // //         delete ruleptr;
// // // }

// // // GlobalMessageRule::~GlobalMessageRule() {}

// // // RuleTree::~RuleTree()
// // // {
// // //     for (auto ruleptr : ruleTree)
// // // 		delete ruleptr;
// // // }

// //----------------------------------------End Of Rule Class---------------------------------------//
// // Not completed
// class Player {

// };


