#pragma once
#include "base/types.h"
#include "base/point.h"
#include "base/types.h"
#include <vector>
#include <map>
#include <tuple>
#include <set>

namespace ewd
{
    using VertexIndex = size_t;
    using EdgeIndex = size_t;
    using Edge = std::pair<VertexIndex, VertexIndex>;

    class EssentialGraph
    {
    protected:
        size_t num_vertex_ = 0;
        size_t num_edge_ = 0;
        std::vector<Edge> edges_;
        vecDouble weights_;

    public:
        EssentialGraph() : num_vertex_(0), num_edge_(0) {}
        EssentialGraph(const EssentialGraph &g) : num_vertex_(g.num_vertex_),num_edge_(g.num_edge_), edges_(g.edges_){};
        ~EssentialGraph() {}

        size_t num_vertex() const { return num_vertex_; }
        size_t num_edge() const { return num_edge_; }
        virtual size_t add_edge(size_t v1, size_t v2, double weight) = 0;

        virtual EdgeIndex find_edge(VertexIndex v1, VertexIndex v2) const = 0;
        virtual EdgeIndex find_edge(const Edge &edge) const = 0;
    
        virtual std::map<size_t, double> reachable_neighbors(size_t v) const = 0;
        std::map<size_t, double> neighbors(size_t v) const {return reachable_neighbors(v);}

        virtual void remove_edge(EdgeIndex i) = 0;

        Edge edge(EdgeIndex k) const { return edges_[k]; }
        void set_edge_weight(EdgeIndex k, double w) {weights_[k] = w;}
        double weight(EdgeIndex k) const { return weights_[k];}
        double total_weight(const std::vector<EdgeIndex>& c) const 
        {
            double sum=0.0;
            for(size_t k:c) sum+=weights_[k];
            return sum;
        }
        double total_weight(const std::set<EdgeIndex>& c) const
        {
            double sum=0.0;
            for(size_t k:c) sum+=weights_[k];
            return sum;
        }

        VertexIndex opposite(VertexIndex v, EdgeIndex k) const
        {
            if (k >= edges_.size())
                return num_vertex_;
            if (edges_[k].first == v)
                return edges_[k].second;
            else if (edges_[k].second == v)
                return edges_[k].first;
            return num_vertex_;
        }

        virtual vecIndex GetAdjacentEdges(VertexIndex i) const = 0;
    };

    class Graph : public EssentialGraph
    {
    protected:
        std::vector<std::vector<EdgeIndex>> adj_list_;

    public:
        Graph() : EssentialGraph() {}
        Graph(const Graph &g) : EssentialGraph(g),
                                adj_list_(g.adj_list_) {}
        ~Graph() {}

        void set_vertex_num(VertexIndex num_vertex);
        EdgeIndex add_edge(VertexIndex v1, VertexIndex v2, double weight=1.0) override;
        EdgeIndex find_edge(const Edge &e) const;
        EdgeIndex find_edge(VertexIndex i, VertexIndex j) const;
        void remove_edge(EdgeIndex k);
        vecIndex GetAdjacentEdges(VertexIndex i) const override;
        std::map<size_t, double> reachable_neighbors(size_t v) const override;
        bool connected() const;
        bool check_connected(std::set<VertexIndex> &pnts) const;
    };

    class GeometricGraph : public Graph
    {
    protected:
        std::vector<Point> vertex_;
        double REL_ERR_ = 0.001;
        double ABS_ERR_ = 0.001;
        double WEAK_PARALLEL_ERR_ = 0.3;
    public:
        GeometricGraph();
        GeometricGraph(const GeometricGraph &g);
        ~GeometricGraph();

        double REL_ERR() const { return REL_ERR_; }
        double ABS_ERR() const { return ABS_ERR_; }
        double WEAK_PARALLEL_ERR() const { return WEAK_PARALLEL_ERR_; }

        void set_REL_ERR(double err) { REL_ERR_ = err; }
        void set_ABS_ERR(double err) { ABS_ERR_ = err; }
        void set_WEAK_PARALLEL_ERR(double err) { WEAK_PARALLEL_ERR_ = err; }

        VertexIndex add_vertex(const Point &pnt);
        VertexIndex add_vertex_simply(const Point &pnt);
        EdgeIndex add_edge(VertexIndex i, VertexIndex j, double weight = 1.0) override;
        void add_edge_safely(VertexIndex i, VertexIndex j);
        EdgeIndex add_edge_simply(VertexIndex i, VertexIndex j, double weight = 1.0);

        EdgeIndex find_edge(const Edge &e) const { return Graph::find_edge(e); }
        EdgeIndex find_edge(VertexIndex i, VertexIndex j) const { return Graph::find_edge(i, j); }
        EdgeIndex find_edge(const Point &pnt1, const Point &pnt2) const;
        Point get_edge_direc(EdgeIndex k, bool normalized = true) const;

        bool IsPntLieInEdge(const Point &pnt, EdgeIndex k) const;
        VertexIndex BreakEdgeWithPnt(const Point &pnt, EdgeIndex k);
        VertexIndex BreakEdgeWithNewPnt(const Point &pnt, EdgeIndex k);

        size_t find_vertex(const Point &p) const
        {
            for (size_t i = 0; i < vertex_.size(); i++)
            {
                if (vertex_[i].distance(p) < ABS_ERR_)
                    return i;
            }
            return vertex_.size();
        }
        Point vertex(size_t v) const 
        { 
            return vertex_[v]; 
        }
    };
}
