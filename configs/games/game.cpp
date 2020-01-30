#include <fstream>
#include "json.hpp"

// for convenience
using json = nlohmann::json;

int main() {
    // read a JSON file
    std::ifstream i("rock_paper_scissors.json");
    json j;
    i >> j;
}
