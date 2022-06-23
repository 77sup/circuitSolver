#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include <vector>
solver::solver(const CircuitGraph &graph)
{
    std::vector<std::string> noPI_lines_name; // store no-PIs
    std::vector<std::string> output;          // store PIs
    std::cout << graph.get_name_to_line().size() << std::endl;
    for (int i = 0; i < graph.get_lines().size(); i++)
    {
        if (graph.get_lines()[i].is_output)
        {
            output.push_back(graph.get_lines()[i].name);
        }
        else
            noPI_lines_name.push_back(graph.get_lines()[i].name);
    }
    for (int i = 0; i < noPI_lines_name.size(); i++)
    {
        lines_status.insert(make_pair(noPI_lines_name[i], -1));
        // find PIs to store
        if (!graph.get_name_to_line().at(noPI_lines_name[i])->source)
        {
            the_name_of_input_line.push_back(noPI_lines_name[i]);
        }
    }
    // print the_name_of_input_line
    for (int i = 0; i < the_name_of_input_line.size(); i++)
    {
        std::cout << "the_name_of_input_line:" << the_name_of_input_line[i] << std::endl;
    }
    // assign POs to 1
    for (int i = 0; i < output.size(); i++)
    {
        lines_status.insert(make_pair(output[i], 1));
    }
    // test,print line's assign status
    std::cout << std::endl;
    for (auto it = lines_status.begin(); it != lines_status.end(); it++)
    {
        std::cout << "the_status_of_line:" << (*it).first << "    " << (*it).second << std::endl;
    }
    // according to fan_outs numbers to order(max---min)
    std::string change;
    for (int i = 0; i < noPI_lines_name.size() - 1; i++)
    {
        for (int j = i + 1; j < noPI_lines_name.size(); j++)
        {
            if (graph.get_name_to_line().at(noPI_lines_name[i])->destination_gates.size() < graph.get_name_to_line().at(noPI_lines_name[j])->destination_gates.size())
            {
                change = noPI_lines_name[i];
                noPI_lines_name[i] = noPI_lines_name[j];
                noPI_lines_name[j] = change;
            }
        }
    }
    // store ordered noPI_lines
    for (int i = 0; i < noPI_lines_name.size(); i++)
    {
        sort_destination_gates.push_back(noPI_lines_name[i]);
    }
    // test
    for (int i = 0; i < sort_destination_gates.size(); i++)
    {
        std::cout << "sort_line_name:" << sort_destination_gates[i] << " num:  " << graph.get_name_to_line().at(sort_destination_gates[i])->destination_gates.size() << std::endl;
    }
}
// choose a line to assign(decision),according to ordered fan_outs numbers
std::string solver::FindDecisionTarget(std::unordered_map<std::string, int> &lines_status)
{
    std::string Target;
    for (int i = 0; i < sort_destination_gates.size(); i++)
    {
        if (lines_status[sort_destination_gates[i]] == -1)
        {
            Target = sort_destination_gates[i];
            break;
        }
    }
    return Target;
}
void solver::test(const CircuitGraph &graph)
{
    //BCP(graph);
    this->lines_status.at(graph.get_lines()[2].name)=1;
    Gate*  back_gate1=*(graph.get_lines()[2].destination_gates.find(0));
    Gate* front_gate1=graph.get_lines()[2].source;
    SingleGateReasoning(back_gate1, graph.get_lines()[2].name);
}
int solver::BCP(const CircuitGraph &graph)
{
    std::string x1 = graph.get_lines()[5].name;
    std::cout << graph.get_name_to_line().at(x1)->name << std::endl;

    std::vector<std::string> bcp;
    return 0;
    for (;;)
    {

        for (;;)
        {
        }
    }
}

bool solver::SingleGateReasoning(Gate *current_gate, std::string reason_line_name)
{
    int flag1 = -1; // the number of  assigned lines
    int flag2 = -1; // judge gate's type; judge do/not do intersect
    Gate::Type GateType = current_gate->get_type();
    Line* input1 = current_gate->get_inputs()[0];
    Line* input2 = current_gate->get_inputs()[1];
    Line* output0 = current_gate->get_output();

    switch (GateType)
    {
    case Gate::Type::And:
    {

    }
        return true;
        break;
    case Gate::Type::Nand:
        return true;
        break;
    case Gate::Type::Not:
        return true;
        break;
    case Gate::Type::Or:
        return true;
        break;
    case Gate::Type::Nor:
        return true;
        break;
    case Gate::Type::Xor:
        return true;
        break;
    case Gate::Type::Xnor:
        return true;
        break;
    case Gate::Type::Buff:
        return true;
        break;
    case Gate::Type::Undefined:
        return true;
        break;

    default:
        return false;
        break;
    }
    return false;
}