#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include <vector>
#include<queue>
solver::solver(const CircuitGraph &graph)
{
    std::vector<std::string> noPI_lines_name; // store no-PIs
    //std::vector<int> noPI_lines_name;
    std::vector<std::string> output;          // store PIs
    std::cout << graph.get_name_to_line().size() << std::endl;
    for (int i = 0; i < graph.get_lines().size(); i++)
    {
        if (graph.get_lines()[i].is_output)
        {
            
            output.push_back(graph.get_lines()[i].name);
        }
        else
        {
            //int temp_name=graph.change_name(graph.get_lines()[i].name);
            //noPI_lines_name.push_back(temp_name);
            noPI_lines_name.push_back(graph.get_lines()[i].name);
        }
    }
    for (int i = 0; i < noPI_lines_name.size(); i++)
    {
        lines_status.insert(make_pair(noPI_lines_name[i], -1));
        // find PIs to store
        if (!graph.get_name_to_line().at(noPI_lines_name[i])->source)
        {
            the_name_of_input_line.push_back(noPI_lines_name[i]);
        }
    }
    // print the_name_of_input_line
    for (int i = 0; i < the_name_of_input_line.size(); i++)
    {
        std::cout << "the_name_of_input_line:" << the_name_of_input_line[i] << std::endl;
    }
    // assign POs to 1
    for (int i = 0; i < output.size(); i++)
    {
        lines_status.insert(make_pair(output[i], 1));
    }
    // test,print line's assign status
    /* std::cout << std::endl;
    for (auto it = lines_status.begin(); it != lines_status.end(); it++)
    {
        std::cout << "the_status_of_line:" << (*it).first << "    " << (*it).second << std::endl;
    } */
    // according to fan_outs numbers to order(max---min)
    std::string change;
    for (int i = 0; i < noPI_lines_name.size() - 1; i++)
    {
        for (int j = i + 1; j < noPI_lines_name.size(); j++)
        {
            if (graph.get_name_to_line().at(noPI_lines_name[i])->destination_gates.size() < graph.get_name_to_line().at(noPI_lines_name[j])->destination_gates.size())
            {
                change = noPI_lines_name[i];
                noPI_lines_name[i] = noPI_lines_name[j];
                noPI_lines_name[j] = change;
            }
        }
    }
    // store ordered noPI_lines
    for (int i = 0; i < noPI_lines_name.size(); i++)
    {
        sort_destination_gates.push_back(noPI_lines_name[i]);
    }
    // test
    /* for (int i = 0; i < sort_destination_gates.size(); i++)
    {
        std::cout << "sort_line_name:" << sort_destination_gates[i] << " num:  " << graph.get_name_to_line().at(sort_destination_gates[i])->destination_gates.size() << std::endl;
    } */
}
// choose a line to assign(decision),according to ordered fan_outs numbers
std::string solver::FindDecisionTarget(std::unordered_map<std::string, int> &lines_status)
{
    std::string Target;
    for (int i = 0; i < sort_destination_gates.size(); i++)
    {
        if (lines_status[sort_destination_gates[i]] == -1)
        {
            Target = sort_destination_gates[i];
            return Target;
        }
    }
    return "null";
}
void solver::test(const CircuitGraph &graph)
{
    this->lines_status.at(graph.get_lines()[2].name) = 0;
    std::cout << "Decision line name:   " << graph.get_lines()[2].name << std::endl;
    std::cout << "Decision line value:   " << lines_status.at(graph.get_lines()[2].name)<< std::endl;
    //std::vector<Gate *> line_connection_gates;
    std::cout<<BCP(graph,graph.get_lines()[2].name)<<std::endl;
    for (int i = 0; i < graph.get_lines().size(); i++)
    {
        std::cout << "line name:" << graph.get_lines()[i].name << "   assigned as:" << lines_status.at(graph.get_lines()[i].name)<< std::endl;
    }
}
void solver::solve(const CircuitGraph& graph)
{
    for(int i=0;i<graph.get_outputs().size();i++)
    {
        BCP(graph,graph.get_outputs()[i]->name);
        //std::cout<<BCP(graph,graph.get_outputs()[i]->name)<<std::endl<<std::endl;
    }
    int dpll_result=DPLL(graph,"null");
    if(!dpll_result) show_result(graph,0);
}
int solver::DPLL(const CircuitGraph& graph,std::string decision_line ) //return 0---unsat;return 1---sat,but this not related to solver's solution
{
    int bcp_result=BCP(graph,decision_line);
    if(bcp_result==0)  return 0;
    std::string next_decision_line=FindDecisionTarget(lines_status);
    if(next_decision_line=="null")   //all nodes were assigned
    {
        show_result(graph,1);
        return 1;  //solution is SAT
    }
    for(int i=0;i<2;i++)  //Traverse two child nodes
    {
        solver CircuitSolver=*this;  //kao bei yi ge duixiang
        CircuitSolver.lines_status.at(next_decision_line)=i; //update kao bei duixiang information
        int DPLL_result=CircuitSolver.DPLL(graph,next_decision_line);  
        if(DPLL_result) return 1;   //solution is SAT
    }
    return 0;  //solution is UNSAT
}

int solver::BCP(const CircuitGraph &graph,std::string decision_line)
{
    if(decision_line=="null") return 1;
    std::queue<std::string>bcp_que;              
    bcp_que.push(decision_line);
    std::string head_line_que=bcp_que.front();
    //SingleGateReasoning(Gate *current_gate, std::string reason_line_name);
    std::vector<Gate*> line_connection_gates;  
    while (!bcp_que.empty())
    {
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
bool solver::SingleGateReasoning(Gate *current_gate, std::queue<std::string>&bcp_que, std::string reason_line_name)
{
    int flag1 = 0;  // the number of  assigned lines
    int flag2 = -1; // judge gate's type; judge do/not do intersect
    int flag4 = -1; // to service for intersect's double loop
    Gate::Type GateType = current_gate->get_type();
    std::vector<std::pair<std::string, int>> all_lines_current_gate;
    for (int i = 0; i < current_gate->get_inputs().size(); i++)
    {
        std::string input = current_gate->get_inputs()[i]->name;
        all_lines_current_gate.push_back(std::make_pair(input, lines_status.at(input)));
    }
    all_lines_current_gate.push_back(std::make_pair(current_gate->get_output()->name, lines_status.at(current_gate->get_output()->name)));

    int assigned_line_index = -1;
    for (int i = 0; i < all_lines_current_gate.size(); i++)
    {
        if (all_lines_current_gate[i].second != -1)
        {
            flag1++;
        }
        if (reason_line_name == all_lines_current_gate[i].first)
        {
            assigned_line_index = i;
        }
    }
    switch (GateType)
    {
    case Gate::Type::And:
    {
        if (flag1 == 1 && assigned_line_index != all_lines_current_gate.size() - 1 && all_lines_current_gate[assigned_line_index].second == 0)
        {
            lines_status.at(all_lines_current_gate[all_lines_current_gate.size() - 1].first) = 0;
            bcp_que.push(all_lines_current_gate[all_lines_current_gate.size() - 1].first);
            return true;
        }
        else if ((flag1 == 1 && assigned_line_index == all_lines_current_gate.size() - 1 && all_lines_current_gate[assigned_line_index].second == 0) ||
                 (flag1 == 1 && assigned_line_index != all_lines_current_gate.size() - 1 && all_lines_current_gate[assigned_line_index].second == 1) ||
                 (flag1 == 2 && all_lines_current_gate[all_lines_current_gate.size() - 1].second == 0 && (all_lines_current_gate[0].second + all_lines_current_gate[1].second == -1)))
        {
            return true;
        }
        else
        {
            flag2 = 0;
            flag4 = 3;
            break;
        }
    }
    case Gate::Type::Nand:
    {
        if (flag1 == 1 && assigned_line_index != all_lines_current_gate.size() - 1 && all_lines_current_gate[assigned_line_index].second == 0)
        {
            lines_status.at(all_lines_current_gate[all_lines_current_gate.size() - 1].first) = 1;
            bcp_que.push(all_lines_current_gate[all_lines_current_gate.size() - 1].first);
            return true;
        }
        else if ((flag1 == 1 && all_lines_current_gate[assigned_line_index].second == 1) || (flag1 == 2 && all_lines_current_gate[all_lines_current_gate.size() - 1].second == 1 && (all_lines_current_gate[0].second + all_lines_current_gate[1].second == -1)))
        {
            return true;
        }
        else
        {
            flag2 = 1;
            flag4 = 3;
            break;
        }
    }
    case Gate::Type::Or:
    {    
        if (flag1 == 1 && assigned_line_index != all_lines_current_gate.size() - 1 && all_lines_current_gate[assigned_line_index].second == 1)
        {
            lines_status.at(all_lines_current_gate[all_lines_current_gate.size() - 1].first) = 1;
            bcp_que.push(all_lines_current_gate[all_lines_current_gate.size() - 1].first);
            return true;
        }
        else if ((flag1 == 1 && assigned_line_index != all_lines_current_gate.size() - 1 && all_lines_current_gate[assigned_line_index].second == 0) ||
                 (flag1 == 1 && assigned_line_index == all_lines_current_gate.size() - 1 && all_lines_current_gate[assigned_line_index].second == 1) ||
                 (flag1 == 2 && all_lines_current_gate[all_lines_current_gate.size() - 1].second == 1 && (all_lines_current_gate[0].second + all_lines_current_gate[1].second == 0)))
        {
            return true;
        }
        else
        {
            flag2 = 2;
            flag4 = 3;
            break;
        }
    }
    case Gate::Type::Nor:
    {
        if (flag1 == 1 && assigned_line_index != all_lines_current_gate.size() - 1 && all_lines_current_gate[assigned_line_index].second == 1)
        {
            lines_status.at(all_lines_current_gate[all_lines_current_gate.size() - 1].first) = 0;
            bcp_que.push(all_lines_current_gate[all_lines_current_gate.size() - 1].first);
            return true;
        }
        else if ((flag1 == 1 && assigned_line_index != all_lines_current_gate.size() - 1 && all_lines_current_gate[assigned_line_index].second == 0) ||
                 (flag1 == 1 && assigned_line_index == all_lines_current_gate.size() - 1 && all_lines_current_gate[assigned_line_index].second == 0) ||
                 (flag1 == 2 && all_lines_current_gate[all_lines_current_gate.size() - 1].second == 0 && (all_lines_current_gate[0].second + all_lines_current_gate[1].second == 0)))
        {
            return true;
        }
        else
        {
            flag2 = 3;
            flag4 = 3;
            break;
        }
    }
    case Gate::Type::Xor:
    {
        if (flag1 == 1)
            return true;
        else
        {
            flag2 = 4;
            flag4 = 4;
            break;
        }
    }
    case Gate::Type::Xnor:
    {
        if (flag1 == 1)
            return true;
        else
        {
            flag2 = 5;
            flag4 = 4;
            break;
        }
    }
    case Gate::Type::Not:
    {
        if (flag1 == 1)
        {
            if (all_lines_current_gate[0].second == -1)
            {
                lines_status.at(all_lines_current_gate[0].first) = 1 - all_lines_current_gate[1].second;
                 bcp_que.push(all_lines_current_gate[0].first);
            }
            else
            {
                lines_status.at(all_lines_current_gate[1].first) = 1 - all_lines_current_gate[0].second;
                bcp_que.push(all_lines_current_gate[1].first);
            }
            return true;
        }
        else
        {
            if (all_lines_current_gate[0].second + all_lines_current_gate[1].second != 1)
            {
                return false; // confilict
            }
        }
    }
    case Gate::Type::Buff:
    {
        if (flag1 == 1)
        {
            if (all_lines_current_gate[0].second == -1)
            {
                lines_status.at(all_lines_current_gate[0].first) = all_lines_current_gate[1].second;
                bcp_que.push(all_lines_current_gate[0].first);
            }
            else
            {
                lines_status.at(all_lines_current_gate[1].first) = all_lines_current_gate[0].second;
                bcp_que.push(all_lines_current_gate[1].first);
            }
            return true;
        }
        else
        {
            if (all_lines_current_gate[0].second + all_lines_current_gate[1].second == 1)
            {
                return false; // confilict
            }
        }
    }
    case Gate::Type::Undefined:
        return true;

    default:
        return false;
    }
    int lut[8][4][3] = {
        // look-up table
        /*与门*/ {{0, -1, 0}, {-1, 0, 0}, {1, 1, 1}},
        /*与非门*/ {{0, -1, 1}, {-1, 0, 1}, {1, 1, 0}},
        /*或门*/ {{0, 0, 0}, {-1, 1, 1}, {1, -1, 1}},
        /*或非门*/ {{0, 0, 1}, {-1, 1, 0}, {1, -1, 0}},
        /*异或门*/ {{0, 0, 0}, {0, 1, 1}, {1, 0, 1}, {1, 1, 0}},
        /*同或门*/ {{0, 0, 1}, {0, 1, 0}, {1, 0, 0}, {1, 1, 1}},
        /*非门*/ {{0, 1}, {1, 0}},
        /*缓冲器*/ {{0, 0}, {1, 1}},
    };
    //  intersect computation
    if (flag2 != -1)
    {
        std::vector<int> temp; // to temporarily store all_lines_current_gate's line assignmets
        int conflict = 1;
        for (int i = 0; i < flag4; i++)
        {
            int j = 0;
            temp.clear();
            for (int i = 0; i < all_lines_current_gate.size(); i++)
            {
                temp.push_back((all_lines_current_gate[i].second));
            }
            for (j = 0; j < 3; j++)
            {
                if (temp[j] == lut[flag2][i][j])
                    ; // consistent with look up table
                else if (temp[j] + lut[flag2][i][j] == 1)
                { // conflict
                    break;
                }
                else
                {
                    temp[j] = temp[j] + lut[flag2][i][j] + 1; // reason assignment
                }
            }
            if (j == 3)  //
            {
                for (int i = 0; i < temp.size(); i++)
                { // after reasoning, update  lines_status
                    if(lines_status.at(all_lines_current_gate[i].first)==-1 && temp[i]!=-1){ 
                        lines_status.at(all_lines_current_gate[i].first) = temp[i];
                        bcp_que.push(all_lines_current_gate[i].first);
                    }
                    
                }
                return true;
            }
        }
    }
    return false;
    
}
void solver::show_result(const CircuitGraph& graph, int dpll_result)
    {
        if(dpll_result)
        {
            std::cout<<"SAT"<<std::endl;
            for(int i=0;i<graph.get_lines().size();i++){
                std::string line_name=graph.get_lines()[i].name;
                std::cout<<line_name<<"  "<<lines_status.at(line_name)<<std::endl;
            }
        }
        else
            std::cout<<"UNSAT"<<std::endl;
    }
//design for multiple inputs gates
bool solver::SingleGateReasonBoost(Gate* current_gate,std::queue<std::string>&bcp_que, std::string reason_line_name)
{
    Gate::Type GateType = current_gate->get_type();
    std::vector<std::pair<std::string, int>> all_lines_current_gate;
    for(int i=0;i<current_gate->get_inputs().size();i++)
    {
        std::string input=current_gate->inputs()[i]->name;
        all_lines_current_gate.push_back(std::make_pair(input,lines_status.at(input)));
    }
    all_lines_current_gate.push_back(std::make_pair(current_gate->get_output()->name,lines_status.at(current_gate->get_output()->name)));
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
                    lines_status.at(all_lines_current_gate[i].first)=0;
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
                    lines_status.at(all_lines_current_gate[i].first)=1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }

        }
        else if(output_line_status==-1&&input_line_status[0]>0)
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&input_line_status[1]==number_lineOfGate-1)
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
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
                    lines_status.at(all_lines_current_gate[i].first)=0;
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
                    lines_status.at(all_lines_current_gate[i].first)=1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if(output_line_status==-1&&input_line_status[0]>0)
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&input_line_status[1]==number_lineOfGate-1)
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
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
                    lines_status.at(all_lines_current_gate[i].first)=0;
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
                    lines_status.at(all_lines_current_gate[i].first)=1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }

        }
        else if(output_line_status==-1&&input_line_status[0]==number_lineOfGate-1)
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&input_line_status[1]>0)
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
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
                    lines_status.at(all_lines_current_gate[i].first)=1;
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
                    lines_status.at(all_lines_current_gate[i].first)=0;
                    bcp_que.push(all_lines_current_gate[i].first);
                }
            }
                    return true;
        }
        else if(output_line_status==-1&&input_line_status[0]==number_lineOfGate-1)
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&input_line_status[1]>0)
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
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
                        lines_status.at(all_lines_current_gate[i].first)=0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if(input_line_status[1]>0)
                    {
                        lines_status.at(all_lines_current_gate[i].first)=1;
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
                        lines_status.at(all_lines_current_gate[i].first)=1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if(input_line_status[1]>0)
                    {
                        lines_status.at(all_lines_current_gate[i].first)=0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }

        }
        else if(output_line_status==-1&&input_line_status[0]>0&&input_line_status[1]>0)
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&(input_line_status[0]==2||input_line_status[1]==2))
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
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
                        lines_status.at(all_lines_current_gate[i].first)=0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if(input_line_status[1]>0)
                    {
                        lines_status.at(all_lines_current_gate[i].first)=1;
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
                        lines_status.at(all_lines_current_gate[i].first)=1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if(input_line_status[1]>0)
                    {
                        lines_status.at(all_lines_current_gate[i].first)=0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }

        }
        else if(output_line_status==-1&&input_line_status[0]>0&&input_line_status[1]>0)
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&(input_line_status[0]==2||input_line_status[1]==2))
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
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
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(input_line_status[1]==1&&output_line_status==-1)
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(input_line_status[2]==1)
        {
            lines_status.at(all_lines_current_gate[0].first)=1-all_lines_current_gate[1].second;
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
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(input_line_status[1]==1&&output_line_status==-1)
        {
            lines_status.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(input_line_status[2]==1)
        {
            lines_status.at(all_lines_current_gate[0].first)=all_lines_current_gate[1].second;
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