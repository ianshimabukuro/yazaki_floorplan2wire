#include "graph_constructor.h"
#include "algorithms/interval.h"
#include <math.h>
#include <algorithm>
#include <set>
#include <iostream>
#include <fstream>
#include <numeric>
#include <ctime>
#include <cmath>
#include "base/algorithm_error.h"
template <typename T1, typename T2>
constexpr auto MIN(T1 X, T2 Y) { return (Y) < (X) ? (Y) : (X); }
template <typename T1, typename T2>
constexpr auto MAX(T1 X, T2 Y) { return (Y) > (X) ? (Y) : (X); }

using namespace std;
namespace ewd
{
	const Point Zpos(0, 0, 1);
    const Point Ypos(0, 1, 0);
    const Point Xpos(1, 0, 0);

	size_t FindInOrderedVector(const std::vector<double> &vec, const double &val, size_t left, size_t right, double ABS_ERR)
	{
		if (left >= right)
			return left;
		if (fabs(vec[left] - val) <= ABS_ERR)
			return left;
		if (fabs(vec[right - 1] - val) <= ABS_ERR)
			return right - 1;
		if (right - left == 1)
		{
			if(vec[left] < val)
				return right;
			else
				return left;
		}
		size_t mid = (left + right) / 2;
		if (vec[mid] <= val - ABS_ERR)
			return FindInOrderedVector(vec, val, mid, right, ABS_ERR);
		else
			return FindInOrderedVector(vec, val, left, mid, ABS_ERR);
	}

	size_t FindInOrderedVector(const std::vector<double> &vec, const double &val, double ABS_ERR)
	{
		return FindInOrderedVector(vec, val, 0, vec.size(), ABS_ERR);
	}

	void RemoveIndexUpdate(set<size_t> &set2, size_t thres)
	{
		set<size_t> set1;
		for (auto &t : set2)
			if (t > thres)
				set1.insert(t);
		for (auto &t : set1)
			set2.erase(t);
		set2.erase(thres);
		for (auto &t : set1)
			set2.insert(t - 1);
	}

	void OrderedInsert(vector<double>& arr, double val, double ABS_ERR)
	{
		size_t t = FindInOrderedVector(arr, val, ABS_ERR);
		if(t<arr.size() && fabs(arr[t] - val) <= ABS_ERR) return;
		arr.insert(arr.begin() + t, val);
	}

	GraphConstructor::GraphConstructor() {g.set_ABS_ERR(ABS_ERR); }
	GraphConstructor::~GraphConstructor()
	{
	}

	void GraphConstructor::set_floor_height(double height) { this->floor_height = height; }
	void GraphConstructor::set_offset_door(double off) { offset_door = off; }

	void GraphConstructor::add_wall(const Wall &wl)
	{
		walls_.push_back(wl);
		walls_.back().set_offset(0.0);
		wall_id_map_[wl.get_id()] = walls_.size() - 1;
	}

	void GraphConstructor::add_door(const Door &wd) { doors_.push_back(wd); }
	size_t GraphConstructor::num_vertex() const { return g.num_vertex(); }
	size_t GraphConstructor::num_edge() const { return g.num_edge(); }
	Point GraphConstructor::vertex(size_t i) const { return g.vertex(i); }
	Edge GraphConstructor::edge(size_t k) const { return g.edge(k); }

	EdgeIndex GraphConstructor::add_edge(VertexIndex i, VertexIndex j, bool further_check)
	{
		Point v1(g.vertex(i)), v2(g.vertex(j));
		if(valid_point(v1) && valid_point(v2))
		{
			if(! LnThroughNotPass(v1,v2))
			{

				EdgeIndex k;
				if(further_check)
					k = g.add_edge(i, j);	
				else
					k=g.add_edge_simply(i, j);
				return k;
			}
		}
		return g.num_edge();
	}

	bool GraphConstructor::LnThroughNotPass(const Point &pnt0, const Point &pnt1, double offset, bool checkwindoor) const
	{
		for (const Wall &wl : walls_)
		{
			if ((!wl.allow_through()) || (fabs(wl.get_u() * (pnt1 - pnt0).normalized()) > 0.7))
			{
				auto rslt = wl.HousingIntersectLineSegment(pnt0, pnt1, floor_height);
				if (rslt.first==LineCuboidRelation::INTERSECTING && rslt.second.first <= rslt.second.second - ABS_ERR)
				{
					auto rslt = wl.HousingIntersectLineSegment(pnt0, pnt1, floor_height);
					double len = rslt.second.second - rslt.second.first;
					for (const Door &wd : doors_)
					{
						if (wd.get_host() != wl.get_id())
							continue;
						auto t_wd = wd.HousingIntersectLineSegment(pnt0, pnt1, floor_height);
						if (t_wd.first != LineCuboidRelation::DISJOINT)
							len -= t_wd.second.second - t_wd.second.first;
					}
					if (len > ABS_ERR)
						return true;
				}
			}
		}
		vector<size_t> related_wall_ind;
		for (size_t i = 0; i < walls_.size(); i++)
		{
			const Wall &wl = walls_[i];
			if ((!wl.allow_through()))
			{
				auto rslt = wl.HousingIntersectLineSegment(pnt0, pnt1, floor_height, offset + 2 * ABS_ERR);
				if (rslt.first==LineCuboidRelation::INTERSECTING && rslt.second.first <= rslt.second.second - ABS_ERR)
				{
					double len = rslt.second.second - rslt.second.first;
					for (const Door &wd : doors_)
					{
						if (wd.get_host() != wl.get_id())
							continue;
						auto t_wd = wd.HousingIntersectLineSegment(pnt0, pnt1, floor_height, offset);
						if (t_wd.first != LineCuboidRelation::DISJOINT)
							len -= t_wd.second.second - t_wd.second.first;
					}
					if (len > ABS_ERR)
						related_wall_ind.push_back(i);
				}
			}
		}
		vector<Point> movable_direcs;
		Point e = pnt1 - pnt0;
		if (fabs(e.x) > REL_ERR)
		{
			movable_direcs.push_back(Point(0, 1, 1));
			movable_direcs.push_back(Point(0, -1, 1));
			movable_direcs.push_back(Point(0, 1, -1));
			movable_direcs.push_back(Point(0, -1, -1));
		}
		if (fabs(e.y) > REL_ERR)
		{
			movable_direcs.push_back(Point(1, 0, 1));
			movable_direcs.push_back(Point(-1, 0, 1));
			movable_direcs.push_back(Point(1, 0, -1));
			movable_direcs.push_back(Point(-1, 0, -1));
		}
		if (fabs(e.z) > REL_ERR)
		{
			movable_direcs.push_back(Point(1, 1, 0));
			movable_direcs.push_back(Point(-1, 1, 0));
			movable_direcs.push_back(Point(1, -1, 0));
			movable_direcs.push_back(Point(-1, -1, 0));
		}
		for (size_t i : related_wall_ind)
		{
			const Wall &wl = walls_[i];
			wl.SubtleIntersectLine(pnt0, pnt1, floor_height, movable_direcs, 0);
		}
		if (movable_direcs.empty())
			return true;
		if (!checkwindoor)
			return false;
		for (const HouseInwallBarrier &br : doors_)
		{
			auto rslt= br.IntvIntersectLineSegment(pnt0, pnt1, floor_height, offset);
			if (rslt.first==LineCuboidRelation::INTERSECTING)
			{
				return true;
			}
		}
		return false;
	}

	map<LineCuboidRelation,double> GraphConstructor::intersection_analysis(const Point& pnt1, const Point& pnt2) const
	{
		vector<interval<double>> along_windoor, intersecting_solid, along_solid;
		map<LineCuboidRelation,double> out;
        out[LineCuboidRelation::DISJOINT] = pnt1.distance(pnt2);
        out[LineCuboidRelation::COINCIDENT] = 0.0;
        out[LineCuboidRelation::INTERSECTING] = 0.0;
		double addition_along_solid=0.0; // 绕梁产生
		for(size_t i=0;i<walls_.size();i++)
		{
			auto& wl = walls_[i];
			auto rslt = wl.HousingIntersectLineSegment(pnt1,pnt2,floor_height);
			if(rslt.first==LineCuboidRelation::DISJOINT) continue;
			if(wl.get_name().find("beam")!=string::npos)
			{
				addition_along_solid += pnt1.z + pnt2.z - wl.get_start().z*2;
				along_solid.push_back(rslt.second);
			}
			else 
			{
				if(rslt.first == LineCuboidRelation::INTERSECTING)
					intersecting_solid.push_back(rslt.second);
				else 
					along_solid.push_back(rslt.second);
			}
		}
		for(size_t i=0;i<doors_.size();i++)
		{
			auto& wd = doors_[i];
			auto rslt = wd.HousingIntersectLineSegment(pnt1,pnt2,floor_height);
			if(rslt.first==LineCuboidRelation::DISJOINT) continue;
			along_windoor.push_back(rslt.second);
		}

		intervals_union(along_windoor);
		intervals_union(along_solid);
		intervals_union(intersecting_solid);
		intervals_exclude(along_windoor, intersecting_solid);
		intervals_exclude(intersecting_solid, along_solid);
		intervals_union(along_solid);
		intervals_union(intersecting_solid);
		vector<interval<double>> alongs;
		for(auto in: along_windoor)
			alongs.push_back(in);
		for(auto in : along_solid)
			alongs.push_back(in);
		intervals_union(alongs);

		for(auto intv: intersecting_solid)
            out[LineCuboidRelation::INTERSECTING] += MAX(0, intv.second - intv.first);
        for(auto intv: alongs)
            out[LineCuboidRelation::COINCIDENT] += MAX(0,intv.second - intv.first);
        out[LineCuboidRelation::DISJOINT] += addition_along_solid - out[LineCuboidRelation::INTERSECTING] - out[LineCuboidRelation::COINCIDENT];
        return out;
	}


	vector<tuple<double, double, string>> GraphConstructor::GetCrossPoints(const Wall &wl, const Point &start, const Point &end, double offset, bool forcing) const
	{
		vector<tuple<double, double, string>> out;
		for (const Wall &wl2 : walls_)
		{
			if (wl2.get_id() == wl.get_id())
				continue;
			bool b = wl.IsIntersectBarrier(wl2, floor_height, offset);
			if (b && (!wl.get_u().IsParallel(wl2.get_u())))
			{
				auto rslt = wl2.HousingIntersectLineSegment(start,end,floor_height,offset,wl.get_thickness());
				double t1 = rslt.second.first, t2 = rslt.second.second;
				double thickness = wl.get_thickness() > wl2.get_thickness() ? wl.get_thickness() : wl2.get_thickness();

				if (rslt.first!=LineCuboidRelation::DISJOINT && t1 >= -thickness - ABS_ERR && t2 <= start.distance(end) + thickness + ABS_ERR)
				{
					vector<tuple<double, double, string>>::iterator it;
					for (it = out.begin(); it != out.end(); it++)
					{
						if (get<0>(*it) > t1)
							break;
					}
					out.insert(it, make_tuple(t1, t2, wl2.get_id()));
				}
			}
		}
		return out;
	}

	bool GraphConstructor::CheckConnect(set<size_t> &pointnums) const
	{
		return g.check_connected(pointnums);
	}

	int GraphConstructor::DoorProcess()
	{
		for (size_t k = 0; k < doors_.size(); k++)
		{
			for (const Wall &wl : walls_)
			{
				if (wl.get_id() == doors_[k].get_host())
				{
					doors_[k].set_thickness(wl.get_thickness());
				}
			}
		}


		for(size_t wdi = 0; wdi<doors_.size(); )
		{
			Door& wd = doors_[wdi];
			bool found_wall=false;
			for (const Wall &wl : walls_)
			{
				if (wl.get_id() != wd.get_host())
				{
					continue;
				}
				found_wall = true;
				wd.set_zup(MIN(floor_height, wd.get_zup() + offset_door));
				wd.set_zlow(MAX(0, wd.get_zlow() - offset_door));

				Point lmost = wl.get_start();
				Point rmost = wl.get_end();
				vector<tuple<double, double, string>> ts = GetCrossPoints(wl, lmost, rmost);
				double wd_relpos = wd.get_start() * wl.get_u() - lmost * wl.get_u();
				pair<double, double> the_intv({0,wl.get_length()});
				bool intv_init = ts.empty();
				if (ts.size()>0 && get<0>(ts[0]) > ABS_ERR)
				{
					if (-ABS_ERR <= wd_relpos && wd_relpos <= get<0>(ts[0]))
					{
						intv_init = true;
						the_intv.first = 0;
						the_intv.second = get<0>(ts[0]);
					}
				}
				for (size_t k = 0; !intv_init && k + 1 < ts.size() ; k++)
				{
					if (get<1>(ts[k]) - ABS_ERR <= wd_relpos && wd_relpos <= get<0>(ts[k + 1]) + ABS_ERR)
					{
						intv_init = true;
						the_intv.first = get<1>(ts[k]);
						the_intv.second = get<0>(ts[k + 1]);
					}
				}
				if (ts.size()>0 && !intv_init && get<1>(*ts.rbegin()) < wl.get_length() - ABS_ERR)
				{
					if (get<1>(*ts.rbegin()) <= wd_relpos && wd_relpos <= wl.get_length() + ABS_ERR)
					{
						intv_init = true;
						the_intv.first = get<1>(*ts.rbegin());
						the_intv.second = wl.get_length();
					}
				}
				if (!intv_init)
				{
					cout<< "The position of one  door is ILLEGAL!\n";
					return Error::INPUT_DATA_CONFLICT_ERROR;
				}

				double l_lim = ((lmost + wl.get_u() * the_intv.first) - wd.get_start()) * wd.get_u();
				double r_lim = ((lmost + wl.get_u() * the_intv.second) - wd.get_start()) * wd.get_u();
				if (r_lim < l_lim)
				{
					double tmp = l_lim;
					l_lim = r_lim;
					r_lim = tmp;
				}
				wd.set_ul(MAX(l_lim, -offset_door));
				wd.set_ur(MIN(r_lim, wd.get_length() + offset_door));
			}

			if(found_wall)
				wdi++;
			else 
			{
				cout<<"a(n) "<<wd.get_name()<<" can't find its attaching wall."<<endl;
				doors_.erase(doors_.begin() + wdi);
			}
		}

		return Error::COMPUTE_NO_ERROR;
	}

	void GraphConstructor::WallsPreprocess()
	{
		vector<double>  walls_forward_extension_, walls_backward_extension_;
		for (const auto &wl : walls_)
		{
			double for_ex = 0.0, back_ex = 0.0;
			Point start(wl.get_start()), end(wl.get_end());
			Point n(wl.get_n()), vert(wl.get_vert_direc());
			double thick = wl.get_thickness(), height = wl.get_height();
			pair<Point, Point> ground_side_wpos = make_pair(start + n * (thick / 2), end + n * (thick / 2));
			pair<Point, Point> ground_side_wneg = make_pair(start - n * (thick / 2), end - n * (thick / 2));
			pair<Point, Point> ceiling_side_wpos = make_pair(start + n * (thick / 2) + vert * height, end + n * (thick / 2) + vert * height);
			pair<Point, Point> ceiling_side_wneg = make_pair(start - n * (thick / 2) + vert * height, end - n * (thick / 2) + vert * height);
			for (const auto &wl2 : walls_)
			{
				if (wl.get_id() == wl2.get_id())
					continue;
				
				auto rel1 = wl2.HousingIntersectLineSegment(ground_side_wpos.first,ground_side_wpos.second,floor_height,0.0,wl2.get_thickness()/2);
				auto rel2 = wl2.HousingIntersectLineSegment(ceiling_side_wpos.first, ceiling_side_wpos.second,floor_height,0.0,wl2.get_thickness()/2);
				if((rel1.first!=LineCuboidRelation::DISJOINT)&&(rel2.first!=LineCuboidRelation::DISJOINT)&&(MAX(rel1.second.first, rel1.second.first) < -REL_ERR))
					back_ex = MAX(back_ex, -MAX(rel1.second.first, rel2.second.first));
				if((rel1.first!=LineCuboidRelation::DISJOINT)&&(rel2.first!=LineCuboidRelation::DISJOINT)&&(MIN(rel1.second.second, rel2.second.second) > wl.get_length() + REL_ERR))
					for_ex = MAX(for_ex, MIN(rel1.second.second, rel2.second.second) - wl.get_length());

				rel1 = wl2.HousingIntersectLineSegment(ground_side_wneg.first, ground_side_wneg.second,floor_height,0.0,wl2.get_thickness()/2);
				rel2 = wl2.HousingIntersectLineSegment(ceiling_side_wneg.first, ceiling_side_wneg.second,floor_height,0.0,wl2.get_thickness()/2);
				if((rel1.first!=LineCuboidRelation::DISJOINT)&&(rel2.first!=LineCuboidRelation::DISJOINT)&&(MAX(rel1.second.first, rel1.second.first) < -REL_ERR))
					back_ex = MAX(back_ex, -MAX(rel1.second.first, rel2.second.first));
				if((rel1.first!=LineCuboidRelation::DISJOINT)&&(rel2.first!=LineCuboidRelation::DISJOINT)&&(MIN(rel1.second.second, rel2.second.second) > wl.get_length() + REL_ERR))
					for_ex = MAX(for_ex, MIN(rel1.second.second, rel2.second.second) - wl.get_length());
			}
			walls_forward_extension_.push_back(for_ex);
			walls_backward_extension_.push_back(back_ex);
		}
		for(size_t l=0;l<walls_.size();l++)
		{
			Point u = walls_[l].get_u().normalized();
			auto& wl = walls_[l];
			walls_[l] = Wall(wl.get_name(),
							 wl.get_id(),
							 wl.get_start() - walls_backward_extension_[l] * u,
							 wl.get_end() + walls_forward_extension_[l] * u,
							 wl.get_height(),
							 wl.get_thickness(),
							 wl.get_type());
		}
	}

	void GraphConstructor::read_config(const Config& conf)
	{
		floor_height = conf.floor_height;
		offset_door = conf.offset_door;

		connect_threshold = conf.connect_threshold;
		mini_radius = conf.mini_radius;
		live_wire_unit_cost = conf.live_wire_unit_cost;
		neutral_wire_unit_cost = conf.neutral_wire_unit_cost;
		earth_wire_unit_cost = conf.earth_wire_unit_cost;
		through_wall_conduit_unit_cost = conf.through_wall_conduit_unit_cost;
		in_groove_conduit_unit_cost = conf.in_groove_conduit_unit_cost;
		conduit_unit_cost = conf.conduit_unit_cost;
	}
	void GraphConstructor::set_mini_radius(double r) 					{ mini_radius = r; }
	void GraphConstructor::set_conduit_unit_cost(double c)				{ conduit_unit_cost = c;}
	void GraphConstructor::set_in_groove_conduit_unit_cost(double c)	{ in_groove_conduit_unit_cost = c;}
	void GraphConstructor::set_through_wall_conduit_unit_cost(double c)	{ through_wall_conduit_unit_cost = c;}
	void GraphConstructor::set_live_wire_unit_cost(double c) 			{ live_wire_unit_cost = c; }
	void GraphConstructor::set_neutral_wire_unit_cost(double c) 		{ neutral_wire_unit_cost = c; }
	void GraphConstructor::set_earth_wire_unit_cost(double c) 			{ earth_wire_unit_cost = c; }
	void GraphConstructor::set_connect_threshold(double t) 				{ connect_threshold = t; }

	void GraphConstructor::set_PSB(const Device& dev)
	{
		PSB = dev;
	}

	void GraphConstructor::add_device(const Device& dev)
	{
		devices.push_back(dev);
	}

	void GraphConstructor::calc_costs()
	{
		for (size_t k = 0; k < num_edge(); k++)
		{
			Edge e = edge(k);
			bool _ = false;
			Point& v1 = vertex(e.first);
			Point& v2 = vertex(e.second);
			
			// double dom = live_wire_unit_cost + neutral_wire_unit_cost+earth_wire_unit_cost+through_wall_conduit_unit_cost+in_groove_conduit_unit_cost+conduit_unit_cost;
			double dom = 1.0;
			bool onceil = (v1.z > floor_height - ABS_ERR) && (v2.z > floor_height - ABS_ERR);
			bool onfloor= (v1.z < ABS_ERR) && (v2.z < ABS_ERR);
			auto analysis = intersection_analysis(v1,v2);
			double totallen = 0.0;
			for(auto& p: analysis) totallen+=p.second;

			double clive = totallen * live_wire_unit_cost/dom;
			double cneutral = totallen * neutral_wire_unit_cost/dom;
			double cearth = totallen * earth_wire_unit_cost/dom;
			double cConduit = 0.0;
			cConduit += through_wall_conduit_unit_cost/dom * analysis[LineCuboidRelation::INTERSECTING];
			cConduit += in_groove_conduit_unit_cost/dom * analysis[LineCuboidRelation::COINCIDENT];
			cConduit += conduit_unit_cost/dom * analysis[LineCuboidRelation::DISJOINT];

			g.set_edge_weight(k, clive+cneutral+cearth+cConduit);
		}
	}

	void GraphConstructor::finalDeletingCheck()
	{
		for (size_t k = 0; k < g.num_edge();)
		{
			if (LnThroughNotPass(vertex(edge(k).first), vertex(edge(k).second),0.0,false)){
				g.remove_edge(k);
			}
			else
				k++;
		}
	}

	void GraphConstructor::construct()
	{
		int err;
		WallsPreprocess();
		err = DoorProcess();

		vector<double> xs,ys;

		collect_wall_grid(xs, ys);
		collect_door_grid(xs, ys);
		collect_device_grid(xs, ys);

		Hanan(xs,ys,{0.0});

		calc_costs();
	}

	void GraphConstructor::collect_wall_grid(vector<double>& xs, vector<double>& ys)
	{
		for(auto& wl: walls_)
		{
			Point start = wl.get_start(), 
				  end=wl.get_end(),
				  normal = wl.get_n();
			double thick = wl.get_thickness();

			if(Xpos.IsParallel(normal) || Ypos.IsParallel(normal))
			{
				Point p1 = start - normal * thick / 2.0, p2 = end + normal * thick / 2.0;

				OrderedInsert(xs, p1.x, ABS_ERR);
				OrderedInsert(ys, p1.y, ABS_ERR);
				OrderedInsert(xs, p2.x, ABS_ERR);
				OrderedInsert(ys, p2.y, ABS_ERR);
			}
		}
	}

	void GraphConstructor::collect_door_grid(vector<double>& xs, vector<double>& ys)
	{
		for(auto& d: doors_)
		{
			Point start = d.get_start(),
				  end = d.get_end(),
				  normal = d.get_n(),
				  u=d.get_u();

			Point p0 = start, p1 = (start+end)/2, p2 = end;
			if(Xpos.IsParallel(normal))
			{
				OrderedInsert(ys, p0.y, ABS_ERR);
				OrderedInsert(ys, p1.y, ABS_ERR);
				OrderedInsert(ys, p2.y, ABS_ERR);
			}
			else if(Ypos.IsParallel(normal))
			{
				OrderedInsert(xs, p0.x, ABS_ERR);
				OrderedInsert(xs, p1.x, ABS_ERR);
				OrderedInsert(xs, p2.x, ABS_ERR);
			}
		}
	}

	void GraphConstructor::collect_device_grid(vector<double>& xs, vector<double>& ys)
	{
		OrderedInsert(xs, PSB.location.x, ABS_ERR);
		OrderedInsert(ys, PSB.location.y, ABS_ERR);

		for(auto& dev: devices)
		{
			OrderedInsert(xs, dev.location.x, ABS_ERR);
			OrderedInsert(ys, dev.location.y, ABS_ERR);
		}
	}

	void GraphConstructor::Hanan(const vector<double>& xs, const vector<double>& ys, const vector<double>& zs)
	{
		int nx = xs.size(), ny = ys.size(), nz = zs.size();
		
		for(int k=0;k<nz;k++)
		{
			for(int j=0;j<ny;j++)
			{
				for(int i=0;i<nx;i++)
				{ 
					
					size_t pnt = g.add_vertex_simply(Point(xs[i], ys[j], zs[k]));
					
					if(i>0)
					{
						add_edge(pnt, (i-1)+nx*j+nx*ny*k);	
					}
					if(j>0)
					{
						add_edge(pnt, i+nx*(j-1)+nx*ny*k);
					}
					if(k>0)
					{
						add_edge(pnt, i+nx*j+nx*ny*(k-1));
					}
				}
			}
		}

		if(fabs(PSB.location.z)>ABS_ERR)
		{
			PSB_index = g.add_vertex_simply(PSB.location);
			add_edge(PSB_index, g.find_vertex(Point(PSB.location.x, PSB.location.y, 0.0)));
		}
		else
			PSB_index = g.find_vertex(PSB.location);
		
		for(auto& dev : devices)
		{
			if(fabs(dev.location.z)>ABS_ERR)
			{
				devices_indices.push_back(g.add_vertex_simply(dev.location));
				add_edge(devices_indices.back(), g.find_vertex(Point(dev.location.x, dev.location.y, 0.0)));
			}
			else 
				devices_indices.push_back(g.find_vertex(dev.location));
			if (dev.name == "Junction Box")
			{
				JB_index = devices_indices.back();
			}
		}

		for(int i=0;i<devices.size();i++)
		{
			for(int j=i+1;j<devices.size();j++)
			{
				if(devices[i].location.distance(devices[j].location) <= connect_threshold)
					add_edge(devices_indices[i], devices_indices[j],true);
			}
		}
	}

	bool GraphConstructor::valid_point(const Point& p) const
	{
		bool out = true;
		for(auto& wl: walls_)
		{
			out &= (! wl.IsContainPoint(p,floor_height));
		}
		if(!out)
		{
			for(auto& d: doors_)
			{
				out |= d.IsContainPoint(p,floor_height);
			}
		}
		return out;
	}
}