#pragma once

#include <string>
#include <vector>

class Component
{
protected:
    // Attributes
    std::string _name;
    std::string _node1, _node2;

public:
    Component(std::string name, std::string node1, std::string node2)
        : _name(name), _node1(node1), _node2(node2) {
    }

    std::string getName() const
    {
        return _name;
    }

    std::vector<std::string> getNodes() const
    {
        return { _node1, _node2 };
    }

    void setName(std::string name)
    {
        this->_name = name;
    }

    void setNodes(std::vector<std::string> nodes)
    {
        if (nodes.size() != 2)
        {
            return;
        }
        this->_node1 = nodes[0];
        this->_node2 = nodes[1];
    }

    virtual ~Component() = default;
};

class Resistor : public Component
{
private:
    double _resistance;

public:
    Resistor(std::string name, std::string node1, std::string node2, double resistance)
        : Component(name, node1, node2), _resistance(resistance) {
    }

    double getResistance() const
    {
        return _resistance;
    }

    void setResistance(double resistance)
    {
        this->_resistance = resistance;
    }
};

class CurrentSource : public Component
{
private:
    double _current;

public:
    CurrentSource(std::string name, std::string node1, std::string node2, double current)
        : Component(name, node1, node2), _current(current) {
    }

    double getCurrent() const
    {
        return _current;
    }

    void setCurrent(double current)
    {
        this->_current = current;
    }
};

class VoltageSource : public Component
{
private:
    double _voltage;

public:
    VoltageSource(std::string name, std::string node1, std::string node2, double voltage)
        : Component(name, node1, node2), _voltage(voltage) {
    }

    double getVoltage() const
    {
        return _voltage;
    }

    void setVoltage(double voltage)
    {
        this->_voltage = voltage;
    }
};