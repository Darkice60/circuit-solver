module;
#include <string>
#include <vector>

export module components;

export class Component {
protected:
	//Attributes
	std::string name;
	std::string node1, node2;
public:
	Component(std::string name, std::string node1, std::string node2)
		: name(name), node1(node1), node2(node2) {}

	std::string getName() const {
		return name;
	}

	std::vector<std::string> getNodes() const {
		return {node1, node2};
	}

	void setName(std::string name) {
		this->name = name;
	}

	void setNodes(std::vector<std::string> nodes) {
		if (nodes.size() != 2) {
			return;
		}
		this->node1 = nodes[0];
		this->node2 = nodes[1];
	}

	virtual ~Component() = default;
};

export class Resistor : public Component {
private:
	double resistance;
public:
	Resistor(std::string name, std::string node1, std::string node2, double resistance) 
		: Component(name, node1, node2), resistance(resistance) {}

	double getResistance() const {
		return resistance;
	}

	void setResistance(double resistance) {
		this->resistance = resistance;
	}
};

export class CurrentSource : public Component {
private:
	double current;
public:
	CurrentSource(std::string name, std::string node1, std::string node2, double current) 
		: Component(name, node1, node2), current(current) {}

	double getCurrent() const {
		return current;
	}

	void setCurrent(double current) {
		this->current = current;
	}
};

export class VoltageSource : public Component {
private:
	double voltage;
public:
	VoltageSource(std::string name, std::string node1, std::string node2, double voltage) 
		: Component(name, node1, node2), voltage(voltage) {}

	double getVoltage() const {
		return voltage;
	}

	void setVoltage(double voltage) {
		this->voltage = voltage;
	}
};