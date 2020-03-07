#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <limits>
//#include <translator.h>
#include "../../translator/include/translator.h"

class Gamespec {
public:
    Gamespec(Configuration& configuration){
    	this->configuration = &configuration;
    }
    // Configuration getConfiguration();
    // Configuration setConfiguration(const Configuration& configuration);
private:
    Configuration* configuration;
};


