#pragma once
#include <vector>
#include <algorithm>
#include <limits>
#include <mutex>
#include "qap.h"
#include "thread_pool.h"

namespace qap
{
    /// @brief Struct used to encapsulate logic of solving a singular instance of QAP using BAB algorithm
    struct BranchAndBound
    {
        threading::ThreadPool thread_pool{}; // threadpool used to queue jobs
        std::mutex best_cost_mutex;
        std::mutex best_permutation_mutex;

        /// @brief Solution of the QAP
        struct Solution
        {
            Permutation permutation; // best permutation
            int cost;                     // cost of the best permutation
        };

        /// @brief Calculates the cost of a permutation
        /// @param problem instance of QAP problem
        /// @param permutation permutation for which the cost will be calculated
        /// @return returns the cost of the permutation
        inline int calculate_cost(QAP &problem, Permutation &permutation)
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
            Permutation &best_permutation,
            int &best_cost,
            Permutation &current_permutation,
            int level)
        {
            if (level == problem.n)
            {
                best_cost_mutex.lock();
                best_permutation_mutex.lock();
                int current_cost = calculate_cost(problem, current_permutation);
                if (current_cost < best_cost)
                {
                    best_cost = current_cost;
                    best_permutation = current_permutation;
                }
                best_permutation_mutex.unlock();
                best_cost_mutex.unlock();
            }
            else
            {
                for (int i = level; i < problem.n; ++i)
                {
                    std::swap(current_permutation[i], current_permutation[level]);
                    int new_cost = calculate_cost(problem, current_permutation);

                    best_cost_mutex.lock();
                    if (new_cost < best_cost)
                    {
                        best_cost_mutex.unlock();
                        branch_and_bound(
                            problem,
                            best_permutation,
                            best_cost,
                            current_permutation,
                            level + 1);
                    }
                    best_cost_mutex.unlock();
                    std::swap(current_permutation[i], current_permutation[level]);
                }
            }
        }

        /// @brief Threaded function for solving QAP problem using BAB algorithm
        /// @param problem instance of QAP problem
        /// @param best_permutation currently best permutation, top-most function on callstack sets this to best permutation over-all
        /// @param best_cost currently best cost, top-most function on callstack sets this to best cost over-all
        /// @param level level of deepness, starts at 0
        inline void branch_and_bound_threaded(
            QAP &problem,
            Permutation &best_permutation,
            int &best_cost,
            int level)
        {
            for (int i = level; i < problem.n; ++i)
            {
                thread_pool.queueJob(
                    [&, i, level]
                    {
                        Permutation current_permutation(problem.n);
                        for (int i = 0; i < problem.n; ++i)
                        {
                            current_permutation[i] = i;
                        }

                        std::swap(current_permutation[i], current_permutation[level]);
                        int new_cost = calculate_cost(problem, current_permutation);

                        best_cost_mutex.lock();
                        if (new_cost < best_cost)
                        {
                            best_cost_mutex.unlock();
                            branch_and_bound(
                                problem,
                                best_permutation,
                                best_cost,
                                current_permutation,
                                level + 1);
                        }
                        best_cost_mutex.unlock();
                    });
            }
        }

        /// @brief Helper function that starts the BAB computation and returns the best solution found
        /// @param problem instance of QAP problem
        /// @return solution of the QAP problem
        inline Solution solve(QAP &problem)
        {
            Permutation best_permutation(problem.n);
            int best_cost = qap::INF;

            thread_pool.start();

            branch_and_bound_threaded(problem, best_permutation, best_cost, 0);

            while (thread_pool.busy())
                continue;

            thread_pool.stop();

            return {
                .permutation = best_permutation,
                .cost = best_cost};
        }
    };

} // namepsace qap