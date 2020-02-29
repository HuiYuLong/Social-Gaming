#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <translator.h>

class GameSpec {
public:
    GameSpec(const nlohmann::json& gamespec, const std::vector<Player>& players);
    const std::string& getName() const;
    size_t getPlayerCountMin() const;
    size_t getPlayerCountMax() const;
    Variable& getVariables();
    Connection getConnectionByName(const std::string& name);
    void launchGame(PseudoServer& server);
    std::thread launchGameDetached(PseudoServer& server);

private:
    std::string name;
    size_t playerCountMin;
    size_t playerCountMax;
    Variable variables;
    RuleTree rules;
    using PlayerMap = std::unordered_map<std::string, Connection>;
    PlayerMap playersMap;
};


