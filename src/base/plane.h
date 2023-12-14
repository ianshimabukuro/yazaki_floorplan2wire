#pragma once
#include "base/point.h"
#include <vector>
namespace ewd
{
    enum class LinePlaneRelation
    {
        COINCIDENT,
        INTERSECTING,
        DISJOINT
    };
    enum class PlaneRelation
    {
        COINCIDENT,
        INTERSECTING,
        DISJOINT
    };

    /**
     * @brief 平面或矩形，暂时不能表示平行四边行
     * 
     */
    class Plane
    {
    private:
        Point base_, u_, v_;
        Point normal_;
        bool bounded_;

    public:
        Plane(const Point& base, const Point& u, const Point& v, bool bounded = false);
        Plane(const Point& base, const Point& normal);
        Plane(const Plane& pl);
        ~Plane();
        Point get_base() const;
        Point get_u() const;
        Point get_v() const;
        Point get_normal() const;
        bool is_bounded() const;
        Plane to_unbounded_plane() const;

        bool ContainPoint(const Point& p, double ABS_ERR = 1e-6) const;
        Point get_projection(const Point& p) const;
        double get_distance(const Point& p) const;
        double get_signed_distance(const Point& p) const;
        LinePlaneRelation IntersectLine(const Point& p, const Point& d, double& t1,double& t2, LineType type = LineType::SEGMENT,double ABS_ERR=1e-6, double REL_ERR=1e-4) const;
        PlaneRelation IntersectPlane(const Plane& p, Point& start, Point& direc,double ABS_ERR=1e-6, double REL_ERR=1e-4) const; //TODO: implement
    };

    std::vector<Plane> get_Hanan_planes(const Point& p, bool z = false);
}

