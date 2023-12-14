#include "decomposition_approach.h"
#include "algorithms/mst.h"
using namespace std;
using namespace ewd;

void DecompositionApproach::solve()
{
    if(devices.empty())
        return;

    // Path Generation 
    vector<CostBend> dist0;
    vector<vector<CostBend>> dist;
    MinBendShortestPath mbsp(g_);

    mbsp.solve(PSB);
    for(int i = 0; i < devices.size(); i++)
    {
        size_t iv = devices[i];
        dist0.push_back(CostBend(mbsp.distance(iv), mbsp.num_bend(iv),1e-2));
    }

    for(int i = 0; i < devices.size(); i++)
    {
        dist.push_back({});
        size_t iv = devices[i];
        mbsp.solve(iv);
        for(int j=0;j<devices.size(); j++)
        {
            size_t jv = devices[j];
            dist[i].push_back(CostBend(mbsp.distance(jv), mbsp.num_bend(jv), 1e-2));
        }
    }

    // Path Selection

    int a0k=0;
    CostBend a0mincb(dist0[0]);
    for(int k=1;k<devices.size();k++)
    {
        if(dist0[k] < a0mincb)
        {
            a0k = k;
            a0mincb = dist0[k];
        }
    }

    mbsp.solve(PSB);
    paths.push_back(mbsp.get_path(devices[a0k]));
    obj = a0mincb;

    if(devices.size()>1)
    {
        Graph h;
        h.set_vertex_num(devices.size());
        vector<CostBend> weights;
        for (int i = 0; i < devices.size(); i++)
        {
            for (int j = i + 1; j < devices.size(); j++)
            {
                h.add_edge(i, j);
                weights.push_back(dist[i][j]);
            }
        }
        auto mst_h = PrimMinimumSpanningTree(h, weights, 1e-2);

        for (auto ek : mst_h)
        {
            auto e = h.edge(ek);
            int i = e.first, j = e.second;
            mbsp.solve(devices[i]);
            paths.push_back(mbsp.get_path(devices[j]));
            obj += dist[i][j];
        }
    }
}