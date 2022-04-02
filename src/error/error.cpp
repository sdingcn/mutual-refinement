#include "error.h"
#include <cstdlib>
#include <iostream>

void error(int code) {
	std::cerr << "Error: " << code << std::endl;
	std::exit(EXIT_FAILURE);
}
