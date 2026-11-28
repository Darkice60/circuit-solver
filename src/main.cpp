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
#include <utility>

#ifdef __INTELLISENSE__
#include "components-declarations.hpp"
#else
import components;
#endif

double timeIncrement = 0.1;
double timeStop = 0.5;
bool UIC = false;

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

void updateCapacitors(const std::vector<std::unique_ptr<Component>>& comps, const std::unordered_map<std::string, int>& nodes, const std::vector<double>& x) {
	for (const auto& comp : comps) {
		if (auto* capacitor = dynamic_cast<Capacitor*>(comp.get())) {
			auto [node1, node2] = capacitor->getNodes();
			double v1 = (node1 == "0") ? 0.0 : x[nodes.at(node1)];
			double v2 = (node2 == "0") ? 0.0 : x[nodes.at(node2)];
			capacitor->setPrevVolt(v1 - v2);
		}
	}
}

void updateInductors(const std::vector<std::unique_ptr<Component>>& comps, const std::unordered_map<std::string, int>& nodes, const std::vector<double>& x) {
	int branchIndex = nodes.size();
	for (const auto& comp : comps) {
		if (dynamic_cast<VoltageSource*>(comp.get())) {
			branchIndex++;
		}
		else if (auto* inductor = dynamic_cast<Inductor*>(comp.get())) {
			inductor->setPrevCurrent(x[branchIndex]);
			branchIndex++;
		}
	}
}

System constructMatrix(const std::vector<std::unique_ptr<Component>>& comps, const std::unordered_map<std::string, int>& nodes, bool isDC = false) {
	int n = nodes.size();
	int m = 0;
	for (const auto& comp : comps) {
		if (dynamic_cast<VoltageSource*>(comp.get()) || dynamic_cast<Inductor*>(comp.get())) {
			m++;
		}
	}

	int size = m + n;
	int i1, i2;
	int branchIndex = n;
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
				A[i1][branchIndex] += 1.0;
				A[branchIndex][i1] += 1.0;
			}

			if (node2 != "0") {
				i2 = nodes.at(node2);
				A[i2][branchIndex] -= 1.0;
				A[branchIndex][i2] -= 1.0;
			}

			b[branchIndex] += voltage;
			branchIndex++;
		}
		else if (auto* capacitor = dynamic_cast<Capacitor*>(comp.get())) {
			if (isDC) {
				continue;
			}
			double g = (capacitor->getCapacitance() / timeIncrement);
			auto [node1, node2] = capacitor->getNodes();

			double prevVolt = capacitor->getPrevVolt();
			double iEq = g * prevVolt;

			if (node1 != "0") {
				i1 = nodes.at(node1);
				A[i1][i1] += g;
				b[i1] += iEq;
			}

			if (node2 != "0") {
				i2 = nodes.at(node2);
				A[i2][i2] += g;
				b[i2] -= iEq;
			}

			if (node1 != "0" && node2 != "0") {
				i1 = nodes.at(node1);
				i2 = nodes.at(node2);
				A[i1][i2] -= g;
				A[i2][i1] -= g;
			}
		}
		else if (auto* inductor = dynamic_cast<Inductor*>(comp.get())) {
			auto [node1, node2] = inductor->getNodes();

			if (node1 != "0") {
				i1 = nodes.at(node1);
				A[i1][branchIndex] += 1.0;
				A[branchIndex][i1] += 1.0;
			}

			if (node2 != "0") {
				i2 = nodes.at(node2);
				A[i2][branchIndex] -= 1.0;
				A[branchIndex][i2] -= 1.0;
			}

			if (!isDC) {
				double rEq = (inductor->getInductance() / timeIncrement);
				A[branchIndex][branchIndex] -= rEq;
				b[branchIndex] -= rEq * inductor->getPrevCurrent();
			}
			branchIndex++;
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
	std::string name;
	if (!(iss >> name)) {
		return nullptr;
	}

	char initialC = (char)tolower(name[0]);

	if (initialC == '*') {
		return nullptr;
	}

	if (name == ".tran") {
		if (!(iss >> timeIncrement >> timeStop)) {
			std::cerr << "Invalid time formatting: " << data << '\n';
			std::exit(1);
		}
		std::string uicFlag;
		if (iss >> uicFlag && uicFlag == "uic") {
			UIC = true;
		}
		return nullptr;
	}

	double value;
	std::string node1, node2;
	if (!(iss >> node1 >> node2 >> value)) {
		std::cerr << "Invalid component formatting: " << data << '\n';
		std::exit(1);
	}


	if (initialC == 'r') {
		return std::make_unique<Resistor>(name, node1, node2, value);
	}
	else if (initialC == 'i') {
		return std::make_unique<CurrentSource>(name, node1, node2, value);
	}
	else if (initialC == 'v') {
		return std::make_unique<VoltageSource>(name, node1, node2, value);
	}
	else if (initialC == 'c') {
		auto capacitor = std::make_unique<Capacitor>(name, node1, node2, value);
		double initialVoltage;
		if (iss >> initialVoltage) {
			capacitor->setPrevVolt(initialVoltage);
		}
		return capacitor;
	}
	else if (initialC == 'l') {
		auto inductor = std::make_unique<Inductor>(name, node1, node2, value);
		double initialCurrent;
		if (iss >> initialCurrent) {
			inductor->setPrevCurrent(initialCurrent);
		}
		return inductor;
	}
	else {
		std::cerr << "Invalid component type" << std::endl;
		std::exit(1);
	}
}

int main() {
	//read from circuit file
	std::ifstream file("examples/circuit.ckt");

	//if the file cant be opened
	if (!file.is_open()) {
		std::cerr << "Error, file not opened" << std::endl;
		return 1;
	}

	//declare component arrya
	std::vector<std::unique_ptr<Component>> comps;

	//decalre strring to parse file lines
	std::string line;

	//while the file has lines
	while (std::getline(file, line)) {
		//add the liens data to the componenet vector
		std::unique_ptr<Component> tempComp = createComp(line);
		if (tempComp == nullptr) {
			continue;
		}
		comps.push_back(std::move(tempComp));
	}

	auto nodes = findNodes(comps);

	if (!UIC) {
		System dcSystem = constructMatrix(comps, nodes, true);
		std::vector<double> x = solve(dcSystem);
		std::cout << "t=0.0";
		for (const auto& [node, index] : nodes) {
			std::cout << " | V(" << node << ")=" << x[index];
		}
		std::cout << '\n';
		updateCapacitors(comps, nodes, x);
		updateInductors(comps, nodes, x);
	}

	for (double t = timeIncrement; t <= timeStop + 1e-12; t += timeIncrement) {
		System system = constructMatrix(comps, nodes);
		std::vector<double> x = solve(system);
		std::cout << "t=" << t;
		for (const auto& [node, index] : nodes) {
			std::cout << " | V(" << node << ")=" << x[index];
		}
		std::cout << '\n';

		updateCapacitors(comps, nodes, x);
		updateInductors(comps, nodes, x);
	}

	return 0;
}