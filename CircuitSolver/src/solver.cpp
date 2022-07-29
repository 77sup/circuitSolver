#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include"../include/cnf.h"
#include <vector>
#include<queue>
#include <cmath>
solver::solver(const CircuitGraph &graph)
{
    std::vector<int> noPO_lines_name;  // store no-PIs
    std::vector<int> output;          // store PIs
    std::cout<<"the number of all lines:";
    std::cout << graph.get_name_to_line().size() << std::endl;
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
        lines_status_num.insert(std::make_pair(noPO_lines_name[i], -1));
        // find PIs to store
        if (!graph.get_name_to_line().at(noPO_lines_name[i])->source)
        {
            the_name_of_input_line.push_back(noPO_lines_name[i]);
        }
    }
    // assign POs to 1
    for (int i = 0; i < output.size(); i++)
    {
        lines_status_num.insert(std::make_pair(output[i], 1));
    }
    
    // according to fan_outs numbers to order(max---min)
    int noPO_lines_name_size=noPO_lines_name.size();
    for (int i = 0; i < noPO_lines_name_size; i++)
    {    //put (fanouts>3 || inputs) lines into sort_destination_gates
        if((graph.get_name_to_line().at(noPO_lines_name[i])->destination_gates.size()>3)||
        (!graph.get_name_to_line().at(noPO_lines_name[i])->source))
        {
            sort_destination_gates.push_back(noPO_lines_name[i]);
        }
    }
    m_cnf.initialize();
}
// choose a line to assign(decision),according to ordered fan_outs numbers
int solver::FindDecisionTarget(std::unordered_map<int, int> &lines_status_num,const CircuitGraph &graph)
{
    int Target=-1;
    int max_fanouts=0;
    for (int i = 0; i < sort_destination_gates.size(); i++)
    {
        if(lines_status_num.at(sort_destination_gates[i])==-1 && graph.get_name_to_line().at(sort_destination_gates[i])->destination_gates.size()>max_fanouts)
        {
            Target=sort_destination_gates[i];
            max_fanouts=graph.get_name_to_line().at(sort_destination_gates[i])->destination_gates.size();

        }
    }
    return Target;
}
void solver::solve(const CircuitGraph& graph)
{
    int bcp_result;
    for (int i = 0; i < graph.get_outputs().size(); i++)
    {
        bcp_result=BCP(graph, graph.get_outputs()[i]->num_name);
        if(bcp_result==0) break;
    }
    std::cout<<"bcp_result: "<<bcp_result<<std::endl;
    if(bcp_result==1)
    {
        std::cout<<"when bcp_result equal to 1, enter DPLL"<<std::endl;
        int dpll_result = DPLL(graph,-1);
        if (!dpll_result)
        show_result(graph, 0);
    }
    std::cout<<"after DPLL operation"<<std::endl;
}

int solver::DPLL(const CircuitGraph& graph,int decision_line ) //return 0---unsat;return 1---sat,but this not related to solver's solution
{
    int bcp_result=BCP(graph,decision_line);
    if(bcp_result==0)  return 0;
    int next_decision_line=FindDecisionTarget(lines_status_num,graph);
    if(next_decision_line==-1)   //all nodes were assigned
    {
        show_result(graph,1);
        return 1;  //solution is SAT
    }
    int flag=rand()%2;  //randomly choose left or right node to decide assignment
    for(int i=0;i<2;i++)  //Traverse two child nodes
    {
        int flag2;
        if(flag)  //1,right node
        {
            if(i) flag2 = 0;
            else flag2 = 1;
        }
        else
        {
            if(i) flag2 = 1;
            else flag2 = 0;
        }
        solver CircuitSolver = *this;
        CircuitSolver.lines_status_num.at(next_decision_line) = flag2;
        int dpll_result = CircuitSolver.DPLL(graph, next_decision_line);
        if (dpll_result == 1)
            return 1;
    }
    return 0;  //solution is UNSAT
}

int solver::BCP(const CircuitGraph &graph,int decision_line)
{
    if(decision_line==-1) return 1;
    std::queue<int>bcp_que;              
    bcp_que.push(decision_line);
    int head_line_que=bcp_que.front();
    std::vector<Gate*> line_connection_gates;  
    //std::cout<<"1111"<<std::endl;
    apply_transform(m_cnf, bcp_que,head_line_que);
   // std::cout<<"2222"<<std::endl;

    while (!bcp_que.empty())
    {
        unit_propagate(m_cnf,bcp_que,head_line_que);
        line_connection_gates.clear();
        //add the source gate
        if(graph.get_name_to_line().at(head_line_que)->source)
        {
            line_connection_gates.push_back(graph.get_name_to_line().at(head_line_que)->source);
        }
        //add the back gates
        for(auto it=graph.get_name_to_line().at(head_line_que)->destination_gates.begin();it!=graph.get_name_to_line().at(head_line_que)->destination_gates.end();it++)
        {
            line_connection_gates.push_back(*it);
        }
        for(int j=0;j<line_connection_gates.size();j++)
        {
            if(!SingleGateReasonBoost(line_connection_gates[j], bcp_que, head_line_que)) return 0; //SingleGateReasoning fail
        }
        bcp_que.pop();
        head_line_que=bcp_que.front();
    }
    return 1; 
}

void solver::show_result(const CircuitGraph& graph, int dpll_result)
    {
        if(dpll_result)
        {
            std::cout<<"SAT"<<std::endl;
            for(int i=0;i<graph.get_lines().size();i++){
                int line_name=graph.get_lines()[i].num_name;
                std::cout<<line_name<<"  "<<lines_status_num.at(line_name)<<std::endl;
            }
        }
        else
            std::cout<<"UNSAT"<<std::endl;
    }



