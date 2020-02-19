#include "include/translator.h"

GlobalMessageRule::GlobalMessageRule(const nlohmann::json& rule): value(rule["value"]) { std::cout << "Global message: " << rule["value"] << std::endl; }

ForEachRule::ForEachRule(const nlohmann::json& rule): list(rule["list"]), element_name(rule["element"])
{
    std::cout << "For each: " << element_name << std::endl;
    for (const auto& it : rule["rules"].items())
    {
		subrules.push_back(rulemap[it.value()["rule"]](it.value()));
    }
}

WhenRule::WhenRule(const nlohmann::json& rule)
{
    std::cout << "When" << std::endl;
	cases.reserve(rule["cases"].size());
    for(const auto& case_ : rule["cases"].items()) {
        std::cout << case_.value()["condition"] << std::endl;
        cases.emplace_back(case_.value());
    }
}

Case::Case(const nlohmann::json& case_): condition(case_["condition"]) {
	for (const auto& subrule : case_["rules"].items()) {
		subrules.push_back(rulemap[subrule.value()["rule"]](subrule.value()));
	}
}

AddRule::AddRule(const nlohmann::json& rule): to(rule["to"]), value(rule["value"]) { std::cout << "Add " << value << std::endl; }

RuleTree::RuleTree(const nlohmann::json& gameConfig)
{
    for (const auto& it: gameConfig.items())
    {
        const nlohmann::json& rule = it.value();
        const std::string& rulename = rule["rule"];
        rules.push_back(rulemap[rulename](rule));
    }
}

RuleTree::RuleTree(RuleTree&& oldTree)
{
    for (std::unique_ptr<Rule>& ptr : oldTree.getRules()) {
        rules.push_back(std::move(ptr));
    }
}

RuleTree& RuleTree::operator=(RuleTree&& oldTree)
{
    rules.clear();
    for (std::unique_ptr<Rule>& ptr : oldTree.getRules()) {
        rules.push_back(std::move(ptr));
    }
    return *this;
}

ruleList& RuleTree::getRules() { return rules; }



void GlobalMessageRule::run(PseudoServer& server, GameSpec& spec)
{
	List& players = boost::get<List>(boost::get<Map>(spec.getVariables())["players"]);
	for (Variable& player : players) {
		const std::string& name = boost::get<std::string>(boost::get<Map>(player)["name"]);
		server.send({spec.getConnectionByName(name), value.fill_with(spec.getVariables())});
	}
}

void ForEachRule::run(PseudoServer& server, GameSpec& spec)
{
	Getter getter(list);
	GetterResult result = getter.get_from(spec.getVariables());
	Variable temp;
	List& elements = boost::get<List>(result.needs_to_be_saved ? temp = result.result, temp : result.result);
	Map& toplevel = boost::get<Map>(spec.getVariables());
	for (Variable& element : elements) {
		toplevel[element_name] = &element;
		//PrintTheThing p;
		//boost::apply_visitor(p, spec.getVariables());
		for (const auto& ptr : subrules) {
			ptr->run(server, spec);
		}
	}
}

void WhenRule::run(PseudoServer& server, GameSpec& spec)
{
	for(Case& current_case : cases) {
		if (current_case.condition.evaluate(spec.getVariables())) {
			for (const auto& ptr : current_case.subrules) {
				ptr->run(server, spec);
			}
			break;
		}
	}
}

void AddRule::run(PseudoServer& server, GameSpec& spec)
{
	Getter getter(to);
	GetterResult result = getter.get_from(spec.getVariables());
	assert(!result.needs_to_be_saved);
	int& integer = boost::get<int>(result.result);
	integer += value;
}

// class TEST : public boost::static_visitor<>
// {
// public:

//     void operator()(bool boolean) const
//     {
//         std::cout << (boolean ? "true" : "false") << std::endl;
//     }

//     void operator()(int integer) const
//     {
//         std::cout << integer << std::endl;
//     }

//     void operator()(const std::string& string) const
//     {
//         std::cout << string << std::endl;
//     }

//     void operator()(const List& list) const
//     {
//         std::cout << "Size: " << list.size() << std::endl;
//     }

//     void operator()(const Map& map) const
//     {
// 		std::cout << "Map: " << map.size() << std::endl;
//     }
// };

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Usage:\n  " << argv[0] << "<server config>\n"
				<< "  e.g. " << argv[0] << " ../configs/server/congfig1.json\n";
		return 1;
	}

	// Variable boovec = true;
	// boost::apply_visitor(TEST(), boovec);
	// List list;
	// list.push_back(1);
	// boovec = list;
	// boost::apply_visitor(TEST(), boovec);
	// List& newlist = boost::get<List>(boovec);
	// newlist.reserve(10);
	// newlist.push_back(4);
	// std::iota(newlist.begin(), newlist.end(), 11);
	// boost::apply_visitor(TEST(), boovec);
	// return 0;


	// Variable l = List();
	// const List& ref = boost::get<List>(l);
	// const Variable& cvref = ref;
	// std::cout << cvref.which() << std::endl;
	// Variable& vref = const_cast<Variable&>(cvref);
	// std::cout << vref.which() << std::endl;
	// List& newlist = boost::get<List>(vref);
	// newlist.push_back(5);
	// std::cout << newlist.size() << std::endl;
	// std::cout << ref.size() << std::endl;
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
	for (const auto& [key, gamespecfile]: j["games"].items())
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
	Variable& variables = configurations.back().getVariables();
	PrintTheThing p;
	boost::apply_visitor(p, variables);

	std::cout << "\nStarting a test\n\n";
	PseudoServer server;
	std::thread t1 = configurations.front().launchGameDetached(server);
	std::thread t2 = configurations.back().launchGameDetached(server);
	t1.join();
	t2.join();
	//PseudoServer server;
	//configurations.back().launchGame(server);
	// for(GameSpec& c : configurations) {
	// 	std::cout << "\nGame " << c.getName() << "\n\n";
	// 	c.launchGame(server);
	// }
	std::cout << "\nFinished\n";

	return 0;
}

// under construction
// requires more group meetings to flesh out a more concrete structure