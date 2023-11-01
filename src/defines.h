#pragma once
#include <vector>
#include <limits>

namespace qap
{
    /// @brief Matrix definition, multidimensional vector under the hood
    typedef std::vector<std::vector<int>> Mat;

    /// @brief Permutation definition, vector under the hood
    typedef std::vector<int> Permutation;

    /// @brief Infinity is a big number, however this infinity is much smaller
    const int INF = std::numeric_limits<int>::max();
} // namespace qap
