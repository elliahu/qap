#pragma once
#include <vector>
#include "defines.h"


namespace qap
{
    /// @brief Describes a singular instance of QAP
    struct QAP
    {
        int n;          // number of factories
        Mat distance;   // distance matrix
        Mat flow;       // flow matrix
    };
    
} // namespace qap