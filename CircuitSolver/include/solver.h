#include "circuit_graph.h"
#include <iostream>
#include <vector>
#include <queue>
#include <limits.h>
#include <algorithm>
#ifndef SOLVER_H
#define SOLVER_H
extern std::vector<std::vector<int>> origin_cnf;
extern int number;
enum Cat
{
    unsatisfied, // 0, when no satisfying assignment has been found after exhaustively searching,
    satisfied,   // 1, when a satisfying assignment has been found
    normal,      // 2,到目前为止没有找到可满足赋值，并且DPLL()已正常退出
    completed    // 3,当DPLL算法完成执行时
};
class line_information
{
public:
    int assign;
    int weight;
    int level;
    std::vector<int> source_lines;
    line_information()
    {
        this->assign = 2;
        this->weight = -1;
        this->level = -1;
    }
};

class solver
{
public:
    using watching_type = std::vector<std::vector<int>>;
    int learnt_gate_num = 0;
    // constructor function initialize literals and literal_frequency;output = 1
    solver(CircuitGraph &);
    // solve process
    void solve(CircuitGraph &);
    int CDCLsolver(CircuitGraph &);
    void test(CircuitGraph &);
private:
    //存储lines的赋值，其中2 - unassigned；0 - false； 1 - true
    std::unordered_map<int, line_information> ls; //update
    std::vector<int> sort_destination_gates;
    std::vector<int> the_name_of_input_line;
    std::vector<int> conflict_line; // conflict line's name
    int previous_conflict;          // record conflict gate address
    Gate *conflict_gate;
    std::vector<watching_type> watching_list;

    int compute_wight(const CircuitGraph &grahp,int line_name);
    //for two-literal watch to struct direct and indirect implicaiton graph
    void structural_implication_map(CircuitGraph &graph);
    void struct_implication(Gate &, int );

    int FindDecisionTarget();
    int watch_BCP(CircuitGraph &, int);
    int conflict_backtrack(int, CircuitGraph &, std::vector<int> &, int);
    void cancel_assignment(int decision_line_level);
    int second_maxDecision_line(std::vector<Line *> &);
    void update_wight(const std::vector<int> &input_line, int);
    void multiplication_q(int, int);

    // only for circuit solving
    int change_lines_information(int line_name, int level, std::vector<int> source_lines);
    std::vector<int> &update_learnt_gate(std::vector<int> &update_gate, int trace_line);
    Gate::Type tran_type(Gate::Type is,Gate::Type other);

    bool single_gate_dir(Gate *current_gate, std::vector<int> &bcp_vec, int decision_line);
    int single_gate_indir(CircuitGraph &graph,Gate *current_gate, std::vector<int> &bcp_vec, int decision_line , int bcp_idx, int list_idx,int i);
    int x_gate_indir(Gate *current_gate, std::vector<int> &bcp_vec, int decision_line , int bcp_idx, int list_idx);
    int learn_gate_indir(Gate *current_gate, std::vector<int> &bcp_vec, int decision_line, int bcp_idx, int list_idx);

    void show_result(CircuitGraph &, int);
    void print_lines_source(CircuitGraph &);
};

#endif