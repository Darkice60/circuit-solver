module;
#include <string>

export class Component;

class Component {
public:
	//Attributes
	std::string name;
	std::string node1, node2;

	Component(std::string name, std::string node1, std::string node2)
		: name(name), node1(node1), node2(node2) {}
};

class Resistor : public Component {
	double resistance;
	Resistor(std::string name, std::string node1, std::string node2, double resistance) 
		: Component(name, node1, node2), resistance(resistance) {}
};