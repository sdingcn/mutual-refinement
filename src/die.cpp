#include "die.h"

void die(int code) {
	std::cerr << "Error: " << code << std::endl;
	std::exit(EXIT_FAILURE);
}
