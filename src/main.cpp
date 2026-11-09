#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cctype>
#include <memory>
#include <sstream>
#include <unordered_map>

#ifdef __INTELLISENSE__
#include "components-declarations.hpp"
#else
import components;
#endif

std::vector<std::vector<double>> constructMatrix(const std::vector<std::unique_ptr<Component>>& comps, const std::unordered_map<std::string, int>& nodes) {
	int n = nodes.size();
	int m = 0;
	for (const auto& comp : comps) {
		if (dynamic_cast<VoltageSource*>(comp.get())) {
			m++;
		}
	}

	int size = m + n;

	std::vector<std::vector<double>> A(size, std::vector<double>(size, 0.0));
}

std::unordered_map<std::string, int> findNodes(const std::vector<std::unique_ptr<Component>>& comps) {
	std::unordered_map<std::string, int> nodes;
	int i = 0;
	for (const auto& comp : comps) {
		for (const auto& node : comp->getNodes()) {
			if (node == "0") {
				continue;
			}

			if (!nodes.contains(node)) {
				nodes[node] = i++;
			}
		}
	}

	return nodes;
}

std::unique_ptr<Component> createComp(const std::string& data) {
	std::istringstream iss(data);

	std::string name, node1, node2;
	double value;

	if (!(iss >> name >> node1 >> node2 >> value)) {
		std::cerr << "Invalid component formatting: " << data << '\n';
		std::exit(1);
	}

	char initialC = (char)tolower(name[0]);
	if (initialC == 'r') {
		return std::make_unique<Resistor>(name, node1, node2, value);
	}
	else if (initialC == 'i') {
		return std::make_unique<CurrentSource>(name, node1, node2, value);
	}
	else if (initialC == 'v') {
		return std::make_unique<VoltageSource>(name, node1, node2, value);
	}
	else {
		std::cerr << "Invalid component type" << std::endl;
		std::exit(1);
	}
}

int main() {
	std::ifstream file("examples/circuit.ckt");

	if (!file.is_open()) {
		std::cerr << "Error, file not opened" << std::endl;
		return 1;
	}

	std::vector<std::unique_ptr<Component>> comps;

	std::string line;

	while (std::getline(file, line)) {
		comps.push_back(createComp(line));
	}

	auto nodes = findNodes(comps);



	//test code

	for (const auto& comp : comps) {
		std::cout << comp->getName() << '\n';
	}

	for (const auto& [node, index] : nodes) {
		std::cout << node << ": " << index << "\n";
	}

	return 0;
}