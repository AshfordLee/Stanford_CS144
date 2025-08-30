#include<iostream>
#include<fstream>
#include<string>
#include<regex>
#include<vector>
#include<unordered_map>


int main()
{
    std::ifstream file("../data/baidu.txt");
    std::unordered_map<int, float> rtt_map;
    
    if(!file.is_open()) {
        std::cerr << "File cannot be opened!" << std::endl;
        return 1;
    }
    
    std::string line;
    std::regex icmp_seq_regex(R"(icmp_seq=(\d+))");
    std::regex rtt_regex(R"(time=(\d+\.?\d*) ms)");
    std::smatch seq_match, rtt_match;
    
    while(std::getline(file, line)) {
        if(std::regex_search(line, seq_match, icmp_seq_regex) && 
           std::regex_search(line, rtt_match, rtt_regex)) {
            
            int seq = std::stoi(seq_match[1]);
            float rtt = std::stof(rtt_match[1]);
            rtt_map[seq] = rtt;
        }
    }
    float max_value = 0.0f;
    for (const auto& pair:rtt_map)
    {
        max_value = std::max(max_value, pair.second);
    }

    std::cout<<"Mininum RTT: "<<max_value<<std::endl;

}