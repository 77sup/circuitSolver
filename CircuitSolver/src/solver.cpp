#include "../include/solver.h"

#include <cmath>

#include "../include/circuit_graph.h"
solver::solver(CircuitGraph &graph)
{
    std::vector<int> noPO_lines_name; // store no-P0s
    std::vector<int> output;          // store PIs
    std::cout << "the number of all lines:";
    std::cout << graph.m_name_to_line.size() << std::endl;
    for (auto &line : graph.lines())
    {
        line_information temp;
        if (line.is_output)
        {
            temp.level = 0;
            temp.assign = 1;
        }
        lines_status_num.emplace(line.num_name, temp);
    }
    delete_not_and_buff(graph);
    for (unsigned int i = 0; i < graph.lines().size(); ++i)
    {
        if (graph.lines()[i].is_output)
            output.push_back(graph.lines()[i].num_name);
        else
            noPO_lines_name.push_back(graph.lines()[i].num_name);
    }
    structural_implication_map(graph);
    std::cout<<" END structural_implication_map(graph)"<<std::endl;
    // according to fan_outs numbers to order(max->min)
    int noPO_lines_name_size = noPO_lines_name.size();
    for (int i = 0; i < noPO_lines_name_size; i++)
    { // put (fanouts>12 || inputs) lines into sort_destination_gates
        Line *temp = graph.m_name_to_line.at(noPO_lines_name[i]);
        if ((temp->destination_gates.size() > 12) || (!temp->source))
        {
            sort_destination_gates.push_back(noPO_lines_name[i]);
            lines_status_num.at(noPO_lines_name[i]).weight = temp->destination_gates.size() + int((!temp->source)) * 1;
        }
    }
    std::cout << "out" << std::endl;
}

void solver::delete_not_and_buff(CircuitGraph &graph)
{
    // initialize all gate's inputs polarity to 1, aims:1.open space for inputs_polarit vector, 2.only when encounter NOT change it
    for (auto &gate : graph.get_gates())
    {
        std::vector<int> inputs_polarity(gate.get_inputs().size(), 1);
        gate.change_inputs_polarity(inputs_polarity);
    }
    for (unsigned int i = 0; i < graph.get_gates().size(); ++i) // traverse all gates
    {
        auto type = graph.get_gates()[i].get_type(); // acquire current gate's type
        int not_buff_sum=0;
        if (type == Gate::Type::Not || type == Gate::Type::Buff)
        {
            std::cout << "gate inputs size:" << graph.get_gates()[i].get_inputs().size() << std::endl;
            Line *temp_input = graph.get_gates()[i].get_inputs()[0]; // not and buff only have one input
            Line *temp_output = graph.get_gates()[i].get_output();   // all gates only have one output
            // update temp_input's destination_gates
            temp_input->destination_gates.erase(&graph.get_gates()[i]); // delete current not/buff
            for (auto d_gate : temp_output->destination_gates)          // get current not/buff's all destination gate
            {
                temp_input->destination_gates.insert(d_gate); // add not/buff's destination gate into this input's destination
                // if d_gate is not/buff,don't change d_gate's input polarity,instead change d_gate' type

                if (d_gate->get_type() == Gate::Type::Not || d_gate->get_type() == Gate::Type::Buff)
                {
                    d_gate->inputs()[0] = temp_input;
                    d_gate->type() = tran_type(d_gate->get_type(), type); // type is current gate's type
                    continue;                                             // skip to next time loop
                }
                // d_gate isn't not/buff, change its input's polarity
                for (unsigned int t = 0; t < d_gate->inputs().size(); t++)
                {
                    if (d_gate->inputs()[t] == temp_output)
                    {
                        d_gate->inputs()[t] = temp_input;
                        if (type == Gate::Type::Not)
                        {
                            d_gate->change_inputs_polarity(t, 0);
                        }
                        break;
                    }
                }
            }
            // if current gate's output is PO,need to change graph's m_output vector
            if (temp_output->is_output)
            {
                auto flag = std::find(graph.outputs().begin(), graph.outputs().end(), temp_output);
                (*flag) = temp_input;
                (*flag)->is_output = true;
                lines_status_num.at(temp_input->num_name) = lines_status_num.at(temp_output->num_name);
                if (type == Gate::Type::Not)
                    lines_status_num.at(temp_input->num_name).assign = 1 - lines_status_num.at(temp_input->num_name).assign;
            }
            graph.get_gates().erase(graph.get_gates().begin() + i); // delete not/buff from graph
            lines_status_num.erase(temp_output->num_name);          // delete current buff/not output line infomation
            graph.m_name_to_line.erase(temp_output->num_name);
            for (unsigned int p = 0; p < graph.lines().size(); ++p)
            {
                if (graph.lines()[p].num_name == temp_output->num_name)
                {
                    graph.lines().erase(graph.lines().begin() + p);
                    break;
                }
            }
            // recover seek pointer
            i--;
        }
    }
}

void solver::structural_implication_map(CircuitGraph &graph)
{
    // open up space for watching-0 and watching-1 vector,first find max num_name
    int max_num_name = 0;
    for (auto temp : lines_status_num)
    {
        if (temp.first > max_num_name)
        {
            max_num_name = temp.first;
        }
    }
    std::cout << "max_num_name: " << max_num_name << std::endl;
    watching0.resize(max_num_name+1);
    watching1.resize(max_num_name+1);
    std::cout<<"********: "<<graph.get_gates().size()<<std::endl;

    for (unsigned int i = 0; i < graph.get_gates().size(); ++i)
    {
        //struct dir/indir implicaiton,initialize two pointer, decide watch value
        std::cout<<"%%%%%%%%%%%%%"<<std::endl;
        struct_implication(graph.get_gates()[i], i);
        std::cout<<"222222"<<std::endl;
        std::cout<<"i: "<<i<<std::endl;
    }
        //std::cout<<"1111111"<<std::endl;
}
Gate::Type solver::tran_type(Gate::Type is, Gate::Type other)
{
    if (other == Gate::Type::Buff)
        return is;
    else
    {
        switch (is)
        {
        case Gate::Type::Buff:
            return Gate::Type::Not;
        case Gate::Type::Not:
            return Gate::Type::Buff;
        default:
            return is;
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
    while (!bcp_que.empty())
    {
        head_line_que = bcp_que.front();
        // add the source gate
        if (graph.m_name_to_line.at(head_line_que)->source)
        {
            if (graph.m_name_to_line.at(head_line_que)
                    ->source->get_is_learnt_gate())
            {
                if (!LearntGateReason(graph.m_name_to_line.at(head_line_que)->source,
                                      bcp_que, decision_line))
                {
                    return 0; // SingleGateReasoning fail
                }
            }
            else
            {
                if (!SingleGateReason(graph.m_name_to_line.at(head_line_que)->source,
                                      bcp_que, decision_line))
                {
                    return 0; // SingleGateReasoning fail
                }
            }
        }
        // add the back gates
        for (auto it : graph.m_name_to_line.at(head_line_que)->destination_gates)
        {
            if (it->get_is_learnt_gate())
            {
                if (!LearntGateReason(it, bcp_que, decision_line))
                {
                    return 0; // SingleGateReasoning fail
                }
            }
            else
            {
                if (!SingleGateReason(it, bcp_que, decision_line))
                {
                    return 0; // SingleGateReasoning fail
                }
            }
        }
        bcp_que.pop();
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
            std::cout << line_name << "  " << lines_status_num.at(line_name).assign
                      << std::endl;
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
    for (int i = 0; i < graph.get_outputs().size(); i++)
    {
        bcp_result = BCP(graph, graph.get_outputs()[i]->num_name);
        if (bcp_result == 0)
        {
            show_result(graph, bcp_result);
            return 0; // UNSAT,output reason out confilict,directly return unsat
        }
    }
    int decision_line; // initial decision line
    std::vector<solver> backtrack_solver;
    std::vector<int> decision_line_name;
    backtrack_solver.push_back(*this);
    decision_line_name.push_back(0);
    while (true)
    {
        backtrack_solver.push_back(backtrack_solver.back());
        decision_line = backtrack_solver.back().FindDecisionTarget();
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
            int bcp_result = backtrack_solver.back().BCP(graph, decision_line);
            if (bcp_result == 0) // find conflict
            {
                if (backtrack_solver.back().lines_status_num.at(decision_line).level == 0) // UNSAT
                {
                    show_result(graph, 0);
                    return 0;
                }
                decision_line = conflict_backtrack(
                    decision_line, graph, backtrack_solver, decision_line_name);
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
int solver::conflict_backtrack(int decision_line,
                               CircuitGraph &graph,
                               std::vector<solver> &backtrack_solver,
                               std::vector<int> &decision_line_name)
{
    int decision_level = backtrack_solver.back().lines_status_num.at(decision_line).level;
    std::vector<Line *> m_learnt_inputs;
    // learnt gate initialized with origin conflict gate
    int conflict_decision_level = decision_level;
    std::cout << "conflict_decision_level: " << conflict_decision_level
              << std::endl;
    std::cout << "conflict_line is following:" << conflict_decision_level
              << std::endl;
    std::vector<int> learnt_gate(backtrack_solver.back().conflict_line);
    for (auto temp : learnt_gate)
        std::cout << temp << "   "
                  << backtrack_solver.back().lines_status_num.at(temp).level
                  << std::endl;
    int this_level_count = 0; // number of lines from the same decision level found
    int trace_line = 0;       // line whose previous reason gate will next be used to resolve
    int line_decision_level;
    int second_max_level_line = 0;
    do
    {
        this_level_count = 0;
        for (int i = 0; i < learnt_gate.size(); i++)
        {
            line_decision_level = backtrack_solver.back().lines_status_num.at(learnt_gate[i]).level;
            if (line_decision_level == 0)
            {
                learnt_gate.erase(learnt_gate.begin() + i);
                i--;
                // if learnt_gate size=0, UNSAT
                if (learnt_gate.size() == 0) // UNSAT
                {
                    return -2;
                }
                continue;
            }
            if (line_decision_level == conflict_decision_level) // a line at the same decision level
                                                                // has been found
            {
                this_level_count++;
            }
            // a line at the same decision level but not a decision line
            if (line_decision_level == conflict_decision_level && backtrack_solver.back().lines_status_num.at(learnt_gate[i]).source_lines.size() != 0)
            {
                trace_line = learnt_gate[i];
            }
        }

        // only one line at the same decision level means we have a UIP
        if (this_level_count == 1)
        {
            break; // get learnt gate,which only have one line at decision level
        }
        if (backtrack_solver.back()
                .lines_status_num.at(trace_line)
                .source_lines.size() != 0)
        {
            //!!!
            learnt_gate = update_learnt_gate(learnt_gate, trace_line, backtrack_solver.back());
        }
    } while (true);

    // delete is_fixed_value line,and set learnt line polarity from learnt gate
    for (int i = 0; i < learnt_gate.size();)
    {
        if (backtrack_solver.back().lines_status_num.at(learnt_gate[i]).level == 0)
        {
            learnt_gate.erase(learnt_gate.begin() + i);
        }
        else
            i++;
    }
    std::cout << "final learnt gate is following: first---name; second---level"
              << std::endl;
    for (auto temp : learnt_gate)
        std::cout << temp << "   "
                  << backtrack_solver.back().lines_status_num.at(temp).level
                  << std::endl;
    std::vector<int> polarity;
    // add learnt gate to the graph
    if (learnt_gate.size() == 1) // set is_fixed_value, backtrack to level 0
    {
        int temp_assign = backtrack_solver.back().lines_status_num.at(learnt_gate[0]).assign;
        // backtrack to 0 level
        backtrack_solver.erase(backtrack_solver.begin() + 1,
                               backtrack_solver.end());
        decision_line_name.erase(decision_line_name.begin() + 1,
                                 decision_line_name.end());

        backtrack_solver.back().lines_status_num.at(learnt_gate[0]).level = 0;
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
            polarity.push_back(
                1 - backtrack_solver.back().lines_status_num.at(learnt_gate[i]).assign);
        }

        // add complete learnt_gate into graph
        graph.add_learnt_gate(m_learnt_inputs, output, polarity);

        // backtrack
        int second_max_level_line = backtrack_solver.back().second_maxDecision_line(m_learnt_inputs);
        int second_level = backtrack_solver.back()
                               .lines_status_num.at(second_max_level_line)
                               .level;
        int decision = decision_line_name[second_level];
        int assign = backtrack_solver.back().lines_status_num.at(decision).assign;
        decision_line_name.erase(decision_line_name.begin() + second_level,
                                 decision_line_name.end());
        decision_line_name.push_back(decision);
        backtrack_solver.erase(backtrack_solver.begin() + second_level + 1,
                               backtrack_solver.end());
        backtrack_solver.back() = backtrack_solver[backtrack_solver.size() - 2];
        backtrack_solver.back().lines_status_num.at(decision).assign = assign;
        backtrack_solver.back().lines_status_num.at(decision).level = backtrack_solver.size() - 1;
        backtrack_solver.back().lines_status_num.at(decision).source_lines.clear();

        return decision; // return second_max_level line's name
    }
}

// update learnt gate
std::vector<int> &solver::update_learnt_gate(std::vector<int> &update_gate,
                                             int trace_line,
                                             const solver &end_solver)
{
    // find trace_line's source lines
    std::vector<int> trace_gate = end_solver.lines_status_num.at(trace_line).source_lines;
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
    update_gate.erase(unique(update_gate.begin(), update_gate.end()),
                      update_gate.end());
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

int solver::change_lines_information(int line_name,
                                     int level,
                                     std::vector<int> source_lines)
{
    return 0;
}
void solver::print_lines_source(CircuitGraph &garph)
{
    for (auto temp : garph.get_lines())
        std::cout << temp.num_name << "   "
                  << lines_status_num[temp.num_name].assign << std::endl;
}


void solver::test(CircuitGraph & graph)
{
    


}