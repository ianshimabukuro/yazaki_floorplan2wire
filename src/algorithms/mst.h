#pragma once
#include "base/graph.h"
#include "algorithms/mbsp.h"
namespace ewd
{
    std::vector<size_t> PrimMinimumSpanningTree(
        const ewd::EssentialGraph &g,
        const std::vector<CostBend>& weights, 
        double REL_ERR=0.01);
} 
