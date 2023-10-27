#include <iostream>
#include "parser.h"
#include "qap.h"
#include "bab.h"

void print_matrix(qap::Mat& mat)
{
    for(auto& i: mat)
    {
        for(auto& j: i)
        {
            std::cout << "\t"<< j;
        }
        std::cout  << std::endl;
    }
    std::cout  << std::endl;
}

int main()
{
    qap::QAP qap{};
    qap::Parser parser{};
    parser.parse("../data/data.txt", qap.distance, qap.flow, qap.n);
    std::cout << "Loaded mat size: " << qap.n << std::endl;
    
    auto solution = qap::BranchAndBound::solve(qap);

    std::cout << "Best cost: " << solution.cost << std::endl;
    for(auto& value : solution.permutation)
    {
        std::cout <<  value << std::endl;
    }

    return 0;
}