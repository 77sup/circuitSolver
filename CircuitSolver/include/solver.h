#include "circuit_graph.h"
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
    std::unordered_map<std::string,int> lines_status; 
    std::unordered_map<int,int> lines_status_num; 
    std::vector<int> sort_destination_gates;
    std::vector<int> the_name_of_input_line;
    int DPLL(const CircuitGraph&,int );
    int BCP(const CircuitGraph&,int );
    bool SingleGateReasoning(Gate *current_gate, std::queue<int>&bcp_que, int reason_line_name);
    bool SingleGateReasonBoost(Gate *current_gate, std::queue<int>&bcp_que, int reason_line_name);
    int FindDecisionTarget(std::unordered_map<int,int>&,const CircuitGraph &graph );

    void show_result(const CircuitGraph&,int );               
};
#endif