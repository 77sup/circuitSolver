#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include "../include/cnf.h"
#include <vector>
#include <queue>

// design for multiple inputs gates
bool solver::SingleGateReasonBoost(Gate *current_gate, std::queue<int> &bcp_que, int reason_line_name, int origin_decision)
{
    Gate::Type GateType = current_gate->get_type();
    std::vector<std::pair<int, int>> all_lines_current_gate;
    for (int i = 0; i < current_gate->get_inputs().size(); i++)
    {
        int input = current_gate->inputs()[i]->num_name;
        all_lines_current_gate.push_back(std::make_pair(input, lines_status_num.at(input).assign));
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
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                //std::cout << all_lines_current_gate[i].first << " conflict:  " << all_lines_current_gate[i].second<<
                //" source: "<<lines_status_num.at(all_lines_current_gate[i].first).source<< std::endl;
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            //std::cout<<"------And-----" << std::endl;
            return false; // conflict
        }
        else if (output_line_status == 0 && input_line_status[2] == 1 && input_line_status[1] == number_lineOfGate - 2)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                    lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
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
                    lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if (output_line_status == -1 && input_line_status[0] > 0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (output_line_status == -1 && input_line_status[1] == number_lineOfGate - 1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else
        {
            lines_status_num.at(origin_decision).source=origin_decision;
            return true; // nothing can be reasoned
        }
    }
    case Gate::Type::Nand:
    {
        if ((output_line_status == 1 && input_line_status[1] == number_lineOfGate - 1) || (output_line_status == 0 && input_line_status[0] > 0))
        {
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                //std::cout << all_lines_current_gate[i].first << " conflict:  " << all_lines_current_gate[i].second<<
                //" source: "<<lines_status_num.at(all_lines_current_gate[i].first).source<<std::endl;
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            //std::cout<<"------Nand-----" << std::endl;
            return false; // conflict
        }
        else if (output_line_status == 1 && input_line_status[2] == 1 && input_line_status[1] == number_lineOfGate - 2)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                    lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
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
                    lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if (output_line_status == -1 && input_line_status[0] > 0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (output_line_status == -1 && input_line_status[1] == number_lineOfGate - 1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else
        {
             lines_status_num.at(origin_decision).source=origin_decision;
            return true; // nothing can be reasoned
        }
    case Gate::Type::Or:
    {
        
        if ((output_line_status == 0 && input_line_status[1] > 0) ||
            (output_line_status == 1 && input_line_status[0] == number_lineOfGate - 1))
        {
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                //std::cout << all_lines_current_gate[i].first << " conflict:  " << all_lines_current_gate[i].second<<
                //" source: "<<lines_status_num.at(all_lines_current_gate[i].first).source<< std::endl;
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            //std::cout<<"------Or-----" << std::endl;
            return false; // conflict
        }
        else if (output_line_status == 0 && input_line_status[2] > 0)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                     lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
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
                    lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if (output_line_status == -1 && input_line_status[0] == number_lineOfGate - 1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (output_line_status == -1 && input_line_status[1] > 0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else
        {
            lines_status_num.at(origin_decision).source=origin_decision;
            return true;
        }
    }
    case Gate::Type::Nor:
    {
        if ((output_line_status == 1 && input_line_status[1] > 0) || (output_line_status == 0 && input_line_status[0] == number_lineOfGate - 1))
        {
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                //std::cout << all_lines_current_gate[i].first << " conflict:  " << all_lines_current_gate[i].second<<
                //" source: "<<lines_status_num.at(all_lines_current_gate[i].first).source<< std::endl;
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            //std::cout<<"------Nor-----" << std::endl;
            return false; // conflict
        }
        else if (output_line_status == 0 && input_line_status[0] == number_lineOfGate - 2 && input_line_status[2] == 1)
        {
            for (int i = 0; i < number_lineOfGate - 1; i++)
            {
                if (all_lines_current_gate[i].second == -1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first).assign = 1;
                    lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
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
                    lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
                    bcp_que.push(all_lines_current_gate[i].first);
                }
            }
            return true;
        }
        else if (output_line_status == -1 && input_line_status[0] == number_lineOfGate - 1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (output_line_status == -1 && input_line_status[1] > 0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else
        {
            lines_status_num.at(origin_decision).source=origin_decision;
            return true;
        }
    }
    case Gate::Type::Xor: // two inputs
    {
        if ((output_line_status == 0 && input_line_status[0] > 0 && input_line_status[1] > 0) ||
            (output_line_status == 1 && (input_line_status[0] == 2 || input_line_status[1] == 2)))
        {
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                 //std::cout << all_lines_current_gate[i].first << " conflict:  " << all_lines_current_gate[i].second<<
                //" source: "<<lines_status_num.at(all_lines_current_gate[i].first).source<< std::endl;
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            //std::cout<<"------Xor-----" << std::endl;
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
                        lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if (input_line_status[1] > 0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first).assign = 1;
                        lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
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
                        lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if (input_line_status[1] > 0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                        lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }
        }
        else if (output_line_status == -1 && input_line_status[0] > 0 && input_line_status[1] > 0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (output_line_status == -1 && (input_line_status[0] == 2 || input_line_status[1] == 2))
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else
        {
            lines_status_num.at(origin_decision).source=origin_decision;
            return true;
        }
    }
    case Gate::Type::Xnor: // two inputs
    {
        if ((output_line_status == 1 && input_line_status[0] > 0 && input_line_status[1] > 0) ||
            (output_line_status == 0 && (input_line_status[0] == 2 || input_line_status[1] == 2)))
        {
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                //std::cout << all_lines_current_gate[i].first << " conflict:  " << all_lines_current_gate[i].second<<
                //" source: "<<lines_status_num.at(all_lines_current_gate[i].first).source<< std::endl;
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            //std::cout<<"------Xnor-----" << std::endl;
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
                        lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if (input_line_status[1] > 0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first).assign = 1;
                        lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
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
                        lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if (input_line_status[1] > 0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first).assign = 0;
                        lines_status_num.at(all_lines_current_gate[i].first).source=origin_decision;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }
        }
        else if (output_line_status == -1 && input_line_status[0] > 0 && input_line_status[1] > 0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (output_line_status == -1 && (input_line_status[0] == 2 || input_line_status[1] == 2))
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else
        {
            lines_status_num.at(origin_decision).source=origin_decision;
            return true;
        }
    }
    case Gate::Type::Not:
    {
        if ((input_line_status[0] == 1 && output_line_status == 0) || (input_line_status[1] == 1 && output_line_status == 1))
        {
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                //std::cout << all_lines_current_gate[i].first << " conflict:  " << all_lines_current_gate[i].second<<
                //" source: "<<lines_status_num.at(all_lines_current_gate[i].first).source<< std::endl;
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            //std::cout<<"------Not-----" << std::endl;
            return false;
        }
        else if (input_line_status[0] == 1 && output_line_status == -1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (input_line_status[1] == 1 && output_line_status == -1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (input_line_status[2] == 1)
        {
            lines_status_num.at(all_lines_current_gate[0].first).assign = 1 - all_lines_current_gate[1].second;
            lines_status_num.at(all_lines_current_gate[0].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[0].first);
            return true;
        }
        else
        {
            lines_status_num.at(origin_decision).source=origin_decision;
            return true;
        }
    }
    case Gate::Type::Buff:
    {
        if ((input_line_status[0] == 1 && output_line_status == 1) || (input_line_status[1] == 1 && output_line_status == 0))
        {
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                 //std::cout << all_lines_current_gate[i].first << " conflict:  " << all_lines_current_gate[i].second<<
                //" source: "<<lines_status_num.at(all_lines_current_gate[i].first).source<< std::endl;
                the_name_of_conflict_line.push_back(all_lines_current_gate[i].first);
            }
            //std::cout<<"------Buff-----" << std::endl;
            return false;
        }
        else if (input_line_status[0] == 1 && output_line_status == -1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 0;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (input_line_status[1] == 1 && output_line_status == -1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).assign = 1;
            lines_status_num.at(all_lines_current_gate[number_lineOfGate - 1].first).source=origin_decision; 
            bcp_que.push(all_lines_current_gate[number_lineOfGate - 1].first);
            return true;
        }
        else if (input_line_status[2] == 1)
        {
            lines_status_num.at(all_lines_current_gate[0].first).assign = all_lines_current_gate[1].second;
            lines_status_num.at(all_lines_current_gate[0].first).source=origin_decision;
            bcp_que.push(all_lines_current_gate[0].first);
            return true;
        }
        else
        {
            lines_status_num.at(origin_decision).source=origin_decision;
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