#include "circuit_graph.h"
#include "cnf.h"
#include <iostream>
#include<vector>
#include<queue>
#ifndef SOLVER_H
#define SOLVER_H

enum Cat {
	unsatisfied,  //0, when no satisfying assignment has been found after exhaustively searching,
	satisfied,   //1, when a satisfying assignment has been found
    normal,      // 2,到目前为止没有找到可满足赋值，并且DPLL()已正常退出
	completed    // 3,当DPLL算法完成执行时
};

class solver{
public: 
    //constructor function initialize literals and literal_frequency;output = 1
    solver(const CircuitGraph&);
    //solve process
    void solve(const CircuitGraph&);
    void test(const CircuitGraph&);

private:
    //存储lines的赋值，其中-1 - unassigned；0 - false； 1 - true
    std::unordered_map<int,int> lines_status_num; 
    std::vector<int> sort_destination_gates;
    std::vector<int> the_name_of_input_line;
    cnf m_cnf;
    //common solving operations both cnf and circuit
    int FindDecisionTarget(std::unordered_map<int,int>&,const CircuitGraph &graph );
    int DPLL(const CircuitGraph&,int );
    int BCP(const CircuitGraph&,int );
    // only for circuit solving 
    bool SingleGateReasonBoost(Gate *current_gate, std::queue<int>&bcp_que, int reason_line_name);
    // only for cnf solving 
    int unit_propagate(cnf &f, std::queue<int>&bcp_que,int reason_line_name);  //performs unit propagation
    int apply_transform(cnf &f, std::queue<int>&bcp_que,int reason_line_name); // applies the value of the literal in


    void show_result(const CircuitGraph&,int );     
    void print_lines_status_num(const CircuitGraph&);  
};
#endif