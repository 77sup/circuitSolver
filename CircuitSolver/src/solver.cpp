#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include "../include/cnf.h"
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
    // m_cnf.initialize();
}
// choose a line to assign(decision),according to ordered fan_outs numbers
int solver::FindDecisionTarget(std::unordered_map<int, line_information> &lines_status_num)
{
    int Target = -1;
    int max_weight = -1;
    for (int i = 0; i < sort_destination_gates.size(); i++)
    {
        if (lines_status_num.at(sort_destination_gates[i]).assign == -1 &&
            lines_status_num.at(sort_destination_gates[i]).weight > max_weight)
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
        // std::cout<<"output: "<<graph.get_outputs()[i]->num_name<<std::endl;
        bcp_result = BCP(graph, graph.get_outputs()[i]->num_name);
        // for(int i=0;i<graph.get_lines().size();i++){
        //     int line_name=graph.get_lines()[i].num_name;
        //     std::cout<<line_name<<"  "<<lines_status_num.at(line_name).assign<<std::endl;
        //     }
        if (bcp_result == 0)
        {
            break;
        }
    }
    std::cout << "bcp_result: " << bcp_result << std::endl;
    if (bcp_result == 1)
    {
        std::cout << "bcp_result equal to 1, enter DPLL" << std::endl;
        int dpll_result = DPLL(graph, -1);
        if (!dpll_result)
            show_result(graph, 0);
    }
    std::cout << "after DPLL operation" << std::endl;
}

int solver::DPLL(const CircuitGraph &graph, int decision_line) // return 0---unsat;return 1---sat,but this not related to solver's solution
{
    int bcp_result = BCP(graph, decision_line);
    /*for(int i=0;i<graph.get_lines().size();i++)
    {
        int line_name=graph.get_lines()[i].num_name;
        std::cout<<line_name<<"  "<<lines_status_num.at(line_name).assign<<std::endl;
    }*/
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
        CircuitSolver.lines_status_num.at(next_decision_line).source = next_decision_line;
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
    std::vector<Gate *> line_connection_gates;
    int result = apply_transform(m_cnf, bcp_que, head_line_que);
    if (result == -1)
    {
        return 0;
    }
    while (!bcp_que.empty())
    {
        int res1 = unit_propagate(m_cnf, bcp_que, head_line_que);
        if (res1 == 0)
        {
            // std::cout<<"unit propagation empty clasue"<<std::endl;
            return 0;
        }
        line_connection_gates.clear();
        // add the source gate
        if (graph.m_name_to_line.at(head_line_que)->source)
        {
            line_connection_gates.push_back(graph.m_name_to_line.at(head_line_que)->source);
        }
        // add the back gates
        for (auto it = graph.m_name_to_line.at(head_line_que)->destination_gates.begin(); it != graph.m_name_to_line.at(head_line_que)->destination_gates.end(); it++)
        {
            line_connection_gates.push_back(*it);
        }
        for (int j = 0; j < line_connection_gates.size(); j++)
        {
            if (!SingleGateReasonBoost(line_connection_gates[j], bcp_que, head_line_que, decision_line))
            {

                // std::cout<<"SingleGateReasonBoost conflict: "<<head_line_que<<
                //" assigned: "<<lines_status_num.at(head_line_que)  <<std::endl;
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
void solver::print_lines_source(const CircuitGraph &graph)
{
    for (int i = 0; i < lines_status_num.size(); i++)
    {
        std::cout << "line: " << graph.get_lines()[i].num_name << "  assigned:  " << lines_status_num.at(graph.get_lines()[i].num_name).assign << "  decision source: "
                  << lines_status_num.at(graph.get_lines()[i].num_name).source << std::endl;
    }
}

int solver::CDCLsolver(const CircuitGraph &graph)
{
    int bcp_result;
    for (int i = 0; i < graph.get_outputs().size(); i++)
    {
        bcp_result = BCP(graph, graph.get_outputs()[i]->num_name);
        std::cout<<"decision line: "<<graph.get_outputs()[i]->num_name<<" assigned: "<<lines_status_num.
        at(graph.get_outputs()[i]->num_name).assign<<std::endl;
        print_lines_source(graph);
        if (bcp_result == 0)
        {
            std::cout << "can get unsat by outputs assign" << std::endl;
            show_result(graph, bcp_result);
            return 0; // UNSAT,output reason out confilict,directly return unsat
        }
    }
    solver origin_solver = *this;
    std::vector<solver> decision_solver;
    std::vector<std::pair<int, int>> decision_variable_information; // first--decisoin line name;second--record decisoin line flip
    // all change
    decision_solver.push_back(origin_solver);
    decision_variable_information.push_back(std::make_pair(0, 0));

    int decision_line;
    // all change
    decision_solver.push_back(decision_solver.back());
    decision_line = decision_solver.back().FindDecisionTarget(decision_solver.back().lines_status_num);
    decision_variable_information.push_back(std::make_pair(decision_line, 1));

    if (decision_line == -1)
    {
        show_result(graph, 1);
        return 1; // SAT, output reason out all lines
    }
    int flag = rand() % 2; // randomly choose left or right node to decide assignment
    decision_solver.back().lines_status_num.at(decision_line).assign = flag;
    decision_solver.back().lines_status_num.at(decision_line).source = decision_line;
    decision_variable_information.push_back(std::make_pair(decision_line, 1));
    solver new_solver = decision_solver.back();
 
    while (true)
    {
        int bcp_result = new_solver.BCP(graph, decision_line);
        std::cout<<"decision line: "<<decision_line<<" assigned: "<<lines_status_num.at(decision_line).assign<<std::endl;
        print_lines_source(graph);
        if (bcp_result == 0) // find conflict
        {
            decision_line = new_solver.conflict_backtrack(graph, decision_solver, decision_variable_information);
            if (decision_line == -2) // UNSAT
            {
                show_result(graph, 0);
                return 0;
            }
            new_solver = decision_solver.back();
        }
        else // bcp normally exit,enter into next decision
        {
            decision_solver.back() = new_solver;
            decision_solver.push_back(new_solver);
            decision_line = new_solver.FindDecisionTarget(decision_solver.back().lines_status_num);
            if (decision_line == -1)
            {
                new_solver.show_result(graph, 1);
                return 1; // SAT, all lines have been assigned
            }
            // next decision
            // now_solver=decisioh_solver.back();
            int flag = rand() % 2; // randomly choose left or right node to decide assignment
            new_solver.lines_status_num.at(decision_line).assign = flag;
            new_solver.lines_status_num.at(decision_line).source = decision_line;
            decision_variable_information.push_back(std::make_pair(decision_line, 1));
        }
    }
}
// conflict analysis,add learnt clause,and backtrack
int solver::conflict_backtrack(const CircuitGraph &graph, std::vector<solver> &decisioh_solver, std::vector<std::pair<int, int>> &decision_variable_information)
{
    std::cout<<"conflict_backtrack-------1"<<std::endl;
    std::vector<int> learnt_clause;
    int decision_line = -1;
    learnt_clause.push_back(-1); // flag bit
    //conflict line name
    for (int it = 0; it < the_name_of_conflict_line.size(); it++)
    {
        int learnt_literal = 0;
        std::cout<<"the_name_of_conflict_line[it]"<<the_name_of_conflict_line[it]<<std::endl;
        if (graph.m_name_to_line.at(the_name_of_conflict_line[it])->is_output)
            continue;
        std::cout<<"conflict_backtrack-------2"<<std::endl;
        if (lines_status_num.at(the_name_of_conflict_line[it]).source != -1 && lines_status_num.at(the_name_of_conflict_line[it]).assign != -1)
        {
            int source = lines_status_num.at(the_name_of_conflict_line[it]).source;
            int assign = lines_status_num.at(lines_status_num.at(the_name_of_conflict_line[it]).source).assign;
            learnt_literal = source * pow(-1, assign);
        }
        else
            continue; // conflict line unassigned
        int i;
        std::cout<<"conflict_backtrack-------3"<<std::endl;
        //Delete the duplicate text of the study sentence
        for (i = 1; i < learnt_clause.size(); i++)
        {
            if (learnt_literal == learnt_clause[i])
                break;
        }
        std::cout<<"conflict_backtrack-------4"<<std::endl;
        if (i == learnt_clause.size())
        {
            learnt_clause.push_back(learnt_literal);
        }
    }
    // update cnf
    std::cout<<"conflict_backtrack-------5"<<std::endl;
    if (learnt_clause.size() > 1)
    {
        origin_cnf.insert(origin_cnf.begin(), learnt_clause);
        std::cout<<"decisioh_solver.size()"<<decisioh_solver.size()<<std::endl;
        for (int i = 0; i < decisioh_solver.size(); i++)
        {
            std::cout<<"conflict_backtrack-------6"<<std::endl;
            decisioh_solver[i].m_cnf.clauses.insert(decisioh_solver[i].m_cnf.clauses.begin(), learnt_clause);
            std::cout<<"conflict_backtrack-------61"<<std::endl;
        }
        std::cout<<"conflict_backtrack-------7"<<std::endl;
    }
    // unset assignments
    int i;
    std::cout<<"conflict_backtrack-------8"<<std::endl;
    for (i = decision_variable_information.size() - 1; i > -1; i--)
    {
        //hui su
        if (decision_variable_information[i].second == 2) // decision line both left and right node have been assigned
        {
            if (i == 1) // root node
            {
                std::cout<<"conflict_backtrack-------9"<<std::endl;
                decisioh_solver.clear();
                std::cout<<"conflict_backtrack-------91"<<std::endl;
                decision_variable_information.clear();
                std::cout<<"conflict_backtrack-------92"<<std::endl;
                return -2; // special flag; UNSAT,backtrack to root node
            }
            else
            {
                std::cout<<"conflict_backtrack-------10"<<std::endl;
                decisioh_solver.pop_back();
                decision_variable_information.pop_back();
                std::cout<<"conflict_backtrack-------11"<<std::endl;
            }
        }
        else //fan zhuan
        {
            // i=0 || i>0
            std::cout<<"conflict_backtrack-------12"<<std::endl;
            int assign = lines_status_num.at(decision_variable_information[i].first).assign;
            int convert = 1 - assign;
            std::cout<<"conflict_backtrack-------12"<<std::endl;
            lines_status_num.at(decision_variable_information[i].first).assign = convert;
            std::cout<<"conflict_backtrack-------13"<<std::endl;
            decision_variable_information[i].second = 2;
            std::cout<<"conflict_backtrack-------14"<<std::endl;
            decisioh_solver.push_back(decisioh_solver[i-1]);
            std::cout<<"conflict_backtrack-------15"<<std::endl;
            break;
        }
    }
    std::cout<<"conflict_backtrack-------16"<<std::endl;
    decision_line = decision_variable_information[i].first;
    std::cout<<"conflict_backtrack-------17"<<std::endl;
    return decision_line;
}

