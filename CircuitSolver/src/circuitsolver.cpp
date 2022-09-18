#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include <vector>
#include <queue>
int number = 0;
// design for multiple inputs gates
bool solver::SingleGateReason(Gate *current_gate, std::queue<int> &bcp_que, int decision_line)
{
    number++;
    int number_lineOfGate = current_gate->get_inputs().size();
    int level = lines_status_num.at(decision_line).level;
    Gate::Type GateType = current_gate->get_type();
    // inputs_lines_assign[0]---store value 0's number;inputs_lines_assign[1]---store value 1's number;inputs_lines_assign[2]---store value x's number
    std::vector<std::vector<int>> inputs_lines_assign(3);
    for (int i = 0; i < current_gate->get_inputs().size(); i++)
    {
        int input = current_gate->inputs()[i]->num_name;
        int assign = lines_status_num.at(input).assign;
        if (assign != -1)
        {
            inputs_lines_assign[assign].push_back(input);
        }
        else
        {
            inputs_lines_assign[2].push_back(input);
        }
    }
    int output_name = current_gate->get_output()->num_name;
    int output_assign = lines_status_num.at(output_name).assign;

    switch (GateType)
    {
    case Gate::Type::And:
    {
        if ((output_assign == 0 && inputs_lines_assign[1].size() == number_lineOfGate) || (output_assign == 1 && inputs_lines_assign[0].size() > 0))
        {
            if (output_assign)
            {
                conflict_line.push_back(output_name);
                conflict_line.push_back(inputs_lines_assign[0][0]); //!!!
            }
            else
            {
                conflict_line = inputs_lines_assign[1];
                conflict_line.push_back(output_name);
            }
            return false; // conflict
        }
        else if (output_assign == 0 && inputs_lines_assign[2].size() == 1 && inputs_lines_assign[1].size() == number_lineOfGate - 1)
        {
            lines_status_num.at(inputs_lines_assign[2][0]).assign = 0;
            lines_status_num.at(inputs_lines_assign[2][0]).level = level;
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines = inputs_lines_assign[1];
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(output_name);
            bcp_que.push(inputs_lines_assign[2][0]);
            return true;
        }
        else if (output_assign == 1 && inputs_lines_assign[2].size() > 0)
        {
            for (int i = 0; i < inputs_lines_assign[2].size(); i++)
            {
                lines_status_num.at(inputs_lines_assign[2][i]).assign = 1;
                lines_status_num.at(inputs_lines_assign[2][i]).level = level;
                lines_status_num.at(inputs_lines_assign[2][i]).source_lines.push_back(output_name);
                bcp_que.push(inputs_lines_assign[2][i]);
                return true;
            }
        }
        else if (output_assign == -1 && inputs_lines_assign[0].size() > 0)
        {
            lines_status_num.at(output_name).assign = 0;
            lines_status_num.at(output_name).level = level;
            lines_status_num.at(output_name).source_lines.push_back(inputs_lines_assign[0][0]);
            bcp_que.push(output_name);
            return true;
        }
        else if (output_assign == -1 && inputs_lines_assign[1].size() == number_lineOfGate)
        {
            lines_status_num.at(output_name).assign = 1;
            lines_status_num.at(output_name).level = level;
            lines_status_num.at(output_name).source_lines = inputs_lines_assign[1];
            bcp_que.push(output_name);
            return true;
        }
        else
        {
            return true; // nothing can be reasoned
        }
    }
    case Gate::Type::Nand:
    {
        if ((output_assign == 1 && inputs_lines_assign[1].size() == number_lineOfGate) || (output_assign == 0 && inputs_lines_assign[0].size() > 0))
        {
            if (!output_assign)
            {
                conflict_line.push_back(output_name);
                conflict_line.push_back(inputs_lines_assign[0][0]); //!!!
            }
            else
            {
                conflict_line = inputs_lines_assign[1];
                conflict_line.push_back(output_name);
            }
            return false; // conflict
        }
        else if (output_assign == 1 && inputs_lines_assign[2].size() == 1 && inputs_lines_assign[1].size() == number_lineOfGate - 1)
        {
            lines_status_num.at(inputs_lines_assign[2][0]).assign = 0;
            lines_status_num.at(inputs_lines_assign[2][0]).level = level;
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines = inputs_lines_assign[1];
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(output_name);
            bcp_que.push(inputs_lines_assign[2][0]);
            return true;
        }
        else if (output_assign == 0 && inputs_lines_assign[2].size() > 0)
        {
            for (int i = 0; i < inputs_lines_assign[2].size(); i++)
            {
                lines_status_num.at(inputs_lines_assign[2][i]).assign = 1;
                lines_status_num.at(inputs_lines_assign[2][i]).level = level;
                lines_status_num.at(inputs_lines_assign[2][i]).source_lines.push_back(output_name); // change
                bcp_que.push(inputs_lines_assign[2][i]);
                return true;
            }
        }
        else if (output_assign == -1 && inputs_lines_assign[0].size() > 0)
        {
            lines_status_num.at(output_name).assign = 1;
            lines_status_num.at(output_name).level = level;
            lines_status_num.at(output_name).source_lines.push_back(inputs_lines_assign[0][0]);
            bcp_que.push(output_name);
            return true;
        }
        else if (output_assign == -1 && inputs_lines_assign[1].size() == number_lineOfGate)
        {
            lines_status_num.at(output_name).assign = 0;
            lines_status_num.at(output_name).level = level;
            lines_status_num.at(output_name).source_lines = inputs_lines_assign[1];
            bcp_que.push(output_name);
            return true;
        }
        else
        {
            return true; // nothing can be reasoned
        }
    case Gate::Type::Or:
    {
        if ((output_assign == 0 && inputs_lines_assign[1].size() > 0) || (output_assign == 1 && inputs_lines_assign[0].size() == number_lineOfGate))
        {
            if (!output_assign)
            {
                conflict_line.push_back(output_name);
                conflict_line.push_back(inputs_lines_assign[1][0]); //!!!
            }
            else
            {
                conflict_line = inputs_lines_assign[0];
                conflict_line.push_back(output_name);
            }
            return false; // conflict
        }
        else if (output_assign == 0 && inputs_lines_assign[2].size() > 0)
        {
            for (int i = 0; i < inputs_lines_assign[2].size(); i++)
            {
                lines_status_num.at(inputs_lines_assign[2][i]).assign = 0;
                lines_status_num.at(inputs_lines_assign[2][i]).level = level;
                lines_status_num.at(inputs_lines_assign[2][i]).source_lines.push_back(output_name); // change
                bcp_que.push(inputs_lines_assign[2][i]);
            }
            return true;
        }
        else if (output_assign == 1 && inputs_lines_assign[0].size() == number_lineOfGate - 1 && inputs_lines_assign[2].size() == 1)
        {
            lines_status_num.at(inputs_lines_assign[2][0]).assign = 1;
            lines_status_num.at(inputs_lines_assign[2][0]).level = level;
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines = inputs_lines_assign[0];
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(output_name);
            bcp_que.push(inputs_lines_assign[2][0]);
            return true;
        }
        else if (output_assign == -1 && inputs_lines_assign[0].size() == number_lineOfGate)
        {
            lines_status_num.at(output_name).assign = 0;
            lines_status_num.at(output_name).level = level;
            lines_status_num.at(output_name).source_lines = inputs_lines_assign[0];
            bcp_que.push(output_name);
            return true;
        }
        else if (output_assign == -1 && inputs_lines_assign[1].size() > 0)
        {
            lines_status_num.at(output_name).assign = 1;
            lines_status_num.at(output_name).level = level;
            lines_status_num.at(output_name).source_lines.push_back(inputs_lines_assign[1][0]);
            bcp_que.push(output_name);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Nor:
    {
        if ((output_assign == 1 && inputs_lines_assign[1].size() > 0) || (output_assign == 0 && inputs_lines_assign[0].size() == number_lineOfGate))
        {
            if (output_assign)
            {
                conflict_line.push_back(output_name);
                conflict_line.push_back(inputs_lines_assign[1][0]); //!!!
            }
            else
            {
                conflict_line = inputs_lines_assign[0];
                conflict_line.push_back(output_name);
            }
            return false; // conflict
        }
        else if (output_assign == 0 && inputs_lines_assign[0].size() == number_lineOfGate - 1 && inputs_lines_assign[2].size() == 1)
        {
            lines_status_num.at(inputs_lines_assign[2][0]).assign = 1;
            lines_status_num.at(inputs_lines_assign[2][0]).level = level;
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines = inputs_lines_assign[0];
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(output_name);
            bcp_que.push(inputs_lines_assign[2][0]);
            return true;
        }
        else if (output_assign == 1 && inputs_lines_assign[2].size() > 0)
        {
            for (int i = 0; i < inputs_lines_assign[2].size(); i++)
            {
                lines_status_num.at(inputs_lines_assign[2][i]).assign = 0;
                lines_status_num.at(inputs_lines_assign[2][i]).level = level;
                lines_status_num.at(inputs_lines_assign[2][i]).source_lines.push_back(output_name);
                bcp_que.push(inputs_lines_assign[2][i]);
            }
            return true;
        }
        else if (output_assign == -1 && inputs_lines_assign[0].size() == number_lineOfGate)
        {
            lines_status_num.at(output_name).assign = 1;
            lines_status_num.at(output_name).level = level;
            lines_status_num.at(output_name).source_lines = inputs_lines_assign[0];
            bcp_que.push(output_name);
            return true;
        }
        else if (output_assign == -1 && inputs_lines_assign[1].size() > 0)
        {
            lines_status_num.at(output_name).assign = 0;
            lines_status_num.at(output_name).level = level;
            lines_status_num.at(output_name).source_lines.push_back(inputs_lines_assign[1][0]);
            bcp_que.push(output_name);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Xor: // two inputs
    {
        if ((output_assign == 0 && inputs_lines_assign[0].size() > 0 && inputs_lines_assign[1].size() > 0) ||
            (output_assign == 1 && (inputs_lines_assign[0].size() == 2 || inputs_lines_assign[1].size() == 2)))
        {
            conflict_line.push_back(output_name);
            conflict_line.push_back(current_gate->get_inputs()[0]->num_name);
            conflict_line.push_back(current_gate->get_inputs()[1]->num_name);
            return false;
        }
        else if (output_assign != -1 && inputs_lines_assign[2].size() == 1)
        {
            if (inputs_lines_assign[0].size() > 0)
            {
                lines_status_num.at(inputs_lines_assign[2][0]).assign = 0;
                lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(inputs_lines_assign[0][0]);
            }
            else // inputs_lines_assign[1].size() > 0
            {
                lines_status_num.at(inputs_lines_assign[2][0]).assign = 1;
                lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(inputs_lines_assign[1][0]);
            }
            lines_status_num.at(inputs_lines_assign[2][0]).level = level;
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(output_name);
            bcp_que.push(inputs_lines_assign[2][0]);
            return true;
        }
        else if (output_assign == 1 && inputs_lines_assign[2].size() == 1)
        {
            if (inputs_lines_assign[0].size() > 0)
            {
                lines_status_num.at(inputs_lines_assign[2][0]).assign = 1;
                lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(inputs_lines_assign[0][0]);
            }
            else // (inputs_lines_assign[1].size() > 0)
            {
                lines_status_num.at(inputs_lines_assign[2][0]).assign = 0;
                lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(inputs_lines_assign[1][0]);
            }
            lines_status_num.at(inputs_lines_assign[2][0]).level = level;
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(output_name);
            bcp_que.push(inputs_lines_assign[2][0]);
            return true;
        }
        else if (output_assign == -1 && inputs_lines_assign[0].size() > 0 && inputs_lines_assign[1].size() > 0)
        {
            lines_status_num.at(output_name).assign = 1;
            lines_status_num.at(output_name).level = level;
            lines_status_num.at(output_name).source_lines.push_back(inputs_lines_assign[0][0]);
            lines_status_num.at(output_name).source_lines.push_back(inputs_lines_assign[1][0]);
            bcp_que.push(output_name);
            return true;
        }
        else if (output_assign == -1 && (inputs_lines_assign[0].size() == 2 || inputs_lines_assign[1].size() == 2))
        {
            lines_status_num.at(output_name).assign = 0;
            lines_status_num.at(output_name).level = level;
            for (auto input_name : current_gate->get_inputs())
            {
                lines_status_num.at(output_name).source_lines.push_back(input_name->num_name);
            }
            bcp_que.push(output_name);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Xnor: // two inputs
    {
        if ((output_assign == 1 && inputs_lines_assign[0].size() > 0 && inputs_lines_assign[1].size() > 0) ||
            (output_assign == 0 && (inputs_lines_assign[0].size() == 2 || inputs_lines_assign[1].size() == 2)))
        {
            conflict_line.push_back(output_name);
            conflict_line.push_back(current_gate->get_inputs()[0]->num_name);
            conflict_line.push_back(current_gate->get_inputs()[1]->num_name);
            return false;
        }
        else if (output_assign == 1 && inputs_lines_assign[2].size() == 1)
        {
            if (inputs_lines_assign[0].size() > 0)
            {
                lines_status_num.at(inputs_lines_assign[2][0]).assign = 0;
                lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(inputs_lines_assign[0][0]);
            }
            else // (inputs_lines_assign[1].size() > 0)
            {
                lines_status_num.at(inputs_lines_assign[2][0]).assign = 1;
                lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(inputs_lines_assign[1][0]);
            }
            lines_status_num.at(inputs_lines_assign[2][0]).level = level;
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(output_name);
            bcp_que.push(inputs_lines_assign[2][0]);
            return true;
        }
        else if (output_assign == 0 && inputs_lines_assign[2].size() == 1)
        {
            if (inputs_lines_assign[0].size() > 0)
            {
                lines_status_num.at(inputs_lines_assign[2][0]).assign = 1;
                lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(inputs_lines_assign[0][0]);
            }
            else // (inputs_lines_assign[1].size() > 0)
            {
                lines_status_num.at(inputs_lines_assign[2][0]).assign = 0;
                lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(inputs_lines_assign[1][0]);
            }
            lines_status_num.at(inputs_lines_assign[2][0]).level = level;
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(output_name);
            bcp_que.push(inputs_lines_assign[2][0]);
            return true;
        }
        else if (output_assign == -1 && inputs_lines_assign[0].size() > 0 && inputs_lines_assign[1].size() > 0)
        {
            lines_status_num.at(output_name).assign = 0;
            lines_status_num.at(output_name).level = level;
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(inputs_lines_assign[0][0]);
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(inputs_lines_assign[1][0]);
            bcp_que.push(output_name);
            return true;
        }
        else if (output_assign == -1 && (inputs_lines_assign[0].size() == 2 || inputs_lines_assign[1].size() == 2))
        {
            lines_status_num.at(output_name).assign = 1;
            lines_status_num.at(output_name).level = level;
            for (auto input_name : current_gate->get_inputs())
            {
                lines_status_num.at(output_name).source_lines.push_back(input_name->num_name);
            }
            bcp_que.push(output_name);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Not:
    {
        if ((inputs_lines_assign[0].size() == 1 && output_assign == 0) || (inputs_lines_assign[1].size() == 1 && output_assign == 1))
        {
            conflict_line.push_back(output_name);
            conflict_line.push_back(current_gate->get_inputs()[0]->num_name);
            return false;
        }
        else if (output_assign == -1)
        {
            lines_status_num.at(output_name).assign = 1 - lines_status_num.at(current_gate->get_inputs()[0]->num_name).assign;
            lines_status_num.at(output_name).level = level;
            lines_status_num.at(output_name).source_lines.push_back(current_gate->get_inputs()[0]->num_name);
            bcp_que.push(output_name);
            return true;
        }
        else if (inputs_lines_assign[2].size() == 1)
        {
            lines_status_num.at(inputs_lines_assign[2][0]).assign = 1 - output_assign;
            lines_status_num.at(inputs_lines_assign[2][0]).level = level;
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(output_name);
            bcp_que.push(inputs_lines_assign[2][0]);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Buff:
    {
        if ((inputs_lines_assign[0].size() == 1 && output_assign == 1) || (inputs_lines_assign[1].size() == 1 && output_assign == 0))
        {
            conflict_line.push_back(output_name);
            conflict_line.push_back(current_gate->get_inputs()[0]->num_name);
            return false;
        }
        else if (output_assign == -1)
        {
            lines_status_num.at(output_name).assign = lines_status_num.at(current_gate->get_inputs()[0]->num_name).assign;
            lines_status_num.at(output_name).level = level;
            lines_status_num.at(output_name).source_lines.push_back(current_gate->get_inputs()[0]->num_name);
            bcp_que.push(output_name);
            return true;
        }
        else if (inputs_lines_assign[2].size() == 1)
        {
            lines_status_num.at(inputs_lines_assign[2][0]).assign = output_assign;
            lines_status_num.at(inputs_lines_assign[2][0]).level = level;
            lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(output_name);
            bcp_que.push(inputs_lines_assign[2][0]);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Undefined:
    {
        return true;
    }
    default:
        return true;
    }
    }
}
bool solver::LearntGateReason(Gate *current_gate, std::queue<int> &bcp_que, int decision_line)
{
    number++;
    int number_lineOfGate = current_gate->get_inputs().size();
    std::vector<std::vector<std::pair<int, int>>> inputs_lines_assign(3); // key---line's name; value---line's polarty
    // traverse, and change line's polarity
    for (int i = 0; i < current_gate->get_inputs().size(); i++)
    {
        int input = current_gate->inputs()[i]->num_name;
        int convert = lines_status_num.at(input).assign;
        if (lines_status_num.at(input).assign != -1 && current_gate->get_inputs_polarity()[i] == 0)
        {
            inputs_lines_assign[1 - convert].push_back(std::make_pair(input, current_gate->get_inputs_polarity()[i]));
        }
        else if (lines_status_num.at(input).assign != -1 && current_gate->get_inputs_polarity()[i] == 1)
        {
            inputs_lines_assign[convert].push_back(std::make_pair(input, current_gate->get_inputs_polarity()[i]));
        }
        else
            inputs_lines_assign[2].push_back(std::make_pair(input, current_gate->get_inputs_polarity()[i]));
    }

    // deal with learnt gate ,it's output is fix: output.assign = 1
    if (inputs_lines_assign[0].size() == number_lineOfGate)
    {
        for (auto temp : inputs_lines_assign[0])
            conflict_line.push_back(temp.first);
        return false; // conflict
    }
    else if (inputs_lines_assign[0].size() == number_lineOfGate - 1 && inputs_lines_assign[2].size() == 1)
    {
        lines_status_num.at(inputs_lines_assign[2][0].first).assign = inputs_lines_assign[2][0].second;
        lines_status_num.at(inputs_lines_assign[2][0].first).level = lines_status_num.at(decision_line).level;
        for (auto temp : inputs_lines_assign[0])
            lines_status_num.at(inputs_lines_assign[2][0].first).source_lines.push_back(temp.first);
        bcp_que.push(inputs_lines_assign[2][0].first);
        return true;
    }
    else
    {
        return true;
    }
}

void solver::struct_implication(Gate &gate, int gate_index)
{
    // direct implication; all types gate's destination dir_imp0 and dir_imp1 is identical.xor and xnor don'd have direct implication
    for (const auto &temp : gate.get_output()->destination_gates) // for dir_imp0 and dir_imp1 with destination gates
    {
        switch (temp->get_type())
        {
        case Gate::Type::And: // input watch_value is 1,output watch_value is 0
            gate.get_dir_imp0().push_back(std::make_pair(temp->get_output()->num_name, 0));
            break;
        case Gate::Type::Nand: // input watch_value is 1,output watch_value is 1
            gate.get_dir_imp0().push_back(std::make_pair(temp->get_output()->num_name, 1));
            break;
        case Gate::Type::Or: // input watch_value is 0,output watch_value is 1
            gate.get_dir_imp1().push_back(std::make_pair(temp->get_output()->num_name, 1));
            break;
        case Gate::Type::Nor: // input watch_value is 0,output watch_value is 0
            gate.get_dir_imp1().push_back(std::make_pair(temp->get_output()->num_name, 0));
            break;
        case Gate::Type::Not: // input watch_value is x,output watch_value is -x
            gate.get_dir_imp0().push_back(std::make_pair(temp->get_output()->num_name, 1));
            gate.get_dir_imp1().push_back(std::make_pair(temp->get_output()->num_name, 0));
            break;
        case Gate::Type::Buff: // input watch_value is 0,output watch_value is 0
            gate.get_dir_imp0().push_back(std::make_pair(temp->get_output()->num_name, 0));
            gate.get_dir_imp1().push_back(std::make_pair(temp->get_output()->num_name, 1));
            break;
        default:
            break;
        }
    }
    if (gate.get_type() == Gate::Type::Input)
        ;
    else
    {
        std::pair<int, int> des_output;
        int line_index_output = gate.get_output()->num_name;    // initialize pointer 1
        int line_index_input0 = gate.get_inputs()[0]->num_name; // initialize pointer 2
        switch (gate.get_type())                                // for dir_imp0 and dir_imp1 with source gates
        {
        case Gate::Type::And: // AND, dir_imp0 source gate is NULL
        {
            for (int i = 0; i < gate.get_inputs().size(); i++) // for dir_imp1 with source gates
            {
                des_output = std::make_pair(gate.get_inputs()[i]->num_name, 1);
                gate.get_dir_imp1().push_back(des_output);
            }
            // indirect implication
            watching0[line_index_output].push_back(gate_index); // AND's output watch value is 0
            watching1[line_index_input0].push_back(gate_index); // AND's input watch value is 1
            break;
        }
        case Gate::Type::Nand: // NAND, dir_imp1 source gate is NULL
        {
            for (int i = 0; i < gate.get_inputs().size(); i++) // for dir_imp0 with source gates
            {
                des_output = std::make_pair(gate.get_inputs()[i]->num_name, 1);
                gate.get_dir_imp0().push_back(des_output);
            }
            // indirect implication
            watching1[line_index_output].push_back(gate_index); // NAND's output watch value is 1
            watching0[line_index_input0].push_back(gate_index); // NAND's input watch value is 0
            break;
        }
        case Gate::Type::Or: // OR, dir_imp1 source gate is NULL
        {
            for (int i = 0; i < gate.get_inputs().size(); i++) // for dir_imp0 with source gates
            {
                des_output = std::make_pair(gate.get_inputs()[i]->num_name, 0);
                gate.get_dir_imp0().push_back(des_output);
            }
            // indirect implication
            watching1[line_index_output].push_back(gate_index); // OR's output watch value is 1
            watching0[line_index_input0].push_back(gate_index); // OR's input watch value is 0
            break;
        }
        case Gate::Type::Nor: // NOR, dir_imp0 source gate is NULL
        {
            for (int i = 0; i < gate.get_inputs().size(); i++) // for dir_imp1 with source gates
            {
                des_output = std::make_pair(gate.get_inputs()[i]->num_name, 0);
                gate.get_dir_imp1().push_back(des_output);
            }
            // indirect implication
            watching0[line_index_output].push_back(gate_index); // NOR's output watch value is 0
            watching0[line_index_input0].push_back(gate_index); // NOR's input watch value is 0
            break;
        }
        case Gate::Type::Not: // NOT, dir_imp0 source gate is NULL
        {
            gate.get_dir_imp0().push_back(std::make_pair(gate.get_inputs()[0]->num_name, 1));
            gate.get_dir_imp1().push_back(std::make_pair(gate.get_inputs()[0]->num_name, 0));
            break;
        }
        case Gate::Type::Buff: // Buff, dir_imp0 source gate is NULL
        {
            gate.get_dir_imp0().push_back(std::make_pair(gate.get_inputs()[0]->num_name, 0));
            gate.get_dir_imp1().push_back(std::make_pair(gate.get_inputs()[0]->num_name, 1));
            break;
        }
        default: // for xor and xnor,they don't have direct implication,but indirect implication is identical,following:
        {
            watching0[line_index_output].push_back(gate_index); // xor's output watch value is 0 and 1
            watching1[line_index_output].push_back(gate_index); // xor's output watch value is 0 and 1
            watching0[line_index_input0].push_back(gate_index); // xnor's input watch value is 0 and 1
            watching1[line_index_input0].push_back(gate_index); // xnor's input watch value is 0 and 1
            break;
        }
        }
    }
}