#include <iostream>
#include "parser.h"
#include "qap.h"
#include "bab.h"
#include "thread_pool.h"

/**
 * Matej Elias 2023
 *
 * Compile and run by executing:
 * make
 *
 * or hit f5 in VS Code
 */

int main()
{
    qap::QAP qap{};
    qap::Parser parser{};
    parser.parse("../data/data.txt", qap.distance, qap.flow, qap.n);
    std::cout << "Available threads: " << std::thread::hardware_concurrency() << std::endl;
    std::cout << "Loaded mat size: " << qap.n << std::endl;

    qap::BranchAndBound bab{};
    auto solution = bab.solve(qap);

    std::cout << "Best cost: " << solution.cost << "\nBest permutation: ";
    for (auto &value : solution.permutation)
    {
        std::cout << value << ", ";
    }
    std::cout << std::endl;

    return 0;
}