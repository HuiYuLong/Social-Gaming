

class GameSession {
public:
	GameSession(const GameSpec& spec, const std::vector<Player>& players):
        name(config["configuration"]["name"]),
        playerCountMin(config["configuration"]["player count"]["min"]),
        playerCountMax(config["configuration"]["player count"]["max"]),
        variables(Map()),
        rules(config["rules"])
    {
        // Add players
        // We will need to modify Configuration to progress further
        // We will need to create player_map in Configuration constructor
        // and we will later populate it here
        map["players"] = List();
        List& player_list = boost::get<List>(map["players"]);
        for(const Player& player: players) {
            Map player_map = boost::get<Map>(buildVariables(config["per-player"]));
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
	GameSpec spec;
    using PlayerMap = std::unordered_map<std::string, Connection>;
    PlayerMap playersMap;
};