#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

// classes in this header are under construction

class Configuration {
public:
	std::string name;
	int playerCountMin;
	int playerCountMax;
	bool audience;
	nlohmann::json setup;
	Configuration(std::string name) {
		this->name = name;
	}
};

class Constants {
public:
};

class Variables {
public:
};

class PerPlayer {
public:
};

// maybe we should construct separate class for the different rule classes?
// ex) OutputRule, InputRule, ArithmeticRule, ListOpsRule, ControlStructRule ...

class Rule {
public:
	std::string type; // "foreach", "scores" -> the field under "rule": in the json
	int list;
	std::string element;
	//Graph rules; // a graph data structure to hold subrules ... implementation to be further discussed.
};

class Player {
public:
};