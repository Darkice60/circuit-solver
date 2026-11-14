#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cctype>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <cmath>
#include <tuple>

#ifdef __INTELLISENSE__
#include "components-declarations.hpp"
#else
import components;
#endif

struct System {
	std::vector<std::vector<double>> A;
	std::vector<double> b;
};

std::vector<double> solve(const System& system) {
	const auto& A = system.A;
	const auto& b = system.b;

	int size = b.size();

	std::vector<double> x(size, 0.0);

	std::vector<std::vector<double>> augmented(size, std::vector<double>(size + 1, 0.0));
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			augmented[i][j] = A[i][j];
		}
		augmented[i][size] = b[i];
	}

	for (int i = 0; i < size; i++) {
		int maxRow = i;
		for (int j = i + 1; j < size; j++) {
			if (std::abs(augmented[j][i]) > std::abs(augmented[maxRow][i])) {
				maxRow = j;
			}
		}

		if (maxRow != i) {
			std::swap(augmented[i], augmented[maxRow]);
		}

		for (int k = i + 1; k < size; k++) {
			double multiplier = augmented[k][i] / augmented[i][i];
			for (int j = i; j <= size; j++) {
				augmented[k][j] -= multiplier * augmented[i][j];
			}
		}
	}

	for (int i = size - 1; i >= 0; i--) {
		double sum = augmented[i][size];
		for (int j = i + 1; j < size; j++) {
			sum -= augmented[i][j] * x[j];
		}
		x[i] = sum / augmented[i][i];
	}

	return x;
}

System constructMatrix(const std::vector<std::unique_ptr<Component>>& comps, const std::unordered_map<std::string, int>& nodes) {
	int n = nodes.size();
	int m = 0;
	for (const auto& comp : comps) {
		if (dynamic_cast<VoltageSource*>(comp.get())) {
			m++;
		}
	}

	int size = m + n;
	int i1, i2;
	int voltageIndex = n;
	std::vector<std::vector<double>> A(size, std::vector<double>(size, 0.0));
	std::vector<double> b(size, 0);

	for (const auto& comp : comps) {
		if (auto* resistor = dynamic_cast<Resistor*>(comp.get())) {
			double g = 1.0 / resistor->getResistance();

			auto [node1, node2] = resistor->getNodes();

			if (node1 != "0") {
				i1 = nodes.at(node1);
				A[i1][i1] += g;
			}

			if (node2 != "0") {
				i2 = nodes.at(node2);
				A[i2][i2] += g;
			}

			if (node1 != "0" && node2 != "0") {
				i1 = nodes.at(node1);
				i2 = nodes.at(node2);
				A[i1][i2] -= g;
				A[i2][i1] -= g;
			}
		}
		else if (auto* currentSource = dynamic_cast<CurrentSource*>(comp.get())) {
			double current = currentSource->getCurrent();

			auto [node1, node2] = currentSource->getNodes();

			if (node1 != "0") {
				i1 = nodes.at(node1);
				b[i1] -= current;
			}

			if (node2 != "0") {
				i2 = nodes.at(node2);
				b[i2] += current;
			}
		}
		else if (auto* voltageSource = dynamic_cast<VoltageSource*>(comp.get())) {
			double voltage = voltageSource->getVoltage();

			auto [node1, node2] = voltageSource->getNodes();

			if (node1 != "0") {
				i1 = nodes.at(node1);
				A[i1][voltageIndex] += 1.0;
				A[voltageIndex][i1] += 1.0;
			}

			if (node2 != "0") {
				i2 = nodes.at(node2);
				A[i2][voltageIndex] -= 1.0;
				A[voltageIndex][i2] -= 1.0;
			}

			b[voltageIndex] += voltage;
			voltageIndex++;
		}
	}

	return System{ A, b };
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

	System system = constructMatrix(comps, nodes);

	std::vector<double> x = solve(system);

	for (const auto& sol : x) {
		std::cout << sol << '\n';
	}

	//test code

	for (const auto& comp : comps) {
		std::cout << comp->getName() << '\n';
	}

	for (const auto& [node, index] : nodes) {
		std::cout << node << ": " << index << "\n";
	}

	return 0;
}