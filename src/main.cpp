#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cctype>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <tuple>

#ifdef __INTELLISENSE__
#include "components-declarations.hpp"
#else
import components;
#endif

struct System {
	std::vector<std::vector<double>> A;
	std::vector<double> x;
	std::vector<double> b;
};

System constructMatrix(const std::vector<std::unique_ptr<Component>>& comps, const std::unordered_map<std::string, int>& nodes) {
	int n = nodes.size();
	int m = 0;
	for (const auto& comp : comps) {
		if (dynamic_cast<VoltageSource*>(comp.get())) {
			m++;
		}
	}

	int size = m + n;
	int a, b;
	std::vector<std::vector<double>> A(size, std::vector<double>(size, 0.0));
	std::vector<double> x;
	std::vector<double> B;

	for (const auto& comp : comps) {
		if (auto* resistor = dynamic_cast<Resistor*>(comp.get())) {
			double g = 1.0 / resistor->getResistance();

			auto [node1, node2] = resistor->getNodes();

			if (node1 != "0") {
				a = nodes.at(node1);
				A[a][a] += g;
			}

			if (node2 != "0") {
				b = nodes.at(node2);
				A[b][b] += g;
			}

			if (node1 != "0" && node2 != "0") {
				a = nodes.at(node1);
				b = nodes.at(node2);
				A[a][b] -= g;
				A[b][a] -= g;
			}
		}
		else if (auto* currentSource = dynamic_cast<CurrentSource*>(comp.get())) {
			double current = currentSource->getCurrent();

			auto [node1, node2] = currentSource->getNodes();

			if (node1 != "0") {
				a = nodes.at(node1);
				B[a] -= current;
			}

			if (node2 != "0") {
				b = nodes.at(node2);
				B[b] += current;
			}
		}
	}
}

std::unordered_map<std::string, int> findNodes(const std::vector<std::unique_ptr<Component>>& comps) {
	std::unordered_map<std::string, int> nodes;
	int i = 0;
	for (const auto& comp : comps) {
		auto [node1, node2] = comp->getNodes();
		for (const auto& node : { node1, node2 }) {
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