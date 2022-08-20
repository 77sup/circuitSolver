#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include <cmath>
std::vector<std::vector<int>> origin_cnf;
solver::solver(const CircuitGraph &graph)
{
    std::vector<int> noPO_lines_name; // store no-P0s
    std::vector<int> output;          // store PIs
    std::cout << "the number of all lines:";
    std::cout << graph.m_name_to_line.size() << std::endl;

    for (int i = 0; i < graph.get_lines().size(); i++)
    {
        if (graph.get_lines()[i].is_output)
        {
            output.push_back(graph.get_lines()[i].num_name);
        }
        else
        {
            noPO_lines_name.push_back(graph.get_lines()[i].num_name);
        }
    }
    for (int i = 0; i < noPO_lines_name.size(); i++)
    {
        line_information temp;
        lines_status_num.emplace(noPO_lines_name[i], temp);
        // find PIs to store
        if (!graph.m_name_to_line.at(noPO_lines_name[i])->source)
        {
            the_name_of_input_line.push_back(noPO_lines_name[i]);
        }
    }
    // assign POs to 1
    for (int i = 0; i < output.size(); i++)
    {
        line_information temp;
        temp.assign = 1;
        temp.is_fixed_value = true;
        lines_status_num.emplace(output[i], temp);
    }
    // according to fan_outs numbers to order(max->min)
    int noPO_lines_name_size = noPO_lines_name.size();
    for (int i = 0; i < noPO_lines_name_size; i++)
    { // put (fanouts>3 || inputs) lines into sort_destination_gates
        Line *temp = graph.m_name_to_line.at(noPO_lines_name[i]);
        if ((temp->destination_gates.size() > 10) || (!temp->source))
        {
            sort_destination_gates.push_back(noPO_lines_name[i]);
            lines_status_num.at(noPO_lines_name[i]).weight = temp->destination_gates.size() + int((!temp->source)) * 100;
        }
    }
}
// choose a line to assign(decision),according to ordered fan_outs numbers
int solver::FindDecisionTarget(std::unordered_map<int, line_information> &lines_status_num)
{
    int Target = -1;
    int max_weight = -1;
    for (int i = 0; i < sort_destination_gates.size(); i++)
    {
        if (lines_status_num.at(sort_destination_gates[i]).assign == -1 && lines_status_num.at(sort_destination_gates[i]).weight > max_weight)
        {
            Target = sort_destination_gates[i];
            max_weight = lines_status_num.at(sort_destination_gates[i]).weight;
        }
    }
    if (Target != -1)
        lines_status_num.at(Target).weight = -1;

    return Target;
}
void solver::solve(const CircuitGraph &graph)
{
    int bcp_result;
    for (int i = 0; i < graph.get_outputs().size(); i++)
    {
        bcp_result = BCP(graph, graph.get_outputs()[i]->num_name);
        if (bcp_result == 0)
        {
            break;
        }
    }
    if (bcp_result == 1)
    {
        std::cout << "bcp_result equal to 1, enter DPLL" << std::endl;
        int dpll_result = DPLL(graph, -1);
        if (!dpll_result)
            show_result(graph, 0);
    }
    std::cout << " DPLL operation over" << std::endl;
}

int solver::DPLL(const CircuitGraph &graph, int decision_line) // return 0---unsat;return 1---sat,but this not related to solver's solution
{
    int bcp_result = BCP(graph, decision_line);
    if (bcp_result == 0)
        return 0;
    int next_decision_line = FindDecisionTarget(lines_status_num);

    if (next_decision_line == -1) // all nodes were assigned
    {
        show_result(graph, 1);
        // print_lines_source(graph);
        std::cout << std::endl;
        return 1; // solution is SAT
    }
    int flag = rand() % 2;      // randomly choose left or right node to decide assignment
    for (int i = 0; i < 2; i++) // Traverse two child nodes
    {
        int flag2;
        if (flag) // 1,right node
        {
            if (i)
                flag2 = 0;
            else
                flag2 = 1;
        }
        else
        {
            if (i)
                flag2 = 1;
            else
                flag2 = 0;
        }
        solver CircuitSolver = *this;
        CircuitSolver.lines_status_num.at(next_decision_line).assign = flag2;
        // std::cout<<"decision: "<<next_decision_line<<" assigned: "<<flag2<<
        // " source: "<<CircuitSolver.lines_status_num.at(next_decision_line).source  <<std::endl;
        int dpll_result = CircuitSolver.DPLL(graph, next_decision_line);
        if (dpll_result == 1)
            return 1;
    }
    return 0; // solution is UNSAT
}

int solver::BCP(const CircuitGraph &graph, int decision_line) // decision_line_names
{
    if (decision_line == -1)
        return 1;
    std::queue<int> bcp_que;
    bcp_que.push(decision_line);
    int head_line_que = bcp_que.front();
    std::vector<Gate *> line_connection_gates; // store all gates related with decision line
    while (!bcp_que.empty())
    {
        line_connection_gates.clear();
        // add the back gates
        for (auto it = graph.m_name_to_line.at(head_line_que)->destination_gates.begin(); it != graph.m_name_to_line.at(head_line_que)->destination_gates.end(); it++)
        {
            line_connection_gates.push_back(*it);
        }
        // add the source gate
        if (graph.m_name_to_line.at(head_line_que)->source)
        {
            line_connection_gates.push_back(graph.m_name_to_line.at(head_line_que)->source);
        }
        for (int j = 0; j < line_connection_gates.size(); j++)
        {
            if (!SingleGateReasonBoost(line_connection_gates[j], bcp_que, decision_line))
            {
                return 0; // SingleGateReasoning fail
            }
        }

        bcp_que.pop();
        head_line_que = bcp_que.front();
    }
    return 1;
}

void solver::show_result(const CircuitGraph &graph, int dpll_result)
{
    if (dpll_result)
    {
        std::cout << "SAT" << std::endl;
        for (int i = 0; i < graph.get_lines().size(); i++)
        {
            int line_name = graph.get_lines()[i].num_name;
            std::cout << line_name << "  " << lines_status_num.at(line_name).assign << std::endl;
        }
    }
    else
    {
        std::cout << "UNSAT" << std::endl;
        // print_lines_source(graph);
    }
}

int solver::CDCLsolver(const CircuitGraph &graph)
{
    int bcp_result = 0;
    int decision_level = 0; // initial decision level
    for (int i = 0; i < graph.get_outputs().size(); i++)
    {
        bcp_result = BCP(graph, graph.get_outputs()[i]->num_name);
        if (bcp_result == 0)
        {
            show_result(graph, bcp_result);
            return 0; // UNSAT,output reason out confilict,directly return unsat
        }
    }
    int decision_line = 0;
    solver solver = *this; //?
    while (true)
    {
        decision_line = solver.FindDecisionTarget(lines_status_num); //?
        if (decision_line == -1)
        {
            show_result(graph, 1);
            return 1; // SAT, output reason out all lines
        }
        decision_level++;
        int flag = rand() % 2; // randomly choose left or right node to decide assignment
        lines_status_num.at(decision_line).assign = flag;
        while (true)
        {
            int bcp_result = solver.BCP(graph, decision_line); //？
            if (bcp_result == 0)                               // find conflict
            {
                decision_line = conflict_backtrack(decision_line);
                if (lines_status_num.at(decision_line).level == 0) // UNSAT
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
int solver::conflict_backtrack(int decision_line)
{
    int decision_level = lines_status_num.at(decision_line).level;
    // learnt gate initialized with origin conflict gate
    std::vector<int> learnt_gate = the_name_of_conflict_line;
    the_name_of_conflict_line.clear();
    int conflict_decision_level = decision_level;
    int this_level_count = 0; // number of lines from the same decision level found
    int trace_line = 0;       // line whose previous reason gate will next be used to resolve
    do
    {
        this_level_count = 0;
        for (int i = 0; i < learnt_gate.size(); i++)
        {
            int line_decision_level = lines_status_num.at(learnt_gate[i]).level;
            if (lines_status_num.at(learnt_gate[i]).is_fixed_value)
            {
                learnt_gate.erase(learnt_gate.begin() + i);
            }
            if (line_decision_level == conflict_decision_level) // a line at the same decision level has been found
            {
                this_level_count++;
            }
            if (line_decision_level == conflict_decision_level && lines_status_num.at(decision_line).source_lines.size() != 0)
            {
                trace_line = learnt_gate[i];
            }
        }
        // only one line at the same decision level means we have a UIP
        if (this_level_count == 1)
        {
            break;
        }
        learnt_gate = update_learnt_gate(learnt_gate, trace_line);

    } while (true);
    // delete is_fixed_value line,and set learnt line polarity from learnt gate
    for (int i = 0; i < learnt_gate.size(); i++)
    {
        if (lines_status_num.at(learnt_gate[i]).is_fixed_value)
        {
            learnt_gate.erase(learnt_gate.begin() + i);
        }
        int origin_assign=lines_status_num.at(learnt_gate[i]).assign;//???
        lines_status_num.at(learnt_gate[i]).assign=1-origin_assign; //???
    }

    // add learnt gate to the graph
    if(learnt_gate.size()==1)  //set is_fixed_value, backtrack to level 0
    {
        lines_status_num.at(learnt_gate[0]).is_fixed_value=true;
    }
    else
    {
        Line();
        Gate add_learnt_gate=Gate(or, Line *output, learnt_gate);

    }

    // backtrack
}
// update learnt gate
std::vector<int> &solver::update_learnt_gate(std::vector<int> &update_gate, int trace_line)
{
    // find trace_line's source lines
    std::vector<int> trace_gate = lines_status_num.at(trace_line).source_lines;
    // concatenate the two
    update_gate.insert(update_gate.end(), trace_gate.begin(), trace_gate.end());
    for (int i = 0; i < update_gate.size(); i++)
    {
        // remove the trace_line from the concatenated version learnt gate
        if (update_gate[i] == trace_line)
        {
            update_gate.erase(update_gate.begin() + i);
            i--;
        }
    }
    // remove duplicates from the last concatenated version learnt gate
    sort(update_gate.begin(), update_gate.end());
    update_gate.erase(unique(update_gate.begin(), update_gate.end()), update_gate.end());
    return update_gate; // return final learnt gate
}

int solver::change_lines_information(int line_name, int level, std::vector<int> source_lines)
{
}
