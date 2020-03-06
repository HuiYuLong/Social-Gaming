#include <Gamespec.h>

GameSpec::GameSpec(const Configuration& configuration): configuration(configuration) {}

Gamespec::getConfiguration(const Configuration& configuration) {
    return this->configuration;
}
Gamespec::setConfiguration(const Configuration& configuration) {
    this->configuration = configuration;
}

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