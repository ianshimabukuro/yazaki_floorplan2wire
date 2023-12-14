#include "base/plane.h"
#include <cmath>
#include <limits>
#define MAX(A,B) ((B)>(A)?(B):(A))
#define MIN(A,B) ((B)<(A)?(B):(A))
namespace ewd
{
    Plane::Plane(const Point& base, const Point& u, const Point& v, bool bounded)
        : base_(base), u_(u), v_(v), bounded_(bounded)  
    { 
        normal_ = u_.cross(v_).normalized(); 
    }

    Plane::Plane(const Point& base, const Point& normal)
        : base_(base), normal_(normal.normalized())
    {
        u_ = normal_.cross(Point(0, 0, 1));
        if(u_.norm() < 1e-6)
            u_ = normal_.cross(Point(1, 0, 0));
        u_ = u_.normalized();
        v_ = normal_.cross(u_);
        bounded_ = false;
    }

    Plane::Plane(const Plane& pl)
        : base_(pl.base_), u_(pl.u_), v_(pl.v_), normal_(pl.normal_), bounded_(pl.bounded_){}

    Plane::~Plane() {}

    Point Plane::get_base() const { return base_; }
    Point Plane::get_u() const { return u_; }
    Point Plane::get_v() const { return v_; }
    Point Plane::get_normal() const { return normal_; }
    bool Plane::is_bounded() const { return bounded_; }
    
    Plane Plane::to_unbounded_plane() const
    {
        if(!is_bounded())
            return *this;
        return Plane(base_, u_, v_, false);
    }

    Point Plane::get_projection(const Point& p) const
    {
        return p - normal_ * (normal_*(p - base_));
    }

    double Plane::get_distance(const Point& p) const
    {
        return fabs(normal_ * (p - base_));
    }

    double Plane::get_signed_distance(const Point& p) const
    {
        return normal_ * (p - base_);
    }

    LinePlaneRelation Plane::IntersectLine(const Point& p, const Point& d, double& t1, double& t2, LineType type,double ABS_ERR, double REL_ERR) const
    {
        t1 = 0; t2 = 1;
        if(fabs(d*normal_) < ABS_ERR && get_distance(p) > ABS_ERR)
            return LinePlaneRelation::DISJOINT;
        double uscale = u_.norm(), vscale=v_.norm();
        if(fabs(d*normal_) < ABS_ERR)
        {
            if(!bounded_)
            {
                return LinePlaneRelation::COINCIDENT;
            }
            double usq= u_*u_, vsq= v_*v_, uv= u_*v_;
            double sb = (p-base_)*u_/usq, tb = (p-base_)*v_/vsq; // p = sb*u + tb*v
            double sd = d*u_/usq, td = d*v_/vsq;        // d = sd*u + td*v

            double u_min, u_max, v_min, v_max;
            bool u_flag = fabs(sd) > REL_ERR, v_flag = fabs(td) > REL_ERR;
            if(u_flag)
            {
                u_min = (-sb)/sd;
                u_max = (1-sb)/sd;
                if(u_min > u_max)
                    std::swap(u_min, u_max);
                if(type == LineType::LINE)
                {
                    t1 = u_min; t2 = u_max;   
                }
                else if(type == LineType::RAY)
                {
                    t2 = u_max;
                }
                t1 = MAX(t1, u_min);
                t2 = MIN(t2, u_max);
            }
            else if(sb < -ABS_ERR/uscale || sb > 1+ABS_ERR/uscale)
                return LinePlaneRelation::DISJOINT;
            if(v_flag)
            {
                v_min = (-tb)/td;
                v_max = (1-tb)/td;
                if(v_min > v_max)
                    std::swap(v_min, v_max);
                if(type == LineType::LINE && !u_flag)
                {
                    t1 = v_min; t2 = v_max;
                }
                else if (type == LineType::RAY && !u_flag)
                {
                    t2 = v_max;
                }
                t1 = MAX(t1, v_min);
                t2 = MIN(t2, v_max);
            }
            else if (tb < -ABS_ERR/vscale || tb > 1+ABS_ERR/vscale)
                return LinePlaneRelation::DISJOINT;
            // if(type == LineType::LINE)
            // {
            //     return LinePlaneRelation::COINCIDENT;
            // }
            
            if(t1 > t2 - ABS_ERR/d.norm())
                return LinePlaneRelation::DISJOINT;
            else 
                return LinePlaneRelation::COINCIDENT;
        }
        t1 = (normal_*(base_ - p)) / (d*normal_);
        t2 = t1;
        if(((t1 < -ABS_ERR/d.norm()) && (type != LineType::LINE)) || ((t1 > 1+ABS_ERR/d.norm()) && (type == LineType::SEGMENT)))
            return LinePlaneRelation::DISJOINT;
        if(!bounded_)
            return LinePlaneRelation::INTERSECTING;
        Point proj = p + d * t1;
        if(ContainPoint(proj))
            return LinePlaneRelation::INTERSECTING;
        return LinePlaneRelation::DISJOINT;
    }

    bool Plane::ContainPoint(const Point& p, double ABS_ERR) const
    {
        if (get_distance(p) > ABS_ERR)
            return false;
        if(!bounded_) return true;
        double l1 = u_ * (p - base_)/(u_.norm()*u_.norm()), l2 = v_ * (p - base_)/(v_.norm()*v_.norm());
        double uscale = u_.norm(), vscale = v_.norm();
        if((l1 < -ABS_ERR/uscale) || (l1 > 1 + ABS_ERR/uscale) || (l2 < -ABS_ERR/vscale) || (l2 > 1 + ABS_ERR/vscale))
            return false;
        return true;
    }

    PlaneRelation Plane::IntersectPlane(const Plane& pl, Point& start, Point& direc,double ABS_ERR, double REL_ERR) const
    {

        // 重合或平行
        if(fabs(pl.normal_*u_/u_.norm()) < ABS_ERR && pl.normal_*v_/v_.norm() < ABS_ERR)
        {
            if(get_distance(pl.base_) > ABS_ERR)
                return PlaneRelation::DISJOINT;
            else 
                return PlaneRelation::COINCIDENT;
        }
        // 相交情况
        Point tmp_direc = pl.normal_.cross(normal_).normalized();
        double t1,t2;
        Point to = pl.normal_.cross(tmp_direc);
        auto st = IntersectLine(pl.base_, to, t1,t2, LineType::LINE,ABS_ERR,REL_ERR);
        start = pl.base_ + to * t1;

        double thist1,thist2,plt1,plt2;
        auto thisst = IntersectLine(start, tmp_direc, thist1,thist2, LineType::LINE,ABS_ERR,REL_ERR);
        auto plst = pl.IntersectLine(start, tmp_direc, plt1,plt2, LineType::LINE,ABS_ERR,REL_ERR);
        LineType type1,type2;
        if (is_bounded())
            type1 = LineType::SEGMENT;
        else
            type1 = LineType::LINE;
        if (pl.is_bounded())
            type2 = LineType::SEGMENT;
        else
            type2 = LineType::LINE;
        
        if(thisst == LinePlaneRelation::DISJOINT || plst == LinePlaneRelation::DISJOINT)
            return PlaneRelation::DISJOINT;
        if(type1 == LineType::LINE && type2 == LineType::LINE)
        {
            direc = tmp_direc;
        }
        else 
        {
            if(type1 == LineType::LINE)
            {
                t1 = plt1, t2 = plt2;
            }
            else if (type2 == LineType::LINE)
            {
                t1 = thist1, t2 = thist2;
            }
            else
            {
                t1 = MAX(thist1, plt1), t2 = MIN(thist2, plt2);
            }
            if(t1 > t2 - ABS_ERR)
                return PlaneRelation::DISJOINT;
            direc = tmp_direc * (t2 - t1);
            start = start + tmp_direc * t1;
        }
        return PlaneRelation::INTERSECTING;
    }

    std::vector<Plane> get_Hanan_planes(const Point& p, bool z)
    {
        std::vector<Plane> planes;
        planes.push_back(Plane(p, Point(1,0,0), Point(0,0,1)));
        planes.push_back(Plane(p, Point(0,1,0), Point(0,0,1)));
        if(z)
            planes.push_back(Plane(p, Point(1,0,0), Point(0,1,0)));
        return planes;
    }
} // namespace ewd
