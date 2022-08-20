#include "circuit_graph.h"
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#ifndef SOLVER_H
#define SOLVER_H
extern std::vector<std::vector<int>> origin_cnf;
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
    int assign ;
    int weight ;
    int level;
    // only circuit output and conflict gate's output and learnt gate is one line
    bool is_fixed_value;
    std::vector<int> source_lines;
    line_information()
    {
        this->assign = -1;
        this->weight = -1;
        this->level  = -10;
        this->is_fixed_value = false;
    }
};

class solver
{
public:
    // constructor function initialize literals and literal_frequency;output = 1
    solver(const CircuitGraph &);
    // solve process
    void solve(const CircuitGraph &);
    int CDCLsolver(const CircuitGraph &);
    void test(const CircuitGraph &);
private:
    //存储lines的赋值，其中-1 - unassigned；0 - false； 1 - true
    std::unordered_map<int, line_information> lines_status_num;
    std::vector<int> sort_destination_gates;
    std::vector<int> the_name_of_input_line;
    std::vector<int> the_name_of_conflict_line; // conflict line's name
    int previous_conflict;  //record conflict gate address
    Gate* conflict_gate;
    // common solving operations both cnf and circuit
    int FindDecisionTarget(std::unordered_map<int, line_information> &);
    int conflict_backtrack(int);

    int DPLL(const CircuitGraph &, int);
    int BCP(const CircuitGraph &, int);

    // only for circuit solving
    bool SingleGateReasonBoost(Gate *current_gate, std::queue<int> &bcp_que,int decision_line);
    int change_lines_information(int line_name,int level,std::vector<int>source_lines);
    std::vector<int> &update_learnt_gate(std::vector<int> &update_gate, int trace_line);

    // only for cnf solving
    // int unit_propagate(cnf &f, std::queue<int> &bcp_que, int reason_line_name);  // performs unit propagation
    // int apply_transform(cnf &f, std::queue<int> &bcp_que, int reason_line_name); // applies the value of the literal in

    //
    void show_result(const CircuitGraph &, int);
    void print_lines_source(const CircuitGraph &);
};



#endif