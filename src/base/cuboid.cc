#include "base/cuboid.h"
#include <cmath>

namespace ewd
{
    Cuboid::Cuboid(const Point& base, const Point& l, const Point& w, const Point& h)
        : base_(base), l_(l), w_(w), h_(h), length_(l.norm()), width_(w.norm()), height_(h.norm())
    {
        surfaces_[TOP]    = std::shared_ptr<Plane>(new Plane(base + h, l, w, true));
        surfaces_[BOTTOM] = std::shared_ptr<Plane>(new Plane(base, l, w, true));
        surfaces_[LEFT]   = std::shared_ptr<Plane>(new Plane(base, w, h, true));
        surfaces_[RIGHT]  = std::shared_ptr<Plane>(new Plane(base + l, w, h, true));
        surfaces_[FRONT]  = std::shared_ptr<Plane>(new Plane(base + w, l, h, true));
        surfaces_[BACK]   = std::shared_ptr<Plane>(new Plane(base, l, h, true));
        if(l.norm() < 1e-6 || w.norm() < 1e-6 || h.norm() < 1e-6)
            throw std::invalid_argument("Invalid length, width or height.");
    }

    Cuboid::Cuboid(const Cuboid& c)
        : base_(c.base_), l_(c.l_), w_(c.w_), h_(c.h_), length_(c.length_), width_(c.width_), height_(c.height_)
    {
        for(int i=0;i<6;i++)
            surfaces_[i] = std::shared_ptr<Plane>(new Plane(*c.surfaces_[i]));
    }

    Cuboid::~Cuboid() {}

    Point Cuboid::get_base() const { return base_; }
    Point Cuboid::get_l() const { return l_; }
    Point Cuboid::get_w() const { return w_; }
    Point Cuboid::get_h() const { return h_; }
    Point Cuboid::get_l_direc() const { return l_.normalized(); }
    Point Cuboid::get_w_direc() const { return w_.normalized(); }
    Point Cuboid::get_h_direc() const { return h_.normalized(); }
    double Cuboid::get_length() const { return length_; }
    double Cuboid::get_width() const { return width_; }
    double Cuboid::get_height() const { return height_; }
    Plane Cuboid::get_surface(int s) const { return *surfaces_[s]; }
    
    Point Cuboid::change_coordinate(const Point& p) const
    {
        Point r  = p - base_;
        return Point(r * l_/(length_*length_), r * w_/(width_*width_), r * h_/(height_*height_));
    }

    bool Cuboid::contain_point(const Point& p, double ABS_ERR) const
    {
        Point r = change_coordinate(p);
        return ((r.x >= 0 - ABS_ERR / length_) &&
                (r.x <= 1 + ABS_ERR / length_) &&
                (r.y >= 0 - ABS_ERR / width_) &&
                (r.y <= 1 + ABS_ERR / width_) &&
                (r.z >= 0 - ABS_ERR / height_) &&
                (r.z <= 1 + ABS_ERR / height_));
    }

    LineCuboidRelation Cuboid::IntersectLineSegment(
        const Point &p, const Point &d,
        double &t1, double &t2,
        std::vector<bool> &surface_coincidence, double ABS_ERR, double REL_ERR) const
    {
        if(surface_coincidence.size() < 6)
            surface_coincidence.resize(6, false);
        Point newp1 = change_coordinate(p), newp2 = change_coordinate(p+d);
        Point e = newp2 - newp1;
        double scale = (l_+w_+h_).norm();
        bool l_flag = fabs(e.x) > REL_ERR, w_flag = fabs(e.y) > REL_ERR, h_flag = fabs(e.z) > REL_ERR;
        t1 = 0, t2 = 1;
        double l_min, l_max, w_min, w_max, h_min, h_max;

        if(l_flag)
        {
            l_min =  - newp1.x/e.x;
            l_max = (1 - newp1.x)/e.x;
            if (l_min > l_max)
                std::swap(l_min, l_max);
        }
        else if (newp1.x < -ABS_ERR/length_ || newp1.x > 1+ABS_ERR/length_)
            return LineCuboidRelation::DISJOINT;
        
        if(w_flag)
        {
            w_min =  - newp1.y/e.y;
            w_max = (1 - newp1.y)/e.y;
            if (w_min > w_max)
                std::swap(w_min, w_max);
        }
        else if (newp1.y < -ABS_ERR/width_ || newp1.y > 1+ABS_ERR/width_)
            return LineCuboidRelation::DISJOINT;

        if(h_flag)
        {
            h_min =  - newp1.z/e.z;
            h_max = (1 - newp1.z)/e.z;
            if (h_min > h_max)
                std::swap(h_min, h_max);
        }
        else if (newp1.z < -ABS_ERR/height_ || newp1.z > 1+ABS_ERR/height_)
            return LineCuboidRelation::DISJOINT;
        
        if (l_flag)
        {
            t1 = l_min > t1 ? l_min : t1;
            t2 = l_max < t2 ? l_max : t2;
        }
        if (w_flag)
        {
            t1 = w_min > t1 ? w_min : t1;
            t2 = w_max < t2 ? w_max : t2;
        }
        if (h_flag)
        {
            t1 = h_min > t1 ? h_min : t1;
            t2 = h_max < t2 ? h_max : t2;
        }

        if (t1 > t2-ABS_ERR/scale)
            return LineCuboidRelation::DISJOINT; 
        bool any_coincident = false;
        double temp,temp2;
        for(int s=0;s<6;s++)
        {
            auto st1 = surfaces_[s]->IntersectLine(p,d,temp,temp2, LineType::SEGMENT,ABS_ERR,REL_ERR);
            if(st1 == LinePlaneRelation::COINCIDENT)
            {
                surface_coincidence[s] = true;
                any_coincident = true;
            }
        }
        if(any_coincident)
            return LineCuboidRelation::COINCIDENT;
        return LineCuboidRelation::INTERSECTING;
    }

    LineCuboidRelation Cuboid::IntersectLineSegment(
        const Point &p, const Point &d,
        double &t1, double &t2, double ABS_ERR, double REL_ERR) const
    {
        std::vector<bool> surface_coincidence;
        return IntersectLineSegment(p,d,t1,t2,surface_coincidence, ABS_ERR, REL_ERR);
    }

} // namespace ewd
