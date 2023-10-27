#pragma once
#include <vector>
#include <algorithm>
#include "qap.h"

namespace qap
{
    struct State
    {
        int cost;
        std::vector<int> assignment;
        int level;

        State(int n) : cost(0), assignment(n), level(0) {}
    };

    // Function to compute the lower bound for the current state
    int computeLowerBound(const QAP &qap, const State &state)
    {
        int bound = 0;
        for (int i = 0; i < state.level; ++i)
        {
            for (int j = 0; j < state.level; ++j)
            {
                bound += qap.distance[i][state.level] * qap.flow[state.assignment[i]][state.assignment[state.level]];
            }
        }
        return bound;
    }

    // Recursive branch and bound algorithm
    int branchAndBound(const QAP &qap, State &state, int &bestCost)
    {
        if (state.level == qap.n)
        {
            // Leaf node reached, update the best cost
            bestCost = std::min(bestCost, state.cost);
            return state.cost;
        }

        int lowerBound = computeLowerBound(qap, state);
        if (lowerBound >= bestCost)
        {
            // Prune the subtree if lower bound is not promising
            return INF;
        }

        int minCost = INF;
        for (int i = 0; i < qap.n; ++i)
        {
            if (std::find(state.assignment.begin(), state.assignment.end(), i) == state.assignment.end())
            {
                state.assignment[state.level] = i;
                state.cost += qap.distance[state.level][i] * qap.flow[state.assignment[state.level]][state.level];
                state.level++;

                int cost = branchAndBound(qap, state, bestCost);
                minCost = std::min(minCost, cost);

                state.level--;
                state.cost -= qap.distance[state.level][i] * qap.flow[state.assignment[state.level]][state.level];
            }
        }

        return minCost;
    }

    int solveQAP(const QAP &qap)
    {
        int n = qap.n;
        int bestCost = INF;

        State state(n);
        int finalCost = branchAndBound(qap, state, bestCost);

        return bestCost;
    }
}