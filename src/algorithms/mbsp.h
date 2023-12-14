#pragma once
#include "base/graph.h"
#include <limits>

namespace ewd
{
    class CostBend
    {
    public:
        double first;
        int second;
        double err=1e-6;
        CostBend(double cost=0.0, int bend=1) :first(cost),second(bend) {}
        CostBend(double cost, int bend, double e) :first(cost),second(bend),err(e) {}
        CostBend(const CostBend& r) :first(r.first),second(r.second),err(r.err) {}
        ~CostBend() {}

        bool operator<(const CostBend & right) const
        {
            if (first + err < right.first) return true;
            if (right.first + err < first) return false;
            return second < right.second;
        }
        bool operator==(const CostBend& r) const
        {
            return (fabs(first-r.first)<=err)&&(second == r.second);
        }
        bool operator>(const CostBend & right) const
        {
            return right < *this;
        }

        CostBend& operator=(const CostBend& r)
        {
            first = r.first;
            second = r.second;
            err = r.err;
            return *this;
        }

        CostBend& operator+=(const CostBend& r)
        {
            first += r.first;
            second += r.second;
            return *this;
        }

        CostBend operator+(const CostBend& r) const
        {
            CostBend res(*this);
            res += r;
            return res;
        }
    };


    class MinBendShortestPath
    {
    public:
        MinBendShortestPath(GeometricGraph& g):g_(g) {}
        ~MinBendShortestPath() {}

        size_t get_predecessor_num(size_t v) const;
        size_t predecessor(size_t v, size_t n_pred) const;
        double distance(size_t v) const;
        int num_bend(size_t v) const;
        bool get_path(size_t v, vecIndex& pathvec, size_t n_direc=0) const;
        vecIndex get_path(size_t v, size_t n_direc=0) const;

        void solve(size_t root, size_t expected_end_node = std::numeric_limits<size_t>::max());

        vecIndex predecessors(size_t v) const;

    private:
        GeometricGraph &g_;
        size_t root_;
        matIndex predecessors_;
        std::vector<CostBend> cb_;
        void reset();
    };

}
