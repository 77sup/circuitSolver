#include <string>
#include <cstdint>
#include <limits>
#include <vector>
#include <deque>
#include <set>
#include <unordered_map>
#include <cassert>
#include <sstream>
#include <fstream>
#include <iostream>
#include "log.h"

#ifndef CIRCUIT_GRAPH_H
#define CIRCUIT_GRAPH_H
#pragma once

class Gate;
class CircuitGraph;

struct Line
{
	void connect_as_input(Gate *gate)
	{
		destination_gates.insert(gate);
	}
	Line(int output_name, bool be_output)
	{
		num_name=output_name;
		is_output=be_output;
	}
	Gate *source = nullptr;   // nullptr means input port
	std::set<Gate *> destination_gates;

	bool is_output = false;

	std::string name;
	int num_name;

};

class Gate
{
public:
	enum class Type : uint32_t
	{
		And,
		Nand,
		Not,
		Or,
		Nor,
		Xor,
		Xnor,
		Buff,

		Undefined,
	};

	Gate(Type type, Line *output, std::vector<Line *> &&inputs);

	Gate(const Gate &) = delete;

	Type get_type() const { return m_type; }
	Type &type() { return m_type; }

	const std::vector<Line *> &get_inputs() const { return m_inputs; }
	std::vector<Line *> &inputs() { return m_inputs; }

	std::string get_str() const;

	Line *get_output() const { return m_output; }
	Line *&output() { return m_output; }
	//new add
	bool get_is_learnt_gate() const { return is_learn_gate;}
	const std::vector<int> &get_inputs_polarity() const { return inputs_polarity; }
	std::vector<int> &change_inputs_polarity() { return inputs_polarity; }


private:
	Type m_type = Type::Undefined;
	std::vector<Line *> m_inputs;
	Line *m_output = nullptr;
	bool is_learn_gate = false;
	std::vector<int> inputs_polarity;
};

class CircuitGraph{
public:
	Line *add_input(const std::string &name);
	Line *add_output(const std::string &name);

	Gate *add_gate(Gate::Type type, const std::vector<std::string> &input_names, const std::string &output_name);

	Line *get_line(const int &name);

	const Line *get_line(const int &name) const;

	const std::vector<Line *> &get_inputs() const;
	const std::vector<Line *> &get_outputs() const;

	const std::deque<Gate> &get_gates() const;
	const std::deque<Line> &get_lines() const;
	//const std::unordered_map<int, Line *> get_name_to_line() const;
	std::unordered_map<int, Line *> m_name_to_line;
	void get_graph_stats() const;
	//int change_name(std::string) const;
	int change_name(const std::string name) 
	{
		int str_len = name.size();
		int i = 0;
		int j = 0;
		//std::cout<<"the name before change : "<<name<<std::endl;
		while (i < str_len)
		{
			if (name[i] >= '0' && name[i] <= '9')
			{
				j = i;
				int len = 0;
				while (name[i] >= '0' && name[i] <= '9')
				{
					i++;
					len++;
				}
				std::string num_name = name.substr(j, len); //获取子串
				int change_name = 0;						//数字字符串转换为整型数字
				std::stringstream s1(num_name);
				s1 >> change_name;
				//std::cout<<"the name after change : ";
				//std::cout<<change_name<<std::endl;
				return change_name;
			}
			else
			{
				i++;
			}
		}
	}
private:
	Line *ensure_line(const int &name);
	// We need to avoid relocations on element addition, hence deque
	std::deque<Line> m_lines;
	std::deque<Gate> m_gates;

	std::vector<Line *> m_inputs;
	std::vector<Line *> m_outputs;
};

#endif