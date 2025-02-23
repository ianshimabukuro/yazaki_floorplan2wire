#include "base/graph.h"
#include "algorithms/mbsp.h"
namespace ewd
{
    class DecompositionApproach
    {
    protected:
        GeometricGraph& g_;

    public:
        DecompositionApproach(GeometricGraph& g): g_(g) {}
        ~DecompositionApproach() {} 

        size_t PSB;
        std::vector<size_t> devices;
        std::vector<std::vector<size_t>> paths;
        CostBend obj;
        void solve(bool use_mst = true);
    };
}