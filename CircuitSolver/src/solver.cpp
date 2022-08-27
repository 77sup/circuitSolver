#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include <cmath>
solver::solver(CircuitGraph &graph)
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
        temp.level = INT_MIN;
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
int solver::FindDecisionTarget()
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
void solver::solve(CircuitGraph &graph)
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

int solver::DPLL(CircuitGraph &graph, int decision_line) // return 0---unsat;return 1---sat,but this not related to solver's solution
{
    int bcp_result = BCP(graph, decision_line);
    if (bcp_result == 0)
        return 0;
    int next_decision_line = FindDecisionTarget();

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

int solver::BCP(CircuitGraph &graph, int decision_line) // decision_line_names
{
    if (decision_line == -1)
        return 1;
    std::queue<int> bcp_que;
    bcp_que.push(decision_line);
    int head_line_que = bcp_que.front();
    std::vector<Gate *> line_connection_gates; // store all gates related with decision line
    while (!bcp_que.empty())
    {
        head_line_que = bcp_que.front();
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
            if (line_connection_gates[j]->get_is_learnt_gate())
            {
                if (!LearntGateReason(line_connection_gates[j], bcp_que, decision_line))
                {
                    return 0; // SingleGateReasoning fail
                }
            }
            else
            {
                if (!SingleGateReason(line_connection_gates[j], bcp_que, decision_line))
                {
                    return 0; // SingleGateReasoning fail
                }
            }
        }

        bcp_que.pop();
        // head_line_que = bcp_que.front();
    }
    return 1;
}

void solver::show_result(CircuitGraph &graph, int dpll_result)
{
    if (dpll_result)
    {
        std::cout << "SAT" << std::endl;
        for (int i = 0; i < graph.get_inputs().size(); i++)
        {
            int line_name = graph.get_inputs()[i]->num_name;
            std::cout << line_name << "  " << lines_status_num.at(line_name).assign << std::endl;
        }
    }
    else
    {
        std::cout << "UNSAT" << std::endl;
        // print_lines_source(graph);
    }
}

int solver::CDCLsolver(CircuitGraph &graph)
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
    int decision_line = 0; // initial decision line
    std::vector<solver> backtrack_solver;
    std::vector<int> decision_line_name;
    backtrack_solver.push_back(*this);
    decision_line_name.push_back(0);
    while (true)
    {

        backtrack_solver.push_back(backtrack_solver.back());
        decision_line = backtrack_solver.back().FindDecisionTarget(); //?
        decision_line_name.push_back(decision_line);
        if (decision_line == -1)
        {
            backtrack_solver.back().show_result(graph, 1);
            return 1; // SAT, output reason out all lines
        }

        int flag = rand() % 2; // randomly choose left or right node to decide assignment
        backtrack_solver.back().lines_status_num.at(decision_line).assign = flag;
        backtrack_solver.back().lines_status_num.at(decision_line).level = backtrack_solver.size() - 1;
        while (true)
        {
            std::cout << "enter bcp" << std::endl;
            int bcp_result = backtrack_solver.back().BCP(graph, decision_line);
            std::cout << "end bcp" << std::endl;
            std::cout << "decision line: " << decision_line << " :" << backtrack_solver.back().lines_status_num.at(decision_line).level << std::endl;
            if (bcp_result == 0) // find conflict
            {
                // std::cout << "find conflict" << std::endl;
                if (backtrack_solver.back().lines_status_num.at(decision_line).level == INT_MIN) // UNSAT
                {
                    show_result(graph, 0);
                    return 0;
                }
                std::cout << "enter con_back" << std::endl;
                decision_line = conflict_backtrack(decision_line, graph, backtrack_solver, decision_line_name);
                std::cout << "end con_back" << std::endl;
                // std::cout << " conflic decision line: " << decision_line << " assign: " << backtrack_solver.back().lines_status_num.at(decision_line).assign
                //          << " decision level: " << backtrack_solver.back().lines_status_num.at(decision_line).level << std::endl;
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
int solver::conflict_backtrack(int decision_line, CircuitGraph &graph, std::vector<solver> &backtrack_solver, std::vector<int> &decision_line_name)
{
    int decision_level = backtrack_solver.back().lines_status_num.at(decision_line).level;
    std::vector<Line *> m_learnt_inputs;
    // learnt gate initialized with origin conflict gate
    std::vector<int> learnt_gate = backtrack_solver.back().conflict_line;
    conflict_line.clear(); // is it necessary ?
    int conflict_decision_level = decision_level;
    int this_level_count = 0; // number of lines from the same decision level found
    int trace_line = 0;       // line whose previous reason gate will next be used to resolve
    int second_max_level_line = 0;
    do
    {
        this_level_count = 0;
        for (int i = 0; i < learnt_gate.size(); i++)
        {
            int line_decision_level = backtrack_solver.back().lines_status_num.at(learnt_gate[i]).level;
            if (backtrack_solver.back().lines_status_num.at(learnt_gate[i]).is_fixed_value)
            {
                learnt_gate.erase(learnt_gate.begin() + i);
                // if learnt_gate size=0, UNSAT
                if (learnt_gate.size() == 0) // UNSAT
                {
                    return -2;
                }
            }
            if (line_decision_level == conflict_decision_level) // a line at the same decision level has been found
            {
                this_level_count++;
                std::cout << "this_level_count: " << this_level_count << std::endl;
            }

            if (line_decision_level == conflict_decision_level && backtrack_solver.back().lines_status_num.at(learnt_gate[i]).source_lines.size() != 0)
            {
                trace_line = learnt_gate[i];
                std::cout << " ***********learnt_gate " << std::endl;
                for (auto temp : learnt_gate)
                {
                    std::cout << "learnt gate line: " << temp << "  level: " << backtrack_solver.back().lines_status_num.at(temp).level << std::endl;
                }
            }
        }

        // only one line at the same decision level means we have a UIP
        if (this_level_count == 1)
        {
            break; // get learnt gate,which only have one line at decision level
        }
        if (backtrack_solver.back().lines_status_num.at(trace_line).source_lines.size() != 0)
        {
            std::cout << " befor update_learnt_gate " << std::endl;
            for (auto temp : learnt_gate)
            {
                std::cout << "learnt gate line: " <<temp << "  assign:  "<<backtrack_solver.back().lines_status_num.at(temp).assign<< "  level: " << backtrack_solver.back().lines_status_num.at(temp).level << 
                "  source line's size: "<<backtrack_solver.back().lines_status_num.at(temp).source_lines.size()<<std::endl;
            }
            learnt_gate = update_learnt_gate(learnt_gate, trace_line, backtrack_solver.back());
            std::cout << "if(trace_line!=0)  "<< "  after update, learnt gate size: " << learnt_gate.size() << "  trace line: " << trace_line << std::endl;
            for (auto temp : learnt_gate)
            {
                std::cout << "learnt gate line: " << temp << "  level: " << backtrack_solver.back().lines_status_num.at(temp).level << std::endl;
            }
        }
        std::cout << "!!!!!!!!!!" << std::endl;
    } while (true);
    // std::cout<<"&&&&&&&&"<<std::endl;

    // delete is_fixed_value line,and set learnt line polarity from learnt gate
    for (int i = 0; i < learnt_gate.size();)
    {
        if (backtrack_solver.back().lines_status_num.at(learnt_gate[i]).is_fixed_value ||
            (backtrack_solver.back().lines_status_num.at(learnt_gate[i]).source_lines.size() == 1 && backtrack_solver.back().lines_status_num.at(learnt_gate[i]).level == INT_MIN))
        {
            learnt_gate.erase(learnt_gate.begin() + i);
        }
        else
            i++;
    }
    std::vector<int> polarity;
    // add learnt gate to the graph
    if (learnt_gate.size() == 1) // set is_fixed_value, backtrack to level 0
    {
        int temp_assign = backtrack_solver.back().lines_status_num.at(learnt_gate[0]).assign;
        // backtrack to 0 level
        backtrack_solver.erase(backtrack_solver.begin() + 1, backtrack_solver.end());
        decision_line_name.erase(decision_line_name.begin() + 1, decision_line_name.end());

        backtrack_solver.back().lines_status_num.at(learnt_gate[0]).is_fixed_value = true;
        backtrack_solver.back().lines_status_num.at(learnt_gate[0]).level = INT_MIN;
        backtrack_solver.back().lines_status_num.at(learnt_gate[0]).assign = 1 - temp_assign;
        return learnt_gate[0]; // which is be_fixed_value
    }
    else // learnt_gate.size()>1
    {
        // add output to graph
        learnt_gate_num++;
        Line output_line(-abs(learnt_gate_num), true);

        graph.ensure_line(-abs(learnt_gate_num));

        Line *output = graph.add_learnt_output(-abs(learnt_gate_num));
        for (int i = 0; i < learnt_gate.size(); ++i)
        {
            // add input lines to graph
            m_learnt_inputs.push_back(graph.m_name_to_line.at(learnt_gate[i]));
            polarity.push_back(1 - backtrack_solver.back().lines_status_num.at(learnt_gate[i]).assign);
        }

        for (auto temp : m_learnt_inputs)
        {
            std::cout << temp->num_name << "------" << backtrack_solver.back().lines_status_num.at(temp->num_name).level << std::endl;
        }
        // construct  a learnt Gate
        // Gate learntGate(output, m_learnt_inputs);

        // add complete learnt_gate into graph
        graph.add_learnt_gate(m_learnt_inputs, output, polarity);

        // backtrack
        int second_max_level_line = backtrack_solver.back().second_maxDecision_line(m_learnt_inputs);
        // auto temp=backtrack_solver.back().lines_status_num.at(second_max_level_line);
        std::cout << "second_max_level_line level:  " << backtrack_solver.back().lines_status_num.at(second_max_level_line).level << std::endl;
        std::cout << "backtrack_solver size first:  " << backtrack_solver.size() << std::endl;

        int temp1 = backtrack_solver.back().lines_status_num.at(second_max_level_line).level;
        auto temp2 = backtrack_solver.back().lines_status_num.at(decision_line_name[temp1]); // second_max_decison_line's decision value
        decision_line_name.erase(decision_line_name.begin() + temp1, decision_line_name.end());
        backtrack_solver.erase(backtrack_solver.begin() + temp1, backtrack_solver.end());
        std::cout << "backtrack_solver size second:  " << backtrack_solver.size() << std::endl;
        backtrack_solver.back() = backtrack_solver[backtrack_solver.size() - 2];
        backtrack_solver.back().lines_status_num.at(decision_line_name[temp1]) = temp2;
        return decision_line_name[temp1]; // return second_max_level line's name
    }
}

// update learnt gate
std::vector<int> &solver::update_learnt_gate(std::vector<int> &update_gate, int trace_line, const solver &end_solver)
{
    // find trace_line's source lines
    std::vector<int> trace_gate = end_solver.lines_status_num.at(trace_line).source_lines;
    std::cout << "trace line: " << trace_line << " trace gate is following: " << std::endl;
    for (auto temp : trace_gate)
    {
        std::cout << temp << "   ";
    }
    std::cout << std::endl;
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
    std::cout << "trace gate insert to update gate: " << std::endl;
    for (auto temp : update_gate)
    {
        std::cout << temp << " ";
    }
    std::cout << std::endl;
    // remove duplicates from the last concatenated version learnt gate
    sort(update_gate.begin(), update_gate.end());
    update_gate.erase(unique(update_gate.begin(), update_gate.end()), update_gate.end());
    return update_gate; // return final learnt gate
}

// find second_Max decision level from learnt gate
int solver::second_maxDecision_line(std::vector<Line *> &a)
{
    if (a.size() < 2)
        return a[0]->num_name;
    int max, second;
    if (lines_status_num.at(a[0]->num_name).level > lines_status_num.at(a[1]->num_name).level)
    {
        max = a[0]->num_name;    //最大值
        second = a[1]->num_name; //第二大值
    }
    else
    {
        max = a[1]->num_name;    //最大值
        second = a[0]->num_name; //第二大值
    }
    for (int i = 2; i < a.size(); ++i)
    {
        std::cout << ":" << lines_status_num.at(a[i]->num_name).level << std::endl;
        if (lines_status_num.at(a[i]->num_name).level > lines_status_num.at(max).level)
        {
            second = max; //更新最大值和次大值
            max = a[i]->num_name;
        }
        else if (lines_status_num.at(a[i]->num_name).level < max && lines_status_num.at(a[i]->num_name).level > second)
        {
            second = a[i]->num_name;
        }
    }
    return second;
}

int solver::change_lines_information(int line_name, int level, std::vector<int> source_lines)
{
    return 0;
}
void solver::print_lines_source(CircuitGraph &garph)
{
    for (auto temp : garph.get_lines())
        std::cout << temp.num_name << "   " << lines_status_num[temp.num_name].assign << std::endl;
}