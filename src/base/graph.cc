#include "base/graph.h"
#include <set>
#include <queue>
#include <cmath>
#include <iostream>
#include <algorithm>
using namespace std;

namespace ewd
{

	void Graph::set_vertex_num(VertexIndex num_vertex)
	{
		num_vertex_ = num_vertex;
		adj_list_.resize(num_vertex_);
	}

	EdgeIndex Graph::add_edge(VertexIndex i, VertexIndex j, double weight)
	{
		if(max(i, j) + 1 > num_vertex_)
			set_vertex_num(max(i, j) + 1);
		edges_.push_back(Edge(i, j));
		weights_.push_back(weight);
		adj_list_[i].push_back(num_edge_);
		adj_list_[j].push_back(num_edge_);
		num_edge_++;
		return num_edge_ - 1;
	}

	// EdgeIndex Graph::add_edge(const Edge &e, double weight)
	// {
	// 	if(max(e.first, e.second) + 1 > num_vertex_)
	// 		set_vertex_num(max(e.first, e.second) + 1);
	// 	edges_.push_back(e);
	// 	weights_.push_back(weight);
	// 	adj_list_[e.first].push_back(num_edge_);
	// 	adj_list_[e.second].push_back(num_edge_);
	// 	num_edge_++;
	// 	return num_edge_ - 1;
	// }

	EdgeIndex Graph::find_edge(const Edge &e) const
	{
		// for (EdgeIndex k = 0; k < edges_.size(); k++)
		// {
		// 	if (edges_[k].first == e.first && edges_[k].second == e.second)
		// 		return k;
		// 	else if (edges_[k].first == e.second && edges_[k].second == e.first)
		// 		return k;
		// }
		for(EdgeIndex k:adj_list_[e.first])
		{
			if (edges_[k].first == e.first && edges_[k].second == e.second)
				return k;
			else if (edges_[k].first == e.second && edges_[k].second == e.first)
				return k;
		}
		return edges_.size();
	}

	EdgeIndex Graph::find_edge(VertexIndex i, VertexIndex j) const
	{
		for (EdgeIndex k = 0; k < edges_.size(); k++)
		{
			if (edges_[k].first == i && edges_[k].second == j)
				return k;
			else if (edges_[k].first == j && edges_[k].second == i)
				return k;
		}
		return edges_.size();
	}

	void Graph::remove_edge(EdgeIndex k)
	{
		if (k >= edges_.size())
			return;
		VertexIndex i = edges_[k].first;
		VertexIndex j = edges_[k].second;
		adj_list_[i].erase(find(adj_list_[i].begin(), adj_list_[i].end(), k));
		adj_list_[j].erase(find(adj_list_[j].begin(), adj_list_[j].end(), k));
		for (size_t v = 0; v < num_vertex_; v++)
		{
			for (auto &l : adj_list_[v])
			{
				if (l > k)
					l--;
			}
		}
		edges_.erase(edges_.begin() + k);
		weights_.erase(weights_.begin()+k);
		num_edge_--;
	}

	vecIndex Graph::GetAdjacentEdges(VertexIndex v) const
	{
		if (v <= num_vertex())
			return adj_list_[v];
		return {};
	}

	std::map<size_t, double> Graph::reachable_neighbors(size_t v) const 
	{
		map<size_t, double> out;
		if(v>= num_vertex_) return out;
		for(size_t k:adj_list_[v])
		{
			size_t u = opposite(v,k);
			if(out.find(u) == out.end() ||out[u] > weights_[k])
			{
				out[u] = weights_[k];
			}
		}
		return out;
	}

	bool Graph::connected() const
	{
		set<VertexIndex> vs;
		for (VertexIndex i = 0; i < num_vertex_; i++)
			vs.insert(i);
		return check_connected(vs);
	}

	bool Graph::check_connected(set<VertexIndex>& vs) const
	{
		vector<bool> visited(num_vertex_, false);

		queue<VertexIndex> q;
		q.push(*vs.begin());
		visited[*vs.begin()] = true;
		vs.erase(vs.begin());
		while (!vs.empty())
		{
			if(q.empty())
			return false;
			VertexIndex v = q.front();
			q.pop();
			for(const auto& k: adj_list_[v] )
			{
				VertexIndex j = opposite(v, k);
				if(!visited[j])
				{
					q.push(j);
					visited[j] = true;
					vs.erase(j);
				}
			}
		}

		return vs.empty();
	}

	GeometricGraph::GeometricGraph() : Graph()
	{
		num_vertex_ = 0;
		num_edge_ = 0;
		vertex_.clear();
		edges_.clear();
		adj_list_.clear();
	}
	GeometricGraph::~GeometricGraph() {}
	GeometricGraph::GeometricGraph(const GeometricGraph &g)
	{
		num_vertex_ = g.num_vertex_;
		num_edge_ = g.num_edge_;
		vertex_ = g.vertex_;
		edges_ = g.edges_;
		adj_list_ = g.adj_list_;
		REL_ERR_ = g.REL_ERR_;
		ABS_ERR_ = g.ABS_ERR_;
		WEAK_PARALLEL_ERR_ = g.WEAK_PARALLEL_ERR_;
	}


	VertexIndex GeometricGraph::add_vertex(const Point &pnt)
	{
		size_t i = find_vertex(pnt);
		if (i == vertex_.size())
		{
			vertex_.push_back(pnt);
			adj_list_.push_back(vector<size_t>());
			num_vertex_++;
		}
		return i;
	}

	VertexIndex GeometricGraph::add_vertex_simply(const Point &pnt)
	{
		size_t n = vertex_.size();
		vertex_.push_back(pnt);
		adj_list_.push_back(vector<size_t>());
		num_vertex_++;
		return n;
	}

	EdgeIndex GeometricGraph::add_edge(VertexIndex i, VertexIndex j, double weight)
	{
		if (i == j)
			return edges_.size();
		if (max(i, j) + 1 > num_vertex_)
			return edges_.size();
		size_t k = find_edge(i, j);
		if (k == edges_.size())
			Graph::add_edge(i, j, weight);
		return k;
	}

	void GeometricGraph::add_edge_safely(VertexIndex i, VertexIndex j)
	{
		if (i == j)
		{
			return;
		}
		size_t k = find_edge(i, j);
		if (k < edges_.size())
			return;

		Point ei = vertex_[i];
		Point ej = vertex_[j];
		Point u = ej - ei;
		vector<size_t> inedgeVers;
		vector<double> dist2ei;
		vector<size_t>::iterator itiv;
		vector<double>::iterator itde;
		for (size_t v = 0; v < vertex_.size(); v++)
		{
			if (v == i || v == j)
			{
				continue;
			}
			Point w = vertex(v) - ei;
			if (w.IsParallel(u,REL_ERR_))
			{
				double proj = w / u;
				if (REL_ERR_ < proj && proj < 1 - REL_ERR_)
				{
					for (itiv = inedgeVers.begin(), itde = dist2ei.begin(); itde != dist2ei.end(); itiv++, itde++)
					{
						if (*itde > proj)
							break;
					}
					inedgeVers.insert(itiv, v);
					dist2ei.insert(itde, proj);
				}
			}
		}
		inedgeVers.push_back(j);
		size_t start = i;
		for (size_t v : inedgeVers)
		{
			add_edge(start, v, vertex_[start].distance(vertex_[v]));
			start = v;
		}
	}

	EdgeIndex GeometricGraph::add_edge_simply(VertexIndex i, VertexIndex j, double weight)
	{
		if (i == j)
		{
			return edges_.size();
		}
		size_t k = edges_.size();
		Edge e = {i, j};
		edges_.push_back(e);
		weights_.push_back(weight);
		adj_list_[i].push_back(k);
		adj_list_[j].push_back(k);
		num_edge_++;
		return k;
	}

	EdgeIndex GeometricGraph::find_edge(const Point &pnt1, const Point &pnt2) const
	{
		size_t i = find_vertex(pnt1);
		size_t j = find_vertex(pnt2);
		return find_edge(i, j);
	}

	Point GeometricGraph::get_edge_direc(EdgeIndex k, bool normalized) const
	{
		Point u = vertex_[edges_[k].second] - vertex_[edges_[k].first];
		if (normalized)
			return u.normalized();
		return u;
	}

	bool GeometricGraph::IsPntLieInEdge(const Point &pnt, EdgeIndex k) const
	{
		Point e0 = vertex_[edges_[k].first];
		Point e1 = vertex_[edges_[k].second];

		// Edge is short and the point is closed to it
		if (pnt.distance(e0) < REL_ERR_ && pnt.distance(e1) < REL_ERR_)
			return true;
		if (e0.distance(e1) < REL_ERR_)
			return false;
		Point u = e1 - e0;
		Point w = pnt - e0;
		if (u.IsParallel(w,REL_ERR_) && REL_ERR_ < w / u && 1 - REL_ERR_ > w / u)
			return true;
		return false;
	}

	VertexIndex GeometricGraph::BreakEdgeWithPnt(const Point &pnt, EdgeIndex k)
	{
		size_t n = add_vertex(pnt);
		size_t e0 = edges_[k].first, e1 = edges_[k].second;
		if (n == e0 || n == e1)
		{
			return n;
		}
		remove_edge(k);
		add_edge(e0, n, vertex_[e0].distance(vertex_[n]));
		add_edge(e1, n, vertex_[e1].distance(vertex_[n]));
		return n;
	}

	VertexIndex GeometricGraph::BreakEdgeWithNewPnt(const Point &pnt, EdgeIndex k)
	{
		size_t n = add_vertex_simply(pnt);
		size_t e0 = edges_[k].first, e1 = edges_[k].second;
		remove_edge(k);
		add_edge_simply(e0, n, vertex_[e0].distance(vertex_[n]));
		add_edge_simply(e1, n, vertex_[e1].distance(vertex_[n]));
		return n;
	}

}