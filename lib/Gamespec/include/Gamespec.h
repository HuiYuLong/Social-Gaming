#include <translator.h>


class Gamespec {
private:
    Top_levelMap& theMap;
    RuleTree& ruleTree;

public:
    Gamespec(const Top_levelMap& theMap);

    Top_levelMap& getTheMap() {return this->theMap;}
    RuleTree& getRuleTree() {return this->ruleTree;}

};
