#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include <cmath>

solver::solver(CircuitGraph &graph) {
  std::vector<int> noPO_lines_name; // store no-P0s
  std::vector<int> output;          // store PIs
  std::cout << "the number of all lines:";
  std::cout << graph.m_name_to_line.size() << std::endl;
  for (const auto &line : graph.lines()) {
    line_information temp;
    if (line.is_output) {
      temp.level = 0;
      temp.assign = 1;
      temp.source_lines.clear();
    } else {
      temp.weight = compute_wight(graph, line.num_name);
    }
    this->ls.emplace(line.num_name, temp);
  }
  for (unsigned int i = 0; i < graph.lines().size(); ++i) {
    if (graph.lines()[i].is_output)
      output.push_back(graph.lines()[i].num_name);
    else
      noPO_lines_name.push_back(graph.lines()[i].num_name);
  }
  structural_implication_map(graph);
  // according to fan_outs numbers to order(max->min)
  int noPO_lines_name_size = noPO_lines_name.size();
  for (int i = 0; i < noPO_lines_name_size; i++) {
    Line *temp = graph.m_name_to_line.at(noPO_lines_name[i]);
    // put (fanouts>12 || inputs) lines into sort_destination_gates
    if ((temp->destination_gates.size() > 0) || (temp->source->get_type() == Gate::Type::Input))
      sort_destination_gates.push_back(noPO_lines_name[i]);
  }
}

void solver::structural_implication_map(CircuitGraph &graph) {
  // open up space for watching-0 and watching-1 vector,first find max num_name
  int max_num_name = 0;
  for (auto temp : ls) {
    if (temp.first > max_num_name) {
      max_num_name = temp.first;
    }
  }
  watching_list.resize(2);
  watching_list[0].resize(max_num_name + 1);
  watching_list[1].resize(max_num_name + 1);
  for (unsigned int i = 0; i < graph.get_gates().size(); ++i) {
    // struct dir/indir implicaiton,initialize two pointer, decide watch value
    struct_implication(graph.get_gates()[i], i);
  }
}

int solver::compute_wight(const CircuitGraph &grahp, int line_name) {
  Line *line = grahp.m_name_to_line.at(line_name);
  int weight = 0;
  weight = line->destination_gates.size();
  if (line->source)
    weight += line->source->get_inputs().size();
  for (const auto &temp : line->destination_gates) {
    if (temp->get_type() == Gate::Type::Not || temp->get_type() == Gate::Type::Buff) {
      weight = weight + temp->get_output()->destination_gates.size() - 1;
    }
  }
  return weight;
}

// choose a line to assign(decision),according to ordered fan_outs numbers
int solver::FindDecisionTarget() {
  int Target = -1;
  int max_weight = -1;
  for (int i = 0; i < sort_destination_gates.size(); i++) {
    if (ls.at(sort_destination_gates[i]).assign == 2 && ls.at(sort_destination_gates[i]).weight > max_weight) {
      Target = sort_destination_gates[i];
      max_weight = ls.at(sort_destination_gates[i]).weight;
    }
  }
  return Target;
}

int solver::watch_BCP(CircuitGraph &graph, int decision_line) {
  std::cout << "enter_bcp,decision line name:assign     " << decision_line << ":" << ls.at(decision_line).assign << std::endl;
  if (decision_line == -1)
    return 1;
  std::vector<int> bcp_vec;
  bcp_vec.push_back(decision_line);
  int dir_idx = 0;
  int indir_idx = 0;
  while (dir_idx < bcp_vec.size() && indir_idx < bcp_vec.size()) {
    //直接蕴含的所有推理
    while (dir_idx < bcp_vec.size()) {
      Gate *gate = graph.m_name_to_line.at(bcp_vec[dir_idx])->source;
      //single gate direct implication
      int result = single_gate_dir(gate, bcp_vec, decision_line);
      if (result == 0) //单个门的直接蕴含发现冲突，即bcp冲突
        return 0;
      ++dir_idx;
    }
    //间接蕴含的所有推理
    while (indir_idx < bcp_vec.size()) {
      int line_name = bcp_vec[indir_idx];
      int this_assign = ls.at(line_name).assign;
      int flag = 0;
      //遍历以该线当前的赋值作为监视指针和监视值的所有gate
      for (unsigned int i = 0; i < watching_list[this_assign][line_name].size();) 
      {
        int gate_idx = watching_list[this_assign][line_name][i];
        int result;
        if (graph.get_gates()[gate_idx].get_is_learnt_gate())
          result = learn_gate_indir(&graph.get_gates()[gate_idx], bcp_vec, decision_line, indir_idx, i);
        else
          result = single_gate_indir(graph, &graph.get_gates()[gate_idx], bcp_vec, decision_line, indir_idx, i, gate_idx);
        if (result == 0)
          return 0;
        else if (result == 1)
          ++i;
        else 
        {
          flag = 1;
          break;
        }
      }
      if (flag)
        break;
      ++indir_idx;
    }
  }
  return 1;
}
void solver::show_result(CircuitGraph &graph, int dpll_result) {
  if (dpll_result) {
    std::cout << "SAT" << std::endl;
    for (int i = 0; i < graph.get_lines().size(); i++) {
      int line_name = graph.get_lines()[i].num_name;
      if (line_name > 0) {
        std::cout << line_name << "  " << ls.at(line_name).assign << std::endl;
      }
    }
  } else {
    std::cout << "UNSAT" << std::endl;
  }
}

int solver::CDCLsolver(CircuitGraph &graph) {
  int bcp_result = 0;
  for (int i = 0; i < graph.get_outputs().size(); i++) {
    bcp_result = watch_BCP(graph, graph.get_outputs()[i]->num_name);
    if (bcp_result == 0) {
      show_result(graph, bcp_result);
      return 0; // UNSAT,output reason out confilict,directly return unsat
    }
  }
  int decision_line; // initial decision line
  std::vector<int> decision_line_name;
  decision_line_name.push_back(0);
  while (true) {
    decision_line = FindDecisionTarget();
    decision_line_name.push_back(decision_line);
    if (decision_line == -1) {
      show_result(graph, 1);
      return 1; // SAT, output reason out all lines
    }
    // randomly choose left or right node to decide assignment
    Gate *gate = graph.m_name_to_line.at(decision_line)->source;
    ls.at(decision_line).assign = int(gate->get_dir_imp1().size() > gate->get_dir_imp0().size());
    //ls.at(decision_line).assign = rand()%2;
    ls.at(decision_line).level = decision_line_name.size()-1;
    ls.at(decision_line).source_lines.clear();
    while (true) {
      int bcp_result = watch_BCP(graph, decision_line);
      if (bcp_result == 0) // find conflict
      {
        if (ls.at(decision_line).level == 0) // UNSAT
        {
          show_result(graph, 0);
          return 0;
        }
        decision_line = conflict_backtrack(decision_line, graph, decision_line_name);
        if (decision_line == -2) // special use for learnt_gate size==0
        {
          show_result(graph, 0);
          return 0;
        }
      } 
      else // bcp_result=1, bcp normally exit,enter into next decision
      {
        break;
      }
    }
  }
}
// conflict analysis,add learnt clause,and backtrack
int solver::conflict_backtrack(int decision_line, CircuitGraph &graph, std::vector<int> &decision_line_name) {
  std::cout << "enter function: conflict_backtrack" << std::endl;
  int decision_level = ls.at(decision_line).level;
  std::cout << "current decision line:  "<<decision_line<<"   level: " << decision_level << std::endl;
  std::vector<Line *> m_learnt_inputs;
  // learnt gate initialized with origin conflict gate
  int conflict_decision_level = decision_level;
  std::vector<int> learnt_gate(conflict_line);
  std::cout<<"primary learnt_gate size:"<<learnt_gate.size()<<std::endl;
  for (auto temp : learnt_gate) {
    std::cout << temp << "---" << ls.at(temp).level << "  ";
  }
  std::cout << std::endl;
  conflict_line.clear();
  int this_level_count = 0; // number of lines from the same decision level found
  int trace_line = 0;       // line whose previous reason gate will next be used to resolve
  int line_decision_level;
  int second_max_level_line = 0;
  do {
    this_level_count = 0;
    for (int i = 0; i < learnt_gate.size(); i++) 
    {
      line_decision_level = ls.at(learnt_gate[i]).level;
      if (line_decision_level == 0) 
      {
        learnt_gate.erase(learnt_gate.begin() + i);
        i--;
        // if learnt_gate size=0, UNSAT
        if (learnt_gate.size() == 0) 
        {
          return -2;
        }
        continue;
      }
      if (line_decision_level == conflict_decision_level) // a line at the same decision level been found
      {
        this_level_count++;
      }
      // a line at the same decision level but not a decision line
      if (line_decision_level == conflict_decision_level && ls.at(learnt_gate[i]).source_lines.size() != 0) {
        trace_line = learnt_gate[i];
        
      }
    }
    // only one line at the same decision level means we have a UIP
    if (this_level_count == 1) {
      break; // get learnt gate,which only have one line at decision level
    }

    if (ls.at(trace_line).source_lines.size() != 0) {
      learnt_gate = update_learnt_gate(learnt_gate, trace_line);
      if (learnt_gate.size() == 0) 
        {
          return -2;
        }
      //std::cout<<"*************: learnt_gate size: "<<learnt_gate.size()<<std::endl;
    }
  } while (true);

  // delete is_fixed_value line,and set learnt line polarity from learnt gate
  for (int i = 0; i < learnt_gate.size();) {
    if (ls.at(learnt_gate[i]).level == 0) {
      learnt_gate.erase(learnt_gate.begin() + i);
    } else
      i++;
  }
  std::vector<int> polarity;
  // add learnt gate to the graph
  if (learnt_gate.size() == 1) // set is_fixed_value, backtrack to level 0
  {
    std::cout << "learnt_gate.size() == 1, backtrack to level 0" << std::endl;
    int temp_assign = ls.at(learnt_gate[0]).assign;
    // backtrack to 0 level
    decision_line_name.clear();
    cancel_assignment(0);
    // ls.at(learnt_gate[0]).level = 0;
    ls.at(learnt_gate[0]).assign = 1 - temp_assign;
    ls.at(learnt_gate[0]).level = 0;
    ls.at(learnt_gate[0]).source_lines.clear();
    decision_line_name.push_back(learnt_gate[0]);
    return learnt_gate[0]; // which is be_fixed_value
  } else  // learnt_gate.size()>1
  {
    // add output to graph
    std::cout << "get a learnt gate, size: " << learnt_gate.size() << std::endl;
    learnt_gate_num++;
    Line output_line(-abs(learnt_gate_num), true);
    graph.ensure_line(-abs(learnt_gate_num));
    Line *output = graph.add_learnt_output(-abs(learnt_gate_num));
    for (int i = 0; i < learnt_gate.size(); ++i) {
      // add input lines to graph
      m_learnt_inputs.push_back(graph.m_name_to_line.at(learnt_gate[i]));
      polarity.push_back(1 - ls.at(learnt_gate[i]).assign);
    }
    update_wight(learnt_gate);
    // add complete learnt_gate into graph
    auto this_gate = graph.add_learnt_gate(m_learnt_inputs, output, polarity);
    watching_list[1 - this_gate->get_inputs_polarity()[0]][this_gate->get_inputs()[0]->num_name].push_back(graph.get_gates().size() - 1);
    watching_list[1 - this_gate->get_inputs_polarity()[1]][this_gate->get_inputs()[1]->num_name].push_back(graph.get_gates().size() - 1);

    // backtrack
    int second_max_level_line = second_maxDecision_line(m_learnt_inputs);
    int second_level = ls.at(second_max_level_line).level;
    //多回溯一层然后加上一层
    decision_line_name.erase(decision_line_name.begin() + second_level, decision_line_name.end());
    cancel_assignment(decision_line_name.size() - 1);
    int decision = FindDecisionTarget();
    Gate *gate = graph.m_name_to_line.at(decision_line)->source;
    ls.at(decision).assign = int(gate->get_dir_imp1().size() > gate->get_dir_imp0().size());
    ls.at(decision).level = second_level;
    ls.at(decision).source_lines.clear();
    decision_line_name.push_back(decision);
    std::cout << "finish once leant gate : conflict_backtrack" << std::endl;
    return decision; // return second_max_level line's name
  }
}

// update learnt gate
std::vector<int> &solver::update_learnt_gate(std::vector<int> &update_gate, int trace_line) {
  // find trace_line's source lines
  std::vector<int> trace_gate(ls.at(trace_line).source_lines);
  // concatenate the two
  update_gate.insert(update_gate.end(), trace_gate.begin(), trace_gate.end());
  for (int i = 0; i < update_gate.size(); i++) {
    // remove the trace_line from the concatenated version learnt gate
    if (update_gate[i] == trace_line) {
      update_gate.erase(update_gate.begin() + i);
      i--;
    }
  }
  // remove duplicates from the last concatenated version learnt gate
  sort(update_gate.begin(), update_gate.end());
  update_gate.erase(unique(update_gate.begin(), update_gate.end()), update_gate.end());
  return update_gate; // return final learnt gate
}

// find second_Max decision level from learnt gate
int solver::second_maxDecision_line(std::vector<Line *> &a) {
  if (a.size() < 2)
    return a[0]->num_name;
  int max, second;
  if (ls.at(a[0]->num_name).level > ls.at(a[1]->num_name).level) {
    max = a[0]->num_name;    //最大值
    second = a[1]->num_name; //第二大值
  } else {
    max = a[1]->num_name;    //最大值
    second = a[0]->num_name; //第二大值
  }
  for (int i = 2; i < a.size(); ++i) {
    if (ls.at(a[i]->num_name).level > ls.at(max).level) {
      second = max; //更新最大值和次大值
      max = a[i]->num_name;
    } else if (ls.at(a[i]->num_name).level < max && ls.at(a[i]->num_name).level > second) {
      second = a[i]->num_name;
    }
  }
  return second;
}
void solver::cancel_assignment(int decision_line_level) {
  for (auto &temp : ls) {
    if (temp.second.level > decision_line_level) {
      temp.second.assign = 2;
      temp.second.level = -1;
      temp.second.source_lines.clear();
    }
  }
}
void solver::update_wight(const std::vector<int> &input_line) {
  for (const auto &input : input_line) {
    ls.at(input).weight += 100;
  }
}