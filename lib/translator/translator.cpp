#include "include/translator.h"
#include <mutex>


class PseudoServer
{
	std::mutex lock;
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

// class TEST : public boost::static_visitor<>
// {
// public:
// 	void operator()(const bool& boolean) const
// 	{
// 		std::cout << (boolean ? "true" : "false") << std::endl;
// 	}

//     void operator()(const int& integer) const
//     {
//         std::cout << integer << std::endl;
//     }
// };

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Usage:\n  " << argv[0] << "<server config>\n"
				<< "  e.g. " << argv[0] << " ../configs/server/congfig1.json\n";
		return 1;
	}

	// boost::variant<bool, int> boolint = 0;
	// boost::apply_visitor(TEST(), boolint);
	// boolint = true;
	// boost::apply_visitor(TEST(), boolint);
	// return 0;


	std::ifstream serverconfig{argv[1]};
	if (serverconfig.fail())
    {
        std::cout << "cannot open the congiguration file" << std::endl;
        return 0;
    }
	nlohmann::json j = nlohmann::json::parse(serverconfig);

	std::vector<Configuration> configurations;
	std::vector<Variable> player_names;
	for (const std::string& name : {"a", "b", "c"})
		player_names.push_back(name);
	configurations.reserve(j["games"].size());
	for (const auto& [key, gamespecfile]: j["games"].items())
	{
		std::ifstream gamespecstream{gamespecfile};
		if (gamespecstream.fail())
		{
			std::cout << "cannot open the game configuration file " << gamespecfile << std::endl;
			return 0;
		}
		nlohmann::json gamespec = nlohmann::json::parse(gamespecstream);
		configurations.emplace_back(gamespec, player_names);
    }

	// TEST
	Variable& variables = configurations.front().getVariables();
	PrintTheThing p;
	boost::apply_visitor(p, variables);

	return 0;
}

// under construction
// requires more group meetings to flesh out a more concrete structure