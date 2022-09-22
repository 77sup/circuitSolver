#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include <vector>
#include <queue>
int number = 0;
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
        gate.get_pointers() = des_output;
        switch (gate.get_type()) // for dir_imp0 and dir_imp1 with source gates
        {
        case Gate::Type::And: // AND, dir_imp0 source gate is NULL
        {
            for (int i = 0; i < gate.get_inputs().size(); i++) // for dir_imp1 with source gates
            {
                des_output = std::make_pair(gate.get_inputs()[i]->num_name, 1);
                gate.get_dir_imp1().push_back(des_output);
            }
            // indirect implication
            watching_list[0][line_index_output].push_back(gate_index); // AND's output watch value is 0
            watching_list[1][line_index_input0].push_back(gate_index); // AND's input watch value is 1
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
            watching_list[1][line_index_output].push_back(gate_index); // NAND's output watch value is 1
            watching_list[1][line_index_input0].push_back(gate_index); // NAND's input watch value is 1
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
            watching_list[1][line_index_output].push_back(gate_index); // OR's output watch value is 1
            watching_list[0][line_index_input0].push_back(gate_index); // OR's input watch value is 0
            break;
        }
        case Gate::Type::Nor: // NOR, dir_imp0 source gate is NULL
        {
            for (int i = 0; i < gate.get_inputs().size(); i++) // for dir_imp1 with source gates
            {
                des_output = std::make_pair(gate.get_inputs()[i]->num_name, 0);
                gate.get_dir_imp1().push_back(des_output);
    
            // indirect implication
            watching_list[0][line_index_output].push_back(gate_index); // NOR's output watch value is 0
            watching_list[0][line_index_input0].push_back(gate_index); // NOR's input watch value is 0
            break;
            }
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
            watching_list[0][line_index_output].push_back(gate_index); // xor's output watch value is 0 and 1
            watching_list[1][line_index_output].push_back(gate_index); // xor's output watch value is 0 and 1
            watching_list[0][line_index_input0].push_back(gate_index); // xnor's input watch value is 0 and 1
            watching_list[1][line_index_input0].push_back(gate_index); // xnor's input watch value is 0 and 1
            break;
        }
        }
    }
}

//返回0：直接蕴含冲突  返回1：单个门直接蕴含正常
bool solver::single_gate_dir(Gate *current_gate, std::vector<int> &bcp_vec, int decision_line)
{
    int output_name = current_gate->get_output()->num_name;
    int this_level = lines_status_num.at(decision_line).level;
    int assign = lines_status_num.at(output_name).assign;
    std::vector<std::pair<int, int>> *dir;
    if (assign == 1)
        dir = &current_gate->get_dir_imp1();
    else
        dir = &current_gate->get_dir_imp0();
    for (const auto &temp : (*dir))
    {
        if (lines_status_num.at(temp.first).assign == 2)
        {
            lines_status_num.at(temp.first).assign = temp.second;
            lines_status_num.at(temp.first).level = this_level;
            //直接蕴含的source就是该门的输出线
            lines_status_num.at(temp.first).source_lines.push_back(output_name);
            bcp_vec.push_back(temp.first);
        }
        else if (lines_status_num.at(temp.first).assign == temp.second)
            continue;
        else //直接蕴含的冲突定义为同一变量被赋值为不同的值
        {
            conflict_line = lines_status_num.at(temp.first).source_lines;
            conflict_line.push_back(output_name);
            return 0;
        }
    }
    return 1;
}
//如果推出冲突 return 0；如果门什么都推不出来 return 1；如果门能推出一根线的赋值 return 2；
int solver::single_gate_indir(Gate *current_gate, std::vector<int> &bcp_vec, int decision_line, int indir_idx,int idx)
{
    assert(current_gate);
    int inputs_watch;
    int output_watch;
    int this_level = lines_status_num.at(decision_line).level;
    switch (current_gate->get_type())
    {
    case Gate::Type::And:
        inputs_watch = 1;
        output_watch = 0;
        break;
    case Gate::Type::Nand:
        inputs_watch = 1;
        output_watch = 1;
        break;
    case Gate::Type::Or:
        inputs_watch = 0;
        output_watch = 1;
        break;
    case Gate::Type::Nor:
        inputs_watch = 0;
        output_watch = 0;
        break;
    default: // xor xnor
        return x_gate_indir(current_gate, bcp_vec, decision_line , indir_idx, idx);
        break;
    }
    //搜集gate的详细信息
    int number_inputs = current_gate->get_inputs().size();
    std::vector<std::vector<int>> inputs_lines_assign(3);
    int unwatch_value_number = 0;
    int assign;
    for (auto temp : current_gate->get_inputs())
    {
        //=监视值 !=监视值 为x
        assign = lines_status_num.at(temp->num_name).assign;
        if (assign == inputs_watch)
        {
            unwatch_value_number++;
            inputs_lines_assign[0].push_back(temp->num_name);
        }
        else if (assign == 2)
            inputs_lines_assign[2].push_back(temp->num_name);
        else
            inputs_lines_assign[1].push_back(temp->num_name);
    }
    int output_name = current_gate->get_output()->num_name;
    int opt_is_watching = 0; /// opt=output
    if (lines_status_num.at(output_name).assign == output_watch)
    {
        opt_is_watching = 1;
        unwatch_value_number++;
    }
    // 1:所有线的赋值都为门的非监视值，则发生间接蕴含冲突，该门的监视指针不变
    if (unwatch_value_number == number_inputs + 1)
    {
        conflict_line.push_back(output_name);
        for (auto temp : current_gate->get_inputs())
            conflict_line.push_back(temp->num_name);
        return 0;
    }
    // 2:可以产生间接蕴含或正常退出
    if (inputs_lines_assign[0].size() + opt_is_watching == number_inputs)
    {
        if (opt_is_watching)
        {
            if (inputs_lines_assign[1].size() == 1)
            {
                return 0;
            }
            if (inputs_lines_assign[2].size() == 1)
            {
                lines_status_num.at(inputs_lines_assign[2][0]).assign = 1 - inputs_watch;
                lines_status_num.at(inputs_lines_assign[2][0]).level = this_level;
                lines_status_num.at(inputs_lines_assign[2][0]).source_lines = inputs_lines_assign[0];
                lines_status_num.at(inputs_lines_assign[2][0]).source_lines.push_back(output_name);
                bcp_vec.push_back(inputs_lines_assign[2][0]);
                return 2;
            }
        }
        else // opt_is_watching==0
        {
            if (lines_status_num.at(output_name).assign == 2)
            {
                lines_status_num.at(output_name).assign = 1 - output_watch;
                lines_status_num.at(output_name).level = this_level;
                lines_status_num.at(output_name).source_lines = inputs_lines_assign[0];
                bcp_vec.push_back(inputs_lines_assign[2][0]);
                return 2;
            }
            else
            {
                return 1;
            }
        }
    }
    // 3：
    int flag1 = int(lines_status_num.at(output_name).assign == 2);
    if (inputs_lines_assign[2].size() + flag1 >= 2)
    {
        std::vector<int> need_change_pointer;
        if (lines_status_num.at(current_gate->get_pointers().first).assign != 2)
            need_change_pointer.push_back(current_gate->get_pointers().first);
        if (lines_status_num.at(current_gate->get_pointers().first).assign != 2)
            need_change_pointer.push_back(current_gate->get_pointers().second);
        int iterator = 0;  //need_change_pointer's iterator
        if (flag1 == 1)  //output is unassigned
        {
            need_change_pointer[0] = output_name;
            int watch_name = bcp_vec[indir_idx];
            int assign = lines_status_num.at(watch_name).assign;  //watch value
            int gate_idx = watching_list[assign][watch_name][idx];
            watching_list[assign][watch_name].erase(watching_list[assign][watch_name].begin() + idx);
            watching_list[output_watch][need_change_pointer[0]].push_back(gate_idx);
            iterator = 1;
        }
        for (;iterator < need_change_pointer.size(); iterator++)
        {
            need_change_pointer[iterator] = inputs_lines_assign[2].back();
            int watch_name = bcp_vec[indir_idx];
            int assign = lines_status_num.at(watch_name).assign;  //watch value
            int gate_idx = watching_list[assign][watch_name][idx];
            watching_list[assign][watch_name].erase(watching_list[assign][watch_name].begin() + idx);
            watching_list[output_watch][need_change_pointer[0]].push_back(gate_idx);
            inputs_lines_assign[2].pop_back();
        }
        return 1;
    }
    return 1;
}
int solver::x_gate_indir(Gate *current_gate, std::vector<int> &bcp_vec, int decision_line , int watch_name_idx, int gate_idx)
{
    assert(current_gate);
    std::vector<std::pair<int,int>> x_states(3);
    x_states[0] = std::make_pair(current_gate->output()->num_name,lines_status_num.at(current_gate->output()->num_name).assign);
    x_states[1] = std::make_pair(current_gate->inputs()[0]->num_name,lines_status_num.at(current_gate->inputs()[0]->num_name).assign);
    x_states[2] = std::make_pair(current_gate->inputs()[1]->num_name,lines_status_num.at(current_gate->inputs()[1]->num_name).assign);
    std::vector<int> unassigned_name;
    std::vector<int> assigned_name;
    for(const auto &temp : x_states)
    {
        if(temp.second == 2) 
            unassigned_name.push_back(temp.first);
        else 
            assigned_name.push_back(temp.first);
    }
    //=1:xor   =0:xnor
    int gate_type = int(current_gate->get_type() == Gate::Type::Xor);
    //3根线全部有赋值 冲突或满足 不改变监视指针
    if(assigned_name.size() == 3)
    {
        int flag = x_states[0].second + x_states[1].second + x_states[2].second;
        if(flag % 2 != gate_type) 
            return 1;
        else 
            for(const auto& temp : x_states)
                conflict_line.push_back(temp.first);
        return 0;
    }
    //2根线有赋值 得到间接蕴含 不改变监视指针
    else if(assigned_name.size() == 2)
    {
        int temp = lines_status_num.at(assigned_name[0]).assign + lines_status_num.at(assigned_name[1]).assign;
        lines_status_num.at(unassigned_name[0]).assign = int(temp % 2 == gate_type);
        lines_status_num.at(unassigned_name[0]).level = lines_status_num.at(decision_line).level;
        lines_status_num.at(unassigned_name[0]).source_lines = assigned_name;
        return 2;
    }
    else //only one line is assigned, change watching pointer
    {
        //we need to know: if only one line is assigned,this line must a watching pointer
        //we want to do: 
        //1:detele the information of this watching pointer in watching list
        int this_assign =  lines_status_num.at(assigned_name[0]).assign;
        int this_gate = watching_list[this_assign][assigned_name[0]][gate_idx];
        watching_list[this_assign][assigned_name[0]].erase(watching_list[this_assign][assigned_name[0]].begin() + gate_idx);
        auto iterator = std::find(watching_list[1-this_assign][assigned_name[0]].begin(),watching_list[1-this_assign][assigned_name[0]].begin(),this_gate);
        watching_list[1-this_assign][assigned_name[0]].erase(iterator);
        //2:change gate's watching pointer  
        int other_unassign;
        if(unassigned_name[0] == current_gate->get_pointers().first || unassigned_name[0] == current_gate->get_pointers().second)
            other_unassign = 1;
        else 
            other_unassign = 0;
        
        int new_watching_name = unassigned_name[other_unassign];
        if(current_gate->get_pointers().first == assigned_name[0])
            current_gate->get_pointers().first == new_watching_name;
        else
            current_gate->get_pointers().second == new_watching_name;
        //3:change watching pointer and add xiangguan information to watching list
        watching_list[this_assign][new_watching_name].push_back(this_gate);
        watching_list[1-this_assign][assigned_name[0]].push_back(this_gate);
        return 1;
    }
    return 1;
}