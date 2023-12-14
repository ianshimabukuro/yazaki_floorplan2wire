#include "algorithms/mst.h"
#include "algorithms/argheap.h"

using namespace std;
#include <algorithm>
#include <limits>
namespace ewd
{
    vector<EdgeIndex> PrimMinimumSpanningTree(
        const ewd::EssentialGraph &g,
        const std::vector<CostBend>& weights, 
        double REL_ERR)
    {
        vector<EdgeIndex> treeset;
        
        VertexIndex v;
        CostBend total_weight(0.0,0,REL_ERR);
        ArgHeap<CostBend> h(vector<CostBend>(g.num_vertex(), CostBend(numeric_limits<double>::max(), numeric_limits<int>::max(), REL_ERR)));
        vector<EdgeIndex> pred_edges(g.num_vertex(), numeric_limits<EdgeIndex>::max());
        vector<bool> visited(g.num_vertex(), false);
        h.update(0, CostBend(0.0,0,REL_ERR));

        while (h.size() > 0)
        {
            v = h.pop();
            visited[v] = true;
            if (pred_edges[v] != numeric_limits<EdgeIndex>::max())
            {
                treeset.push_back(pred_edges[v]);
                total_weight += weights[pred_edges[v]];
            }
            for (auto e : g.GetAdjacentEdges(v))
            {
                if (!visited[g.opposite(v,e)])
                {
                    if (weights[e] < h.get(g.opposite(v,e)))
                    {
                        h.update(g.opposite(v,e), weights[e]);
                        pred_edges[g.opposite(v,e)] = e;
                    }
                }
            }
        }

        return treeset;
    }
}