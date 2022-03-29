#include "die.h"
#include <iostream>
#include <cstdlib>

void die(int code) {
	std::cerr << "Error: " << code << std::endl;
	std::exit(EXIT_FAILURE);
}
