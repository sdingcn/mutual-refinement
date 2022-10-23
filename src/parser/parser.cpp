#include "parser.h"
#include "../hasher/hasher.h"
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <tuple>

std::vector<Line> parseGraphFile(const std::string &fName) {
	std::ifstream in(fName); // file auto closed via destructor
	std::vector<Line> lines;
	std::string rawLine;
	while (getline(in, rawLine)) {
		if (rawLine.find("->") != std::string::npos) {
			// node1->node2[label="label"]
			std::string::size_type p1 = rawLine.find("->");
			std::string::size_type p2 = rawLine.find('[');
			std::string::size_type p3 = rawLine.find('=');
			std::string::size_type p4 = rawLine.find(']');
			lines.push_back(
				std::make_tuple(
					rawLine.substr(0, p1 - 0),
					rawLine.substr(p3 + 2, p4 - 1 - (p3 + 2)),
					rawLine.substr(p1 + 2, p2 - (p1 + 2))
				)
			);
		}
	}
	return lines;
}
