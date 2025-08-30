#include<iostream>
#include<fstream>
#include<string>
#include<regex>
#include<vector>
#include<unordered_map>

std::unordered_map<int,int> process_data();
float calculate_success_probability(std::unordered_map<int,int> data, int k);
float calculate_failure_probability(std::unordered_map<int,int> data, int k);
void plot(std::unordered_map<int,float> success_seq, std::unordered_map<int,float> failure_seq);

int main()
{
    std::unordered_map<int,int> data=process_data();

    std::unordered_map<int,float> success_seq;
    std::unordered_map<int,float> failure_seq;


    for(int k=-10;k<=10;k++)
    {
        float success_prob=calculate_success_probability(data,k);
        float failure_prob=calculate_failure_probability(data,k);
        success_seq[k]=success_prob;
        failure_seq[k]=failure_prob;
        
        std::cout<<"The success_prob under "<<k<<" is "<<success_prob<<std::endl;
        std::cout<<"The failure_prob under "<<k<<" is "<<failure_prob<<std::endl;
    }

    plot(success_seq,failure_seq);
}

std::unordered_map<int,int> process_data()
{
    std::ifstream file("../data/baidu.txt");

    if(!file.is_open())
    {
        std::cerr<<"File cannot be opened!"<<std::endl;
        std::unordered_map<int,int> empty_return;
        return empty_return;
    }

    std::string line;
    std::regex icmp_seq_regex(R"(icmp_seq=(\d+))");
    std::smatch match;
    std::vector<int> icmp_seq_vec;
    std::unordered_map<int,int> icmp_seq_map;

    while(std::getline(file,line))
    {
        if(std::regex_search(line,match,icmp_seq_regex))
        {
            int icmp_seq=std::stoi(match[1]);
            icmp_seq_map[icmp_seq]=1;
        }
    }

    for(int seq=1;seq<=18000;seq++)
    {
        if(icmp_seq_map.find(seq)==icmp_seq_map.end())
        {
            icmp_seq_map[seq]=0;
        }
    }

    return icmp_seq_map;
}

float calculate_success_probability(std::unordered_map<int,int> data,int k)
{
    if (k==0)
    {
        return 1;
    }

    else if (k>0)
    {
        int all_count=0;
        int success_count=0;

        for(size_t i=1;i<data.size()-k;i++)
        {
            if (data[i]==1)
            {
                all_count++;
                if (data[i+k]==1)
                {
                    success_count++;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }
        float success_probability=float(success_count)/all_count;
        return success_probability;
    }

    else if(k<0)
    {
        int all_count=0;
        int success_count=0;
        for (size_t i=-k;i<data.size();i++)
        {
            if (data[i]==1)
            {
                all_count++;
                if(data[i+k]==1)
                {
                    success_count++;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }
        float success_probability=float(success_count)/all_count;
        return success_probability;
    }
    return 0;

}

float calculate_failure_probability(std::unordered_map<int,int> data,int k)
{
    if (k==0)
    {
        return 1;
    }

    else if (k>0)
    {
        int all_count=0;
        int failure_count=0;

        for(size_t i=1;i<data.size()-k;i++)
        {
            if (data[i]==0)
            {
                all_count++;
                if (data[i+k]==0)
                {
                    failure_count++;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }
        float failure_probability=float(failure_count)/all_count;
        return failure_probability;
    }

    else if(k<0)
    {
        int all_count=0;
        int failure_count=0;
        for (size_t i=-k;i<data.size();i++)
        {
            if (data[i]==0)
            {
                all_count++;
                if(data[i+k]==0)
                {
                    failure_count++;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }
        float failure_probability=float(failure_count)/all_count;
        return failure_probability;
    }
    return 0;
}

void plot(std::unordered_map<int,float> success_seq, std::unordered_map<int,float> failure_seq)
{
    // 输出数据到CSV文件，用Python绘图
    std::ofstream outfile("autocorrelation_data.csv");
    outfile << "k,success_prob,failure_prob\n";
    
    for(int k = -10; k <= 10; k++)
    {
        float success_prob = success_seq[k];
        float failure_prob = failure_seq[k];
        outfile << k << "," << success_prob << "," << failure_prob << "\n";
    }
    outfile.close();
    
    std::cout << "数据已保存到 autocorrelation_data.csv" << std::endl;
    std::cout << "请运行Python脚本生成图形" << std::endl;
}