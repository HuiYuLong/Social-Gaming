#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <translator.h>
// Mot completed
class Gamespec {
    private:
    Configuration configuration;
    Constants constants;
    Variables variables;
    PerPlayer perplayer;
    Rule rules;
    IndividualPlayer player;
    //PerAudience peraudience;
    public:
    Gamespec(Configuration configuration, Constants constants, Variables variables, PerPlayer perplayer, Rule rules, IndividualPlayer player) : Configuration(configuration), Constants(constants), Variables(variables), PerPlayer(perplayer), Rule(rules), IndividualPlayer(player) {}
    Configuration getConfiguration() const;
    Constants getConstants() const;
    Variables getVariables() const;
    PerPlayer getPerplayer() const;
    Rule getRules() const;
    IndividualPlayer getPlayer() const;

    Configuration setConfiguration(Configuration configuration);
    Constants setConstants(Constants constants);
    Variables setVariables(Variables variables);
    PerPlayer setPerplayer(PerPlayer perplayer);
    Rule setRules(Rule rules);
    IndividualPlayer setPlayer(IndividualPlayer player);

    virtual ~Gamespec() {}

};


