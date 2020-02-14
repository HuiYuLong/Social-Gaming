#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <nlohmann/json.hpp>
#include "translator.h"
#include <cmath>
#include <vector>
#include <deque>
#include <fstream>
#include <sstream>
#include <memory>
#include <utility>
#include <unordered_map>

class Gamespec {
    private:
    std::unique_ptr<Configuration> configuration;
    std::unique_ptr<Constants> constants;
    std::unique_ptr<Variables> variables;
    std::unique_ptr<PerPlayer> perplayer;
    std::vector<std::unique_ptr<Rule*>> ruleTree;
  
    //PerAudience peraudience;
    public:
    Gamespec(std::unique_ptr<Configuration> configuration, std::unique_ptr<Constants> constants, std::unique_ptr<Variables> variables, std::unique_ptr<PerPlayer> perplayer, std::vector<std::unique_ptr<Rule*>> ruleTree) : std::unique_ptr<Configuration>(configuration), std::unique_ptr<Constants>(constants), std::unique_ptr<Variables>(variables), std::unique_ptr<PerPlayer>(perplayer), std::vector<std::unique_ptr<Rule*>>(ruleTree) {}
    std::unique_ptr<Configuration> getConfiguration() const;
    std::unique_ptr<Constants> getConstants() const;
    std::unique_ptr<Variables> getVariables() const;
    std::unique_ptr<PerPlayer> getPerplayer() const;
    std::vector<std::unique_ptr<Rule*>> getRuleTree() const;


    std::unique_ptr<Configuration> setConfiguration(std::unique_ptr<Configuration> configuration);
    std::unique_ptr<Constants> setConstants(std::unique_ptr<Constants> constants);
    std::unique_ptr<Variables> setVariables(std::unique_ptr<Variables> variables);
    std::unique_ptr<PerPlayer> setPerplayer(std::unique_ptr<PerPlayer> perplayer);
    std::vector<std::unique_ptr<Rule*>> setRuleTree(std::vector<std::unique_ptr<Rule*>> ruleTree);
  

    virtual ~Gamespec() {}

};


