#include "die.h"
#include <cstdlib>
#include <iostream>

void die(int code) {
	std::cerr << "Error: " << code << std::endl;
	std::exit(EXIT_FAILURE);
}
