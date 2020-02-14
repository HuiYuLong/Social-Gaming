#include "Gamespec.h"
#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <cmath>
#include <vector>
#include <deque>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <memory>
#include <utility>
#include <unordered_map>

/*
Gamespec Msg(Configuration &configuration, Constants constants,
                Variables &variables, PerPlayer &perplayer,
                PerAudience &peraudience, Rule &rule) {
                    // ToDo
                }
*/
// Getters and setters
std::unique_ptr<Configuration> Gamespec::getConfiguration() const {

    return this->configuration;
}

std::unique_ptr<Constants> Gamespec::getConstants() const

    return this->constants;
}
std::unique_ptr<Variables> Gamespec::getVariables() const{

    return this->variables;

}
std::unique_ptr<PerPlayer> Gamespec::getPerplayer() const{

    return this->perplayer;

}
std::vector<std::unique_ptr<Rule*>> Gamespec::getRuleTree() const{

    return this->ruleTree;

}

// Not completed
std::unique_ptr<Configuration> Gamespec::setConfiguration(std::unique_ptr<Configuration> configuration){

}
std::unique_ptr<Constants> Gamespec::setConstants(std::unique_ptr<Constants> constants){

}
std::unique_ptr<Variables> Gamespec::setVariables(std::unique_ptr<Variables> variables){

}
std::unique_ptr<PerPlayer> Gamespec::setPerplayer(std::unique_ptr<PerPlayer> perplayer){

}
std::vector<std::unique_ptr<Rule*>> Gamespec::setRuleTree(std::vector<std::unique_ptr<Rule*>> ruleTree){

    
}



Gamespec::~Gamespec() {}