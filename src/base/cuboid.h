#pragma once
#include "base/plane.h"
#include <tuple>
#include <memory>
namespace ewd
{
    enum class LineCuboidRelation
    {
        DISJOINT,
        INTERSECTING,
        COINCIDENT
    };
    enum CuboidSurface 
    {
        TOP=0,  // h+
        RIGHT, // l+
        FRONT, // w+
        BOTTOM, // h-
        LEFT,  // l-
        BACK   // w-
    };
    class Cuboid
    {
    protected:
        Point base_, l_, w_, h_;
        double length_, width_, height_;
        std::shared_ptr<Plane> surfaces_[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    public:
        Cuboid(const Point& base, const Point& l, const Point& w, const Point& h);
        Cuboid(const Cuboid& c);
        ~Cuboid();
        Point get_base() const;
        Point get_l() const;
        Point get_w() const;
        Point get_h() const;
        Point get_l_direc() const;
        Point get_w_direc() const;
        Point get_h_direc() const;
        double get_length() const;
        double get_width() const;
        double get_height() const;
        Plane get_surface(int s) const;

        Point change_coordinate(const Point& p) const;
        bool contain_point(const Point& p, double ABS_ERR = 1e-6) const;
        // Point get_Hdirec_projection(const Point &p) const;
        // Point get_Wdirec_projection(const Point &p) const;
        // Point get_Ldirec_projection(const Point &p) const;
        LineCuboidRelation IntersectLineSegment(const Point &p, const Point &direc, double &t1, double &t2, std::vector<bool>& surface_coincidence, double ABS_ERR=1e-6, double REL_ERR=1e-4) const;
        LineCuboidRelation IntersectLineSegment(const Point &p, const Point &direc, double &t1, double &t2, double ABS_ERR=1e-6, double REL_ERR=1e-4) const;
    };
}