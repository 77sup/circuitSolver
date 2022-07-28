#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include"../include/cnf.h"
#include <vector>
#include<queue>
solver::solver(const CircuitGraph &graph)
{
    // std::vector<std::string> noPI_lines_name; // store no-PIs
    std::vector<int> noPO_lines_name;  // store no-PIs
    std::vector<int> output;          // store PIs
    std::cout<<"the number of all lins:";
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
    cnfbcp.initialize();
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
    std::cout << "solve2" << std::endl;
    if(bcp_result==1)
    {
        int dpll_result = DPLL(graph,-1);
        if (!dpll_result)
        show_result(graph, 0);
    }
    std::cout << "solve3" << std::endl;
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
    int flag=rand()%2;
    for(int i=0;i<2;i++)  //Traverse two child nodes
    {
        int flag2;
        if(flag)
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
    while (!bcp_que.empty())
    {
        cnfbcp.cnf_BCP(cnfbcp.formula);
        
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
//design for multiple inputs gates
bool solver::SingleGateReasonBoost(Gate* current_gate,std::queue<int>&bcp_que, int reason_line_name)
{
    Gate::Type GateType = current_gate->get_type();
    std::vector<std::pair<int, int>> all_lines_current_gate;
    for(int i=0;i<current_gate->get_inputs().size();i++)
    {
        int input=current_gate->inputs()[i]->num_name;
        all_lines_current_gate.push_back(std::make_pair(input,lines_status_num.at(input)));
    }
    all_lines_current_gate.push_back(std::make_pair(current_gate->get_output()->num_name,lines_status_num.at(current_gate->get_output()->num_name)));
    int number_lineOfGate=all_lines_current_gate.size();
    //respectively record input line's 0 1 x number 
    int input_line_status[3]={0,0,0};
    for(int i=0;i<number_lineOfGate-1;i++)
    {
        if(all_lines_current_gate[i].second==0)
        {
            input_line_status[0]++;
        }
        else if(all_lines_current_gate[i].second==1)
        {
            input_line_status[1]++;
        }
        else{
            input_line_status[2]++;
        }
    }
    int output_line_status=all_lines_current_gate[number_lineOfGate-1].second;
    switch (GateType)
    {
    case Gate::Type::And:
    {
        if((output_line_status==0&&input_line_status[1]==number_lineOfGate-1)||(output_line_status==1&&input_line_status[0]>0))
        {
            return false;  //conflict
        }
        else if(output_line_status==0&&input_line_status[2]==1&&input_line_status[1]==number_lineOfGate-2)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=0;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if(output_line_status==1&&input_line_status[2]>0)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }

        }
        else if(output_line_status==-1&&input_line_status[0]>0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&input_line_status[1]==number_lineOfGate-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else
        {
            return true; //nothing can be reasoned
        }
    }
    case Gate::Type::Nand:
    {
        if((output_line_status==1&&input_line_status[1]==number_lineOfGate-1)
        ||(output_line_status==0&&input_line_status[0]>0))
        {
            return false;  //conflict
        }
        else if(output_line_status==1&&input_line_status[2]==1&&input_line_status[1]==number_lineOfGate-2)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=0;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if(output_line_status==0&&input_line_status[2]>0)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if(output_line_status==-1&&input_line_status[0]>0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&input_line_status[1]==number_lineOfGate-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else
        {
            return true; //nothing can be reasoned
        }
    case Gate::Type::Or:
    {
        if((output_line_status==0&&input_line_status[1]>0)||
        (output_line_status==1&&input_line_status[0]==number_lineOfGate-1))
        {
            return false;  //conflict
        }
        else if(output_line_status==0&&input_line_status[2]>0)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=0;
                    bcp_que.push(all_lines_current_gate[i].first);
                }
            }
            return true;
        }
        else if(output_line_status==1&&input_line_status[0]==number_lineOfGate-2&&input_line_status[2]==1)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }

        }
        else if(output_line_status==-1&&input_line_status[0]==number_lineOfGate-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&input_line_status[1]>0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Nor:
    {
        if((output_line_status==1&&input_line_status[1]>0)
        ||(output_line_status==0&&input_line_status[0]==number_lineOfGate-1))
        {
            return false;  //conflict
        }
        else if(output_line_status==0&&input_line_status[0]==number_lineOfGate-2&&input_line_status[2]==1)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }

        }
        else if(output_line_status==1&&input_line_status[2]>0)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=0;
                    bcp_que.push(all_lines_current_gate[i].first);
                }
            }
                    return true;
        }
        else if(output_line_status==-1&&input_line_status[0]==number_lineOfGate-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&input_line_status[1]>0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Xor:  //two inputs
    {
        if((output_line_status==0&&input_line_status[0]>0&&input_line_status[1]>0)||
        (output_line_status==1&&(input_line_status[0]==2||input_line_status[1]==2)))
        {
            return false;
        }
        else if(output_line_status==0&&input_line_status[2]==1)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    if(input_line_status[0]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if(input_line_status[1]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }

        }
        else if(output_line_status==1&&input_line_status[2]==1)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    if(input_line_status[0]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if(input_line_status[1]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }

        }
        else if(output_line_status==-1&&input_line_status[0]>0&&input_line_status[1]>0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&(input_line_status[0]==2||input_line_status[1]==2))
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else
        {
            return true;
        }
        
    }
    case Gate::Type::Xnor:  //two inputs
    {
        if((output_line_status==1&&input_line_status[0]>0&&input_line_status[1]>0)||
        (output_line_status==0&&(input_line_status[0]==2||input_line_status[1]==2)))
        {
            return false;
        }
        else if(output_line_status==1&&input_line_status[2]==1)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    if(input_line_status[0]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if(input_line_status[1]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }

        }
        else if(output_line_status==0&&input_line_status[2]==1)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    if(input_line_status[0]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if(input_line_status[1]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }

        }
        else if(output_line_status==-1&&input_line_status[0]>0&&input_line_status[1]>0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&(input_line_status[0]==2||input_line_status[1]==2))
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else
        {
            return true;
        }
        
    }
    case Gate::Type::Not:
    {
        if((input_line_status[0]==1&&output_line_status==0)
        ||(input_line_status[1]==1&&output_line_status==1))
        {
            return false;
        }
        else if(input_line_status[0]==1&&output_line_status==-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(input_line_status[1]==1&&output_line_status==-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(input_line_status[2]==1)
        {
            lines_status_num.at(all_lines_current_gate[0].first)=1-all_lines_current_gate[1].second;
            bcp_que.push(all_lines_current_gate[0].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Buff:
    {
         if((input_line_status[0]==1&&output_line_status==1)
         ||(input_line_status[1]==1&&output_line_status==0))
        {
            return false;
        }
        else if(input_line_status[0]==1&&output_line_status==-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(input_line_status[1]==1&&output_line_status==-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(input_line_status[2]==1)
        {
            lines_status_num.at(all_lines_current_gate[0].first)=all_lines_current_gate[1].second;
            bcp_que.push(all_lines_current_gate[0].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Undefined:
    {
        
        return true;
    }
    default:
        return true;
    }

}
}

void solver::test(const CircuitGraph& graph)
{
    std::cout<<"cnf bcp call in solver"<<std::endl;
    cnfbcp.test();
}