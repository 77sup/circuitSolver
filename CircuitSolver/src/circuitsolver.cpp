#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include <vector>
#include <queue>

// design for multiple inputs gates
bool solver::SingleGateReasonBoost(Gate *current_gate, std::queue<int> &bcp_que, int decision_line)
{
    int decision_level = lines_status_num.at(decision_line).level;
    Gate::Type GateType = current_gate->get_type();
    std::vector<std::pair<int, int>> all_lines_current_gate;
    if ((*current_gate).get_is_learnt_gate()) //is learnt gate
    {
        //GateType =Gate::Type::Or;
        //traverse, and change line's polarity
        for (int i = 0; i < current_gate->get_inputs().size(); i++)
        {
            int input = current_gate->inputs()[i]->num_name;
            if (lines_status_num.at(input).assign!=-1 && current_gate->get_inputs_polarity()[i]==0)
            {
                int convert=lines_status_num.at(input).assign;
                lines_status_num.at(input).assign=1-convert;
            }
            all_lines_current_gate.push_back(std::make_pair(input, lines_status_num.at(input).assign));
        }
    }
    else  //nomal gate, is not learnt gate
    {
        for (int i = 0; i < current_gate->get_inputs().size(); i++)
        {
            int input = current_gate->inputs()[i]->num_name;
            all_lines_current_gate.push_back(std::make_pair(input, lines_status_num.at(input).assign));
        }
    }
    all_lines_current_gate.push_back(std::make_pair(current_gate->get_output()->num_name, lines_status_num.at(current_gate->get_output()->num_name).assign));
    int number_lineOfGate = all_lines_current_gate.size();
    // respectively record input line's 0 1 x number
    int input_line_status[3] = {0, 0, 0};
    for (int i = 0; i < number_lineOfGate - 1; i++)
    {
        if (all_lines_current_gate[i].second == 0)
        {
            input_line_status[0]++;
        }
        else if (all_lines_current_gate[i].second == 1)
        {
            input_line_status[1]++;
        }
        else
        {
            input_line_status[2]++;
        }
    }
    int output_line_status = all_lines_current_gate[number_lineOfGate - 1].second;
    switch (GateType)
    {
    case Gate::Type::And:
    {
        if ((output_line_status == 0 && input_line_status[1] == number_lineOfGate - 1) || (output_line_status == 1 && input_line_status[0] > 0))
        {
            if (output_line_status == 0)
            {
                for (int i = 0; i < number_lineOfGate; i++)
                {
                    if (!lines_status_num.at(all_lines_current_gate[i].first).is_fixed_value)
                    {
                        the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
                    }
                }
            }
            else if (output_line_status == 1)
            {
                the_name_of_conflict_line.push_back(current_gate->get_output()->num_name);
                for (int i = 0; i < number_lineOfGate; i++)
                {
                    if (all_lines_current_gate[i].second == 0)
                    {
                        the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
                        break;
                    }
                }
            }
            return false; // conflict
        }
        else if (output_line_status == 0 && input_line_status[2] == 1 && input_line_status[1] == number_lineOfGate - 2)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                    lines_status_num.at(all_lines_current_gate[i].first).level = decision_level;
                    // update source

                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if (output_line_status == 1 && input_line_status[2] > 0)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first).assign = 1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if (output_line_status == -1 && input_line_status[0] > 0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (output_line_status == -1 && input_line_status[1] == number_lineOfGate - 1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else
        {
            return true; // nothing can be reasoned
        }
    }
    case Gate::Type::Nand:
    {
        if ((output_line_status == 1 && input_line_status[1] == number_lineOfGate - 1) || (output_line_status == 0 && input_line_status[0] > 0))
        {
            std::cout << "nand" << std::endl;
            conflict_gate = current_gate;
            for (int i = 0; i < number_lineOfGate; i++)
            {
                if (all_lines_current_gate[i].second != -1)
                {
                    the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
                }
            }
            return false; // conflict
        }
        else if (output_line_status == 1 && input_line_status[2] == 1 && input_line_status[1] == number_lineOfGate - 2)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if (output_line_status == 0 && input_line_status[2] > 0)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first).assign = 1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if (output_line_status == -1 && input_line_status[0] > 0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (output_line_status == -1 && input_line_status[1] == number_lineOfGate - 1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else
        {
            return true; // nothing can be reasoned
        }
    case Gate::Type::Or:
    {

        if ((output_line_status == 0 && input_line_status[1] > 0) ||
            (output_line_status == 1 && input_line_status[0] == number_lineOfGate - 1))
        {
            std::cout << "or" << std::endl;
            conflict_gate = current_gate;
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                // std::cout << all_lines_current_gate[i].first << " conflict:  " << all_lines_current_gate[i].second<<
                //" source: "<<lines_status_num.at(all_lines_current_gate[i].first).source<< std::endl;
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            // std::cout<<"------Or-----" << std::endl;
            return false; // conflict
        }
        else if (output_line_status == 0 && input_line_status[2] > 0)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                    bcp_que.push(all_lines_current_gate[i].first);
                }
            }
            return true;
        }
        else if (output_line_status == 1 && input_line_status[0] == number_lineOfGate - 2 && input_line_status[2] == 1)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first).assign = 1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if (output_line_status == -1 && input_line_status[0] == number_lineOfGate - 1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (output_line_status == -1 && input_line_status[1] > 0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Nor:
    {
        if ((output_line_status == 1 && input_line_status[1] > 0) || (output_line_status == 0 && input_line_status[0] == number_lineOfGate - 1))
        {
            std::cout << "nor" << std::endl;
            conflict_gate = current_gate;
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                // std::cout << all_lines_current_gate[i].first << " conflict:  " << all_lines_current_gate[i].second<<
                //" source: "<<lines_status_num.at(all_lines_current_gate[i].first).source<< std::endl;
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            // std::cout<<"------Nor-----" << std::endl;
            return false; // conflict
        }
        else if (output_line_status == 0 && input_line_status[0] == number_lineOfGate - 2 && input_line_status[2] == 1)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first).assign = 1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if (output_line_status == 1 && input_line_status[2] > 0)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                    bcp_que.push(all_lines_current_gate[i].first);
                }
            }
            return true;
        }
        else if (output_line_status == -1 && input_line_status[0] == number_lineOfGate - 1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (output_line_status == -1 && input_line_status[1] > 0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Xor: // two inputs
    {
        if ((output_line_status == 0 && input_line_status[0] > 0 && input_line_status[1] > 0) ||
            (output_line_status == 1 && (input_line_status[0] == 2 || input_line_status[1] == 2)))
        {
            std::cout << "xor" << std::endl;
            conflict_gate = current_gate;
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                // std::cout << all_lines_current_gate[i].first << " conflict:  " << all_lines_current_gate[i].second<<
                //" source: "<<lines_status_num.at(all_lines_current_gate[i].first).source<< std::endl;
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            // std::cout<<"------Xor-----" << std::endl;
            return false;
        }
        else if (output_line_status == 0 && input_line_status[2] == 1)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    if (input_line_status[0] > 0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if (input_line_status[1] > 0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first).assign = 1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }
        }
        else if (output_line_status == 1 && input_line_status[2] == 1)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    if (input_line_status[0] > 0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first).assign = 1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if (input_line_status[1] > 0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }
        }
        else if (output_line_status == -1 && input_line_status[0] > 0 && input_line_status[1] > 0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (output_line_status == -1 && (input_line_status[0] == 2 || input_line_status[1] == 2))
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Xnor: // two inputs
    {
        if ((output_line_status == 1 && input_line_status[0] > 0 && input_line_status[1] > 0) ||
            (output_line_status == 0 && (input_line_status[0] == 2 || input_line_status[1] == 2)))
        {
            std::cout << "xnor" << std::endl;
            conflict_gate = current_gate;
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                // std::cout << all_lines_current_gate[i].first << " conflict:  " << all_lines_current_gate[i].second<<
                //" source: "<<lines_status_num.at(all_lines_current_gate[i].first).source<< std::endl;
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            // std::cout<<"------Xnor-----" << std::endl;
            return false;
        }
        else if (output_line_status == 1 && input_line_status[2] == 1)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    if (input_line_status[0] > 0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if (input_line_status[1] > 0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first).assign = 1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }
        }
        else if (output_line_status == 0 && input_line_status[2] == 1)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    if (input_line_status[0] > 0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first).assign = 1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if (input_line_status[1] > 0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }
        }
        else if (output_line_status == -1 && input_line_status[0] > 0 && input_line_status[1] > 0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (output_line_status == -1 && (input_line_status[0] == 2 || input_line_status[1] == 2))
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Not:
    {
        if ((input_line_status[0] == 1 && output_line_status == 0) || (input_line_status[1] == 1 && output_line_status == 1))
        {
            std::cout << "not" << std::endl;
            conflict_gate = current_gate;
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            return false;
        }
        else if (input_line_status[0] == 1 && output_line_status == -1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (input_line_status[1] == 1 && output_line_status == -1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (input_line_status[2] == 1)
        {
            lines_status_num.at(all_lines_current_gate[0].first).assign = 1 - all_lines_current_gate[1].second;
            bcp_que.push(all_lines_current_gate[0].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Buff:
    {
        if ((input_line_status[0] == 1 && output_line_status == 1) || (input_line_status[1] == 1 && output_line_status == 0))
        {
            std::cout << "buff" << std::endl;
            conflict_gate = current_gate;
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            return false;
        }
        else if (input_line_status[0] == 1 && output_line_status == -1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (input_line_status[1] == 1 && output_line_status == -1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (input_line_status[2] == 1)
        {
            lines_status_num.at(all_lines_current_gate[0].first).assign = all_lines_current_gate[1].second;
            bcp_que.push(all_lines_current_gate[0].first);
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