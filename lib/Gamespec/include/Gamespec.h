#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <translator.h>

class GameSpec {
public:
    GameSpec(const Configuration& configuration);
    Configuration getConfiguration();
    Configuration setConfiguration();
private:
    Configuration configuration;

};


