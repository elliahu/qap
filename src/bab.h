#pragma once
#include <vector>
#include <algorithm>
#include <limits>
#include "qap.h"
#include "thread_pool.h"

namespace qap
{
    /// @brief Struct used to encapsulate logic of solving a singular instance of QAP using BAB algorithm
    struct BranchAndBound
    {

        threading::ThreadPool threadPool{};

        /// @brief Solution of the QAP
        struct Solution
        {
            std::vector<int> permutation; // best permutation
            int cost;                     // cost of the best permutation
        };

        /// @brief Calculates the cost of a permutation
        /// @param problem instance of QAP problem
        /// @param permutation permutation for which the cost will be calculated
        /// @return returns the cost of the permutation
        inline int calculate_cost(QAP &problem, std::vector<int> &permutation)
        {
            int cost = 0;
            for (int i = 0; i < problem.n; ++i)
            {
                for (int j = 0; j < problem.n; ++j)
                {
                    cost += problem.distance[i][j] * problem.flow[permutation[i]][permutation[j]];
                }
            }
            return cost;
        }

        /// @brief Recursive function for solving QAP problem using BAB algorithm
        /// @param problem instance of QAP problem
        /// @param best_permutation currently best permutation, top-most function on callstack sets this to best permutation over-all
        /// @param best_cost currently best cost, top-most function on callstack sets this to best cost over-all
        /// @param current_permutation permutation being evaluated at the current call
        /// @param level level of deepness, starts at 0
        inline void branch_and_bound(
            QAP &problem,
            std::vector<int> &best_permutation,
            int &best_cost,
            std::vector<int> &current_permutation,
            int level)
        {
            if (level == problem.n)
            {
                int current_cost = calculate_cost(problem, current_permutation);
                if (current_cost < best_cost)
                {
                    best_cost = current_cost;
                    best_permutation = current_permutation;
                }
            }
            else
            {
                for (int i = level; i < problem.n; ++i)
                {
                    std::swap(current_permutation[i], current_permutation[level]);
                    int new_cost = calculate_cost(problem, current_permutation);
                    if (new_cost < best_cost)
                    {
                        branch_and_bound(
                            problem,
                            best_permutation,
                            best_cost,
                            current_permutation,
                            level + 1);
                    }
                    std::swap(current_permutation[i], current_permutation[level]);
                }
            }
        }

        /// @brief Helper function that starts the BAB computation and returns the best solution found
        /// @param problem isntance of QAP problem
        /// @return solution of the QAP problem
        inline Solution solve(QAP &problem)
        {
            std::vector<int> permutation(problem.n);
            for (int i = 0; i < problem.n; ++i)
            {
                permutation[i] = i;
            }
            std::vector<int> best_permutation(problem.n);
            int best_cost = std::numeric_limits<int>::max();

            
            threadPool.queueJob([&]{ branch_and_bound(problem, best_permutation, best_cost, permutation, 0); });
            threadPool.start();

            while(threadPool.busy()){continue;}

            return {
                .permutation = best_permutation,
                .cost = best_cost};
        }
    };

} // namepsace qap