#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include <vector>
#include <queue>
int number = 0;
void solver::struct_implication(Gate &gate, int gate_index) {
  // direct implication; all types gate's destination dir_imp0 and dir_imp1 is identical.xor and xnor don'd have direct implication
  for (const auto &temp : gate.get_output()->destination_gates) // for dir_imp0 and dir_imp1 with destination gates
  {
    switch (temp->get_type()) {
    case Gate::Type::And: // input watch_value is 1,output watch_value is 0
      gate.get_dir_imp0().push_back(std::make_pair(temp->get_output()->num_name, 0));
      ls.at(gate.get_output()->num_name).weight +=5;
      break;
    case Gate::Type::Nand: // input watch_value is 1,output watch_value is 1
      gate.get_dir_imp0().push_back(std::make_pair(temp->get_output()->num_name, 1));
      ls.at(gate.get_output()->num_name).weight +=5;
      break;
    case Gate::Type::Or: // input watch_value is 0,output watch_value is 1
      gate.get_dir_imp1().push_back(std::make_pair(temp->get_output()->num_name, 1));
      ls.at(gate.get_output()->num_name).weight +=5;
      break;
    case Gate::Type::Nor: // input watch_value is 0,output watch_value is 0
      gate.get_dir_imp1().push_back(std::make_pair(temp->get_output()->num_name, 0));
      ls.at(gate.get_output()->num_name).weight +=5;
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
  if (gate.get_type() != Gate::Type::Input) 
  {
    std::pair<int, int> des_output;
    int line_index_output = gate.get_output()->num_name;    // initialize pointer 1
    int line_index_input0 = gate.get_inputs()[0]->num_name; // initialize pointer 2
    gate.get_pointers() = std::make_pair(line_index_output, line_index_input0);
    switch (gate.get_type()) // for dir_imp0 and dir_imp1 with source gates
    {
    case Gate::Type::And: // AND, dir_imp0 source gate is NULL
    {
      for (int i = 0; i < gate.get_inputs().size(); i++) // for dir_imp1 with source gates
      {
        des_output = std::make_pair(gate.get_inputs()[i]->num_name, 1);
        gate.get_dir_imp1().push_back(des_output);
        ls.at(gate.get_output()->num_name).weight +=5;
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
        ls.at(gate.get_output()->num_name).weight +=5;
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
        ls.at(gate.get_output()->num_name).weight +=5;
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
        ls.at(gate.get_output()->num_name).weight +=5;
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
  int this_level = ls.at(decision_line).level;
  int assign = ls.at(output_name).assign;
  std::vector<std::pair<int, int>> *dir;
  if (assign == 1)
    dir = &current_gate->get_dir_imp1();
  else
    dir = &current_gate->get_dir_imp0();
  for (const auto &temp : (*dir)) 
  {   //update ls,with newly direct implication value
    if (ls.at(temp.first).assign == 2) 
    {
      ls.at(temp.first).assign = temp.second;
      ls.at(temp.first).level = this_level;
      //direct implication's source is output of this gate
      ls.at(temp.first).source_lines.push_back(output_name);
      bcp_vec.push_back(temp.first);
      continue;
    } else if (ls.at(temp.first).assign == temp.second)  //before have been assigned
      continue;
    else //occur direct implication confilict,means same line with two different assignment
    {
      conflict_line = ls.at(temp.first).source_lines;
      conflict_line.push_back(output_name);
      return 0;
    }
  }
  return 1;
}
//如果推出冲突 return 0；如果门什么都推不出来 return 1；如果门能推出一根线的赋值 return 2；
int solver::single_gate_indir(CircuitGraph &graph, Gate *current_gate, std::vector<int> &bcp_vec, int decision_line, int bcp_idx, int list_idx,int i) {
  number++;
  int inputs_watch;
  int output_watch;
  int this_level = ls.at(decision_line).level;
  switch (current_gate->get_type()) {
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
    //std::cout<<"xor________________xnor"<<std::endl;
    return x_gate_indir(current_gate, bcp_vec, decision_line, bcp_idx, list_idx);
  }
  //搜集gate的详细信息: 0: =监视值  1: 为x
  int f = current_gate->get_pointers().first;      //pointer1's name
  int s = current_gate->get_pointers().second;     //pointer2's name
  //std::cout<<"f: "<<f<<" s: "<<s<<std::endl;
  int number_lines = current_gate->get_inputs().size() + 1;
  // lines_assign[0]:存放当前gate中赋值==监视值的line's name
  std::vector<std::vector<int>> lines_assign(2);
  //collect gate's lines information
  for (auto temp : current_gate->get_inputs()) 
  {
    if (ls.at(temp->num_name).assign == inputs_watch) 
      lines_assign[0].push_back(temp->num_name);
    else if (ls.at(temp->num_name).assign == 2)
      lines_assign[1].push_back(temp->num_name);
    else
      return 1;
  }
  int output_name = current_gate->get_output()->num_name;
  if (ls.at(output_name).assign == output_watch) 
    lines_assign[0].push_back(output_name);
  else if(ls.at(output_name).assign == 2)
    lines_assign[1].push_back(output_name);
  else
    return 1;
  // 以下情况不会出现某根线的赋值为非监视值
  // 1:所有线的赋值都为门的监视值，则发生间接蕴含冲突，该门的监视指针不变
  if (lines_assign[0].size() == number_lines) {
    conflict_line = lines_assign[0];
    return 0;
  }
  // 2:可以产生间接蕴含
  if(lines_assign[1].size() == 1)
  {
    int this_assign;
    if(lines_assign[1][0] == output_name)
      this_assign = 1 - output_watch;
    else
      this_assign = 1 - inputs_watch;
    ls.at(lines_assign[1][0]).assign = this_assign;
    ls.at(lines_assign[1][0]).level = this_level;
    ls.at(lines_assign[1][0]).source_lines = lines_assign[0];
    bcp_vec.push_back(lines_assign[1][0]);
    return 2;
  }
  // 3：唯一需要更改pointer的情况 需要修改一个或两个
  if(lines_assign[1].size() > 1)
  {
    //std::cout<<"---------------change pointer--------------"<<std::endl;
    for(unsigned int i = 0; i < lines_assign[1].size(); ++i)
    {
      if(lines_assign[1][i] == f || lines_assign[1][i] == s)
      {
        lines_assign[1].erase(lines_assign[1].begin() + i);
        break;
      }
    }
    //bcp_vec[bcp_idx]所指的线一定要修改
    int this_assign = ls.at(bcp_vec[bcp_idx]).assign;
    int gate_idx = watching_list[this_assign][bcp_vec[bcp_idx]][list_idx];
    watching_list[this_assign][bcp_vec[bcp_idx]].erase(watching_list[this_assign][bcp_vec[bcp_idx]].begin() + list_idx);
    int new_pointer = lines_assign[1].back();
    //std::cout<<"new_pointer: "<<new_pointer<<std::endl;
    //std::cout<<"bcp_vec[bcp_idx]: "<<bcp_vec[bcp_idx]<<std::endl;
    //修改监视指针
    if(bcp_vec[bcp_idx] == f)
      current_gate->get_pointers().first = new_pointer;
    else
      current_gate->get_pointers().second = new_pointer;
    //修改监视列表
    if(new_pointer == output_name) 
      watching_list[output_watch][new_pointer].push_back(gate_idx);
    else
      watching_list[inputs_watch][new_pointer].push_back(gate_idx);
    //另一个监视指针可能要修改
    //std::cout<<"lines_assign[1] size: "<<lines_assign[1].size()<<std::endl;
    if(lines_assign[1].size()<2) return 1;
    lines_assign[1].pop_back();
    new_pointer = lines_assign[1].back();  //update another pointer
    //std::cout<<"lines_assign[1].back(): "<<lines_assign[1].back()<<std::endl;
    //std::cout<<"pointer1 name: "<<f<<std::endl;
    if(ls.at(f).assign != 2)
    {
      auto temp = std::find(watching_list[ls.at(f).assign][f].begin(), watching_list[ls.at(f).assign][f].end(), gate_idx);
      if(temp != watching_list[ls.at(f).assign][f].end())
        watching_list[ls.at(f).assign][f].erase(temp);
      current_gate->get_pointers().first = new_pointer;
      if(new_pointer == output_name) 
        watching_list[output_watch][new_pointer].push_back(gate_idx);
      else
        watching_list[inputs_watch][new_pointer].push_back(gate_idx);
    }
    else if(ls.at(s).assign != 2)
    {
      auto temp = std::find(watching_list[ls.at(s).assign][s].begin(), watching_list[ls.at(s).assign][s].end(), gate_idx);
      if(temp != watching_list[ls.at(s).assign][s].end())
        watching_list[ls.at(s).assign][s].erase(temp);
      current_gate->get_pointers().second = new_pointer;
      //std::cout<<"current_gate->get_pointers().second: "<<current_gate->get_pointers().second<<std::endl;
      if(new_pointer == output_name) 
        watching_list[output_watch][new_pointer].push_back(gate_idx);
      else
        watching_list[inputs_watch][new_pointer].push_back(gate_idx);
    }
  }
  int f1 = current_gate->get_pointers().first;      //pointer1's name
  int s1 = current_gate->get_pointers().second;     //pointer2's name
  //std::cout<<"---------------- f1: "<<f1<<" s1: "<<s1<<std::endl;
  return 1;
}
int solver::x_gate_indir(Gate *current_gate, std::vector<int> &bcp_vec, int decision_line, int watch_name_idx, int list_idx) {
  number++;
  std::vector<std::pair<int, int> > x_states(3);
  x_states[0] = std::make_pair(current_gate->output()->num_name, ls.at(current_gate->output()->num_name).assign);
  x_states[1] = std::make_pair(current_gate->inputs()[0]->num_name, ls.at(current_gate->inputs()[0]->num_name).assign);
  x_states[2] = std::make_pair(current_gate->inputs()[1]->num_name, ls.at(current_gate->inputs()[1]->num_name).assign);
  // std::cout<<"x_gate_indir:"<<std::endl;
  // std::cout<<x_states[0].second<<":"<<x_states[1].second<<":"<<x_states[2].second<<std::endl;
  std::vector<int> unassigned_name;
  std::vector<int> assigned_name;
  for (const auto &temp : x_states) {
    if (temp.second == 2)
      unassigned_name.push_back(temp.first);
    else
      assigned_name.push_back(temp.first);
  }
  //=1:xor   =0:xnor
  int gate_type = int(current_gate->get_type() == Gate::Type::Xor);
  // 3根线全部有赋值 冲突或满足 不改变监视指针
  if (assigned_name.size() == 3) {
    int flag = x_states[0].second + x_states[1].second + x_states[2].second;
    if (flag % 2 != gate_type)
    {
      //std::cout<<"return1"<<std::endl;
      return 1;
    }
    else {
      //std::cout<<"return2"<<std::endl;
      for (const auto &temp : x_states)
        conflict_line.push_back(temp.first);
      return 0;
    }
  }
  // 2根线有赋值 得到间接蕴含 不改变监视指针
  else if (assigned_name.size() == 2) {
    int temp = ls.at(assigned_name[0]).assign + ls.at(assigned_name[1]).assign;
    ls.at(unassigned_name[0]).assign = int(temp % 2 == gate_type);
    ls.at(unassigned_name[0]).level = ls.at(decision_line).level;
    ls.at(unassigned_name[0]).source_lines = assigned_name;
    bcp_vec.push_back(unassigned_name[0]);
    return 2;
  } else // only one line is assigned, change watching pointer
  {
    // we need to know: if only one line is assigned,this line must a watching pointer
    // we want to do:
    // 1:detele the information of this watching pointer in watching list
    int this_assign = ls.at(assigned_name[0]).assign;
    int this_gate = watching_list[this_assign][assigned_name[0]][list_idx];
    watching_list[this_assign][assigned_name[0]].erase(watching_list[this_assign][assigned_name[0]].begin() + list_idx);
    auto iterator = std::find(watching_list[1 - this_assign][assigned_name[0]].begin(), watching_list[1 - this_assign][assigned_name[0]].end(), this_gate);
    if(iterator != watching_list[1 - this_assign][assigned_name[0]].end())
      watching_list[1 - this_assign][assigned_name[0]].erase(iterator);
    // 2:change gate's watching pointer
    int other_unassign;
    if (unassigned_name[0] == current_gate->get_pointers().first || unassigned_name[0] == current_gate->get_pointers().second)
      other_unassign = 1;
    else
      other_unassign = 0;
    int new_watching_name = unassigned_name[other_unassign];
    if (current_gate->get_pointers().first == assigned_name[0])
      current_gate->get_pointers().first = new_watching_name;
    else
      current_gate->get_pointers().second = new_watching_name;
    // 3:change watching pointer and add xiangguan information to watching list
    watching_list[this_assign][new_watching_name].push_back(this_gate);
    watching_list[1 - this_assign][new_watching_name].push_back(this_gate);
    return 1;
  }
  return 1;
}

int solver::learn_gate_indir(Gate *current_gate, std::vector<int> &bcp_vec, int decision_line, int bcp_idx, int list_idx) {
  // gate_type = Or, inputs watch value = 0;
  int inputs_number = current_gate->get_inputs().size();
  std::vector<std::vector<std::pair<int, int> > > lg_inputs_state(3); // pair:first---line's name; second---line's polarty
  //std::cout<<"learn_gate_indir inputs_number: "<<bcp_vec[bcp_idx]<<std::endl;
  for (int i = 0; i < inputs_number; i++) 
  {
    //std::cout<<"current_gate->inputs()[i].name:"<<current_gate->inputs()[i]->num_name<<std::endl;
    int input = current_gate->inputs()[i]->num_name;
    int convert = ls.at(input).assign;
    //std::cout<<"*********convert:"<<convert<<std::endl;
    if (ls.at(input).assign != 2 && current_gate->get_inputs_polarity()[i] == 0) {
      lg_inputs_state[1 - convert].push_back(std::make_pair(input, current_gate->get_inputs_polarity()[i]));
    } else if (ls.at(input).assign != 2 && current_gate->get_inputs_polarity()[i] == 1) {
      lg_inputs_state[convert].push_back(std::make_pair(input, current_gate->get_inputs_polarity()[i]));
    } else
      lg_inputs_state[2].push_back(std::make_pair(input, current_gate->get_inputs_polarity()[i]));
  }
  // 1:all lines are assigned watching value: conflict
  if (lg_inputs_state[0].size() == inputs_number)
  {
    for (const auto &input : lg_inputs_state[0])
      conflict_line.push_back(input.first);
    return 0;
  }
  // 2:the gate can't get indirect or conflict
  if (lg_inputs_state[1].size() > 0)
    return 1;
  // 3:can get indirect
  if (lg_inputs_state[2].size() == 1) 
  {
    int line_name = lg_inputs_state[2][0].first;
    int polarity = lg_inputs_state[2][0].second;
    ls.at(line_name).assign = lg_inputs_state[2][0].second;
    bcp_vec.push_back(line_name);
    ls.at(line_name).level = ls.at(decision_line).level;
    for (const auto &input : lg_inputs_state[0])
      ls.at(line_name).source_lines.push_back(input.first);
    return 2;
  }
  // 4:有不小于两根线未赋值，其余线都为监视值,更换监视指针
  if (lg_inputs_state[2].size() >= 2)
   {
    std::vector<std::pair<int, int> > need_change;
    for (unsigned int i = 0; i < lg_inputs_state[0].size(); ++i) {
      if (lg_inputs_state[0][i].first == current_gate->get_pointers().first || lg_inputs_state[0][i].first == current_gate->get_pointers().second) 
        need_change.push_back(lg_inputs_state[0][i]);
    }
    for (unsigned int i = 0; i < lg_inputs_state[2].size(); ++i) {
      if (lg_inputs_state[2][i].first == current_gate->get_pointers().first || lg_inputs_state[2][i].first == current_gate->get_pointers().second) 
        lg_inputs_state[2].erase(lg_inputs_state[2].begin() + i);
    }
    int this_gate = watching_list[ls.at(bcp_vec[bcp_idx]).assign][bcp_vec[bcp_idx]][list_idx];
    for (unsigned int i = 0; i < need_change.size(); ++i) {
      int this_assign = ls.at(need_change[i].first).assign;
      auto iterator = std::find(watching_list[this_assign][need_change[i].first].begin(), watching_list[this_assign][need_change[i].first].end(), this_gate);
      watching_list[this_assign][need_change[i].first].erase(iterator);
      if (need_change[i].first == current_gate->get_pointers().first)
        current_gate->get_pointers().first = lg_inputs_state[2][i].first;
      else
        current_gate->get_pointers().second = lg_inputs_state[2][i].first;
      watching_list[1 - lg_inputs_state[2][i].second][lg_inputs_state[2][i].first].push_back(this_gate);
    }
    return 1;
  }
  return 1;
}
