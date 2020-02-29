#include <Gamespec.h>

GameSpec::GameSpec(const nlohmann::json& gamespec, const std::vector<Player>& players):
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

const std::string& GameSpec::getName() const { return name; }
size_t GameSpec::getPlayerCountMin() const { return playerCountMin; }
size_t GameSpec::getPlayerCountMax() const { return playerCountMax; }
Variable& GameSpec::getVariables() { return variables; }
Connection GameSpec::getConnectionByName(const std::string& name) { return playersMap[name]; }
void GameSpec::launchGame(PseudoServer& server) { rules.spawn(server, *this); }
std::thread GameSpec::launchGameDetached(PseudoServer& server) { return rules.spawn_detached(server, *this); }

// temporarily test code ... should eventually migrate code away from test code in translator.cpp

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage:\n  " << argv[0] << "<server config>\n"
                << "  e.g. " << argv[0] << " ../configs/server/congfig1.json\n";
        return 1;
    }

    // Variable a = "a";
    // boost::apply_visitor(TEST(), a);
    // a = Query{"b"};
    // boost::apply_visitor(TEST(), a);
    // return 0;

    std::ifstream serverconfig{argv[1]};
    if (serverconfig.fail())
    {
        std::cout << "cannot open the congiguration file" << std::endl;
        return 0;
    }
    nlohmann::json j = nlohmann::json::parse(serverconfig);

    std::vector<GameSpec> configurations;
    std::vector<Player> players;
    for (const std::string& name : {"a", "b"})
        players.emplace_back(name, Connection());
    configurations.reserve(j["games"].size());
    for ([[maybe_unused]] const auto& [ key, gamespecfile]: j["games"].items())
    {
        std::ifstream gamespecstream{gamespecfile};
        if (gamespecstream.fail())
        {
            std::cout << "cannot open the game configuration file " << gamespecfile << std::endl;
            return 0;
        }
        nlohmann::json gamespec = nlohmann::json::parse(gamespecstream);
        configurations.emplace_back(gamespec, players);
        std::cout << "\nTranslated game " << key << "\n\n";
    }

    // TEST
    Variable& variables = configurations.front().getVariables();
    PrintTheThing p;
    boost::apply_visitor(p, variables);

    std::cout << "\nStarting a test\n\n";
    PseudoServer server;
    std::thread t1 = configurations.front().launchGameDetached(server);
    // std::thread t2 = configurations.back().launchGameDetached(server);
    t1.join();
    // t2.join();
    //PseudoServer server;
    //configurations.back().launchGame(server);
    // for(GameSpec& c : configurations) {
    //  std::cout << "\nGame " << c.getName() << "\n\n";
    //  c.launchGame(server);
    // }
    std::cout << "\nFinished\n";

    return 0;
};