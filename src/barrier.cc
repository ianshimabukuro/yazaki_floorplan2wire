#include "barrier.h"
#include <cmath>
#include <algorithm>
template <typename T1, typename T2>
constexpr auto MIN(T1 X, T2 Y) { return (Y) < (X) ? (Y) : (X); }
template <typename T1, typename T2>
constexpr auto MAX(T1 X, T2 Y) { return (Y) > (X) ? (Y) : (X); }

using namespace std;
namespace ewd
{

    using ewd::CuboidSurface;

    HouseBarrier::HouseBarrier(const string &sname,
                               const string &sid,
                               const Point &ps,
                               const Point &pe,
                               double h,
                               double thick,
                               BarrierType _type) : name_(sname), id_(sid), start_(ps), end_(pe), height_(h), thickness_(thick), type_(_type)
    {
        length_ = start_.distance(end_);
        u_ = (end_ - start_).normalized();
        n_ = vert_direc_.cross(u_);
        establish8vertices();
        this->update_cu();
    }

    HouseBarrier::HouseBarrier(const HouseBarrier &br) : name_(br.name_), id_(br.id_),
                                                         start_(br.start_), end_(br.end_),
                                                         length_(br.length_), height_(br.height_), thickness_(br.thickness_),
                                                         type_(br.type_), u_(br.u_), n_(br.n_), vert_direc_(br.vert_direc_)
    {
        for (const Point &p : br.eight_vertex_)
        {
            eight_vertex_.push_back(p);
        }
    }

    HouseBarrier::~HouseBarrier() {}

    void HouseBarrier::update_cu()
    {
        Point tmpbase = start_ - n_ * (thickness_ / 2);
        Point l = end_ - start_;
        Point w = n_ * thickness_;
        Point h = vert_direc_ * height_;
        if ((l.norm() > 1e-2) && (w.norm() > 1e-2) && (h.norm() > 1e-2))
        {
            cu_.reset(new Cuboid(tmpbase, l, w, h));
            this->update_offset_cu();
        }
        else
        {
            cu_.reset();
            offset_cu_.reset();
        }
    }

    void HouseBarrier::update_offset_cu()
    {
        Point tmpbase = start_ - n_ * (thickness_ / 2) - offset_ * u_ - offset_ * n_;
        Point l = end_ - start_ + u_ * (2 * offset_);
        Point w = n_ * (thickness_ + 2 * offset_);
        Point h = vert_direc_ * height_;
        if ((l.norm() > 1e-2) && (w.norm() > 1e-2) && (h.norm() > 1e-2))
            offset_cu_.reset(new Cuboid(tmpbase, l, w, h));
        else
            offset_cu_.reset();
    }

    Point HouseBarrier::ChangeCoordinate(const Point &pnt) const
    {
        Point start2pnt = pnt - start_;
        return Point(u_ * start2pnt, n_ * start2pnt, vert_direc_ * start2pnt);
    }

    void HouseBarrier::establish8vertices()
    {
        eight_vertex_.clear();
        eight_vertex_.push_back(start_ + n_ * (thickness_ / 2));
        eight_vertex_.push_back(end_ + n_ * (thickness_ / 2));
        eight_vertex_.push_back(end_ - n_ * (thickness_ / 2));
        eight_vertex_.push_back(start_ - n_ * (thickness_ / 2));
        eight_vertex_.push_back(start_ + n_ * (thickness_ / 2) + vert_direc_ * height_);
        eight_vertex_.push_back(end_ + n_ * (thickness_ / 2) + vert_direc_ * height_);
        eight_vertex_.push_back(end_ - n_ * (thickness_ / 2) + vert_direc_ * height_);
        eight_vertex_.push_back(start_ - n_ * (thickness_ / 2) + vert_direc_ * height_);
    }

    void HouseBarrier::set_vert_direc(const Point &p)
    {
        vert_direc_ = p.normalized();
        n_ = vert_direc_.cross(u_);
        establish8vertices();
        this->update_cu();
    }

    void HouseBarrier::set_offset(double off)
    {
        offset_ = off;
        this->update_offset_cu();
    }
    string HouseBarrier::get_name() const { return name_; }
    string HouseBarrier::get_id() const { return id_; }
    Point HouseBarrier::get_start() const { return start_; }
    Point HouseBarrier::get_end() const { return end_; }
    double HouseBarrier::get_length() const { return length_; }
    double HouseBarrier::get_height() const { return height_; }
    double HouseBarrier::get_thickness() const { return thickness_; }
    BarrierType HouseBarrier::get_type() const { return type_; }
    Point HouseBarrier::get_u() const { return u_; }
    Point HouseBarrier::get_n() const { return n_; }
    Point HouseBarrier::get_vert_direc() const { return vert_direc_; }
    bool HouseBarrier::allow_through() const { return type_ == BarrierType::WALL || type_ == BarrierType::BEAM; }

    Point HouseBarrier::Project(const Point &pnt, double depth) const
    {
        Point a = pnt - (start_ + n_ * (thickness_ / 2 - depth));
        Point b = pnt - (start_ - n_ * (thickness_ / 2 - depth));
        Point &p = a.norm() <= b.norm() ? a : b;
        return pnt - n_ * (n_ * p);
    }

    vector<Point> HouseBarrier::GetCorners(double offset, double descen) const
    {
        vector<Point> plist;
        plist.push_back(start_ + n_ * (thickness_ / 2 + offset) - u_ * offset);
        plist.push_back(end_ + n_ * (thickness_ / 2 + offset) + u_ * offset);
        plist.push_back(end_ - n_ * (thickness_ / 2 + offset) + u_ * offset);
        plist.push_back(start_ - n_ * (thickness_ / 2 + offset) - u_ * offset);
        plist.push_back(start_ + n_ * (thickness_ / 2 + offset) + vert_direc_ * (height_ - descen) - u_ * offset);
        plist.push_back(end_ + n_ * (thickness_ / 2 + offset) + vert_direc_ * (height_ - descen) + u_ * offset);
        plist.push_back(end_ - n_ * (thickness_ / 2 + offset) + vert_direc_ * (height_ - descen) + u_ * offset);
        plist.push_back(start_ - n_ * (thickness_ / 2 + offset) + vert_direc_ * (height_ - descen) - u_ * offset);
        return plist;
    }

    vector<Point> HouseBarrier::GetCorners(double offset, double descen, double floorheight) const
    {
        vector<Point> plist;
        double lower = start_.z;
        double upper = start_.z + height_;
        lower = lower < 0 ? lower : 0;
        upper = upper <= floorheight ? upper : floorheight;
        upper = upper - start_.z;
        plist.push_back(start_ + n_ * (thickness_ / 2 + offset) - vert_direc_ * lower - u_ * offset);
        plist.push_back(end_ + n_ * (thickness_ / 2 + offset) - vert_direc_ * lower + u_ * offset);
        plist.push_back(end_ - n_ * (thickness_ / 2 + offset) - vert_direc_ * lower + u_ * offset);
        plist.push_back(start_ - n_ * (thickness_ / 2 + offset) - vert_direc_ * lower - u_ * offset);
        plist.push_back(start_ + n_ * (thickness_ / 2 + offset) + vert_direc_ * (upper - descen) - u_ * offset);
        plist.push_back(end_ + n_ * (thickness_ / 2 + offset) + vert_direc_ * (upper - descen) + u_ * offset);
        plist.push_back(end_ - n_ * (thickness_ / 2 + offset) + vert_direc_ * (upper - descen) + u_ * offset);
        plist.push_back(start_ - n_ * (thickness_ / 2 + offset) + vert_direc_ * (upper - descen) - u_ * offset);
        return plist;
    }

    std::pair<LineCuboidRelation, std::pair<double, double>> HouseBarrier::HousingIntersectLineSegment(
        const Point &pnt1,
        const Point &pnt2,
        double floorheight,
        double offset,
        double walloffset) const
    {
        if (pnt1.distance(pnt2) < ABS_ERR)
            return make_pair(LineCuboidRelation::DISJOINT, make_pair(0, 0));
        shared_ptr<Cuboid> c(cu_);

        Point e = (pnt2 - pnt1).normalized();
        Point p1 = pnt1 - walloffset * e, p2 = pnt2 + walloffset * e;
        double t1, t2;
        vector<bool> surfs(6, false);
        unsigned cosurf = 0;
        LineCuboidRelation rel = c->IntersectLineSegment(p1, p2 - p1, t1, t2, surfs, ABS_ERR, REL_ERR);
        if (rel == LineCuboidRelation::DISJOINT)
            return make_pair(rel, make_pair(t1, t2));

        for (auto b : surfs)
            cosurf += b;
        if (rel == LineCuboidRelation::COINCIDENT && cosurf == 1)
        {
            if (surfs[CuboidSurface::BOTTOM] && c->get_base().z < ABS_ERR)
                rel = LineCuboidRelation::INTERSECTING;
            else if (surfs[CuboidSurface::TOP] && (c->get_base() + c->get_h()).z + ABS_ERR > floorheight)
                rel = LineCuboidRelation::INTERSECTING;
        }
        double totallen = p2.distance(p1);
        t1 *= totallen;
        t2 *= totallen;
        t1 -= walloffset;
        t2 -= walloffset;
        return make_pair(rel, make_pair(t1, t2));
    }

    void HouseBarrier::SubtleIntersectLine(const Point &pnt1, const Point &pnt2, double floorheight, vector<Point> &movable_direcs, double walloffset) const
    {
        for (size_t i = 0; i < movable_direcs.size();)
        {
            auto intr_result = HousingIntersectLineSegment(pnt1 + movable_direcs[i] * ABS_ERR * 1.5, pnt2 + movable_direcs[i] * ABS_ERR * 1.5, floorheight, 0.0, walloffset);
            if (intr_result.first == LineCuboidRelation::INTERSECTING)
                movable_direcs.erase(movable_direcs.begin() + i);
            else
                i++;
        }
    }

    bool HouseBarrier::IsContainPoint(const Point &pnt, double floorheight, double offset) const
    {
        Point newpnt = ChangeCoordinate(pnt);
        bool out = true;
        out &= fabs(newpnt.y) < thickness_ / 2 + offset - ABS_ERR;
        out &= -offset + ABS_ERR < newpnt.x && newpnt.x < length_ + offset - ABS_ERR;
        if (vert_direc_.IsSameDirection(Point(0, 0, 1)))
        {
            bool checkupper = height_ + start_.z <= floorheight - ABS_ERR;
            bool checklower = start_.z >= ABS_ERR;
            if (checkupper)
            {
                out &= newpnt.z < height_;
            }
            if (checklower)
            {
                out &= 0 < newpnt.z;
            }
        }
        else
        {
            out &= 0 < newpnt.z && newpnt.z < height_;
        }
        return out;
    }

    bool HouseBarrier::IsIntersectBarrier(const HouseBarrier &bar, double floorheight, double offset) const
    {
        unsigned ind[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {0, 4}, {1, 5}, {2, 6}, {3, 7}, {4, 5}, {5, 6}, {6, 7}, {7, 4}};
        unsigned k1, k2;
        pair<double, double> t;
        for (size_t i = 0; i < 12; i++)
        {
            k1 = ind[i][0];
            k2 = ind[i][1];
            auto &p1 = bar.eight_vertex_[k1];
            auto &p2 = bar.eight_vertex_[k2];
            auto result1 = HousingIntersectLineSegment(p1, p2, floorheight, offset);
            if (result1.first == LineCuboidRelation::INTERSECTING)
            {
                return true;
            }
            auto result2 = bar.HousingIntersectLineSegment(eight_vertex_[k1], eight_vertex_[k2], floorheight, offset);
            if (result2.first == LineCuboidRelation::INTERSECTING)
            {
                return true;
            }
        }
        return false;
    }

    void HouseInwallBarrier::set_ul(double _ul)
    {
        u_l_ = _ul;
        this->update_offset_cu();
    }
    void HouseInwallBarrier::set_ur(double _ur)
    {
        u_r_ = _ur;
        this->update_offset_cu();
    }
    void HouseInwallBarrier::set_zup(double _zup) { z_up_ = _zup; }
    void HouseInwallBarrier::set_zlow(double _zlow) { z_low_ = _zlow; }
    double HouseInwallBarrier::get_ul() const { return u_l_; }
    double HouseInwallBarrier::get_ur() const { return u_r_; }
    double HouseInwallBarrier::get_zup() const { return z_up_; }
    double HouseInwallBarrier::get_zlow() const { return z_low_; }
    string HouseInwallBarrier::get_host() const { return hostid; }
    void HouseInwallBarrier::set_thickness(double t)
    {
        thickness_ = t;
        this->update_cu();
    }

    void HouseInwallBarrier::update_offset_cu()
    {
        Point tmpbase = start_ - n_ * (thickness_ / 2) - offset_ * u_ - offset_ * n_;
        Point l = end_ - start_ + u_ * (2 * offset_);
        Point w = n_ * (thickness_ + 2 * offset_);
        Point h = vert_direc_ * height_;
        if ((l.norm() > 1e-2) && (w.norm() > 1e-2) && (h.norm() > 1e-2))
            offset_cu_.reset(new Cuboid(tmpbase, l, w, h));
        else
            offset_cu_.reset();

        tmpbase = start_ - n_ * (thickness_ / 2) + u_l_ * u_;
        l = (u_r_ - u_l_) * u_;
        w = n_ * thickness_;
        h = vert_direc_ * height_;
        if ((l.norm() > 1e-2) && (w.norm() > 1e-2) && (h.norm() > 1e-2))
            asym_cu_.reset(new Cuboid(tmpbase, l, w, h));
        else
            asym_cu_.reset();

        tmpbase = start_ - n_ * (thickness_ / 2) - offset_ * u_ - offset_ * n_ + u_l_ * u_;
        l = u_ * (2 * offset_) + (u_r_ - u_l_) * u_;
        w = n_ * (thickness_ + 2 * offset_);
        h = vert_direc_ * height_;
        if ((l.norm() > 1e-2) && (w.norm() > 1e-2) && (h.norm() > 1e-2))
            offset_asym_cu_.reset(new Cuboid(tmpbase, l, w, h));
        else
            offset_asym_cu_.reset();
    }

    bool HouseInwallBarrier::IsContainPoint(const Point &pnt, double floorheight, double offset) const
    {
        Point newpnt = ChangeCoordinate(pnt);
        bool out = true;
        out &= fabs(newpnt.y) < thickness_ / 2 + offset - ABS_ERR;
        out &= ABS_ERR < newpnt.x && newpnt.x < length_ - ABS_ERR;
        if (vert_direc_.IsSameDirection(Point(0, 0, 1)))
        {
            out &= newpnt.z < height_ - ABS_ERR;
            out &= ABS_ERR < newpnt.z;
        }
        else
        {
            out &= 0 < newpnt.z && newpnt.z < height_;
        }
        return out;
    }

    std::pair<LineCuboidRelation, std::pair<double, double>> HouseInwallBarrier::HousingIntersectLineSegment(
        const Point &pnt1,
        const Point &pnt2,
        double floorheight,
        double offset,
        double walloffset) const
    {
        if (pnt1.distance(pnt2) < ABS_ERR)
            return make_pair(LineCuboidRelation::DISJOINT, make_pair(0, 0));
        shared_ptr<Cuboid> c;

        if (fabs(offset) < ABS_ERR)
            c = cu_;
        else if (fabs(offset_ - offset) < ABS_ERR)
            c = offset_cu_;
        else
        {
            c.reset(new Cuboid(
                cu_->get_base() - offset * u_ - offset * n_,
                cu_->get_l() + offset * 2 * u_,
                cu_->get_w() + offset * 2 * n_,
                cu_->get_h()));
        }

        Point e = (pnt2 - pnt1).normalized();
        Point p1 = pnt1 - walloffset * e, p2 = pnt2 + walloffset * e;
        double t1, t2;
        vector<bool> surfs(6, false);
        unsigned cosurf = 0;
        LineCuboidRelation rel = c->IntersectLineSegment(p1, p2 - p1, t1, t2, surfs, ABS_ERR, REL_ERR);
        if (rel == LineCuboidRelation::DISJOINT)
            return make_pair(rel, make_pair(t1, t2));

        for (auto b : surfs)
            cosurf += b;
        if (rel == LineCuboidRelation::COINCIDENT && cosurf == 1)
        {
            if (surfs[CuboidSurface::FRONT] || surfs[CuboidSurface::BACK])
                rel = LineCuboidRelation::INTERSECTING;
        }
        double totallen = p2.distance(p1);
        t1 *= totallen;
        t2 *= totallen;
        t1 -= walloffset;
        t2 -= walloffset;
        return make_pair(rel, make_pair(t1, t2));
    }

    std::pair<LineCuboidRelation, std::pair<double, double>> HouseInwallBarrier::IntvIntersectLineSegment(
        const Point &pnt1,
        const Point &pnt2,
        double floorheight,
        double offset,
        double walloffset) const
    {
        if (pnt1.distance(pnt2) < ABS_ERR)
            return make_pair(LineCuboidRelation::DISJOINT, make_pair(0, 0));
        shared_ptr<Cuboid> c;

        if (fabs(offset) < ABS_ERR)
            c = asym_cu_;
        else if (fabs(offset_ - offset) < ABS_ERR)
            c = offset_asym_cu_;
        else
        {
            c.reset(new Cuboid(
                asym_cu_->get_base() - offset * u_ - offset * n_,
                asym_cu_->get_l() + offset * 2 * u_,
                asym_cu_->get_w() + offset * 2 * n_,
                asym_cu_->get_h()));
        }

        Point e = (pnt2 - pnt1).normalized();
        Point p1 = pnt1 - walloffset * e, p2 = pnt2 + walloffset * e;
        double t1, t2;
        vector<bool> surfs(6, false);
        unsigned cosurf = 0;
        LineCuboidRelation rel = c->IntersectLineSegment(p1, p2 - p1, t1, t2, surfs, ABS_ERR, REL_ERR); // CHECK_PTR_NOTNULL
        if (rel == LineCuboidRelation::DISJOINT)
            return make_pair(rel, make_pair(t1, t2));

        for (auto b : surfs)
            cosurf += b;
        if (rel == LineCuboidRelation::COINCIDENT && cosurf == 1)
        {
            if (surfs[CuboidSurface::FRONT] || surfs[CuboidSurface::BACK])
                rel = LineCuboidRelation::INTERSECTING;
        }
        double totallen = p2.distance(p1);
        t1 *= totallen;
        t2 *= totallen;
        t1 -= walloffset;
        t2 -= walloffset;
        return make_pair(rel, make_pair(t1, t2));
    }

    Point Door::get_inside_direc(const Point &pnt) const
    {
        Point newpnt = ChangeCoordinate(pnt);
        if (newpnt.z >= height_ - ABS_ERR)
            return vert_direc_ * (-1);
        if (newpnt.z <= ABS_ERR)
            return vert_direc_;
        if (newpnt.x <= ABS_ERR)
            return u_;
        if (newpnt.x >= length_ - ABS_ERR)
            return u_ * (-1);
        return Point(0, 0, 0);
    }

    Door Door::GetUnionWindoor(const Door &wd2) const
    {
        const Point *p[4] = {&start_, &end_, &wd2.start_, &wd2.end_};
        double x1, x2, y1, y2, z1, z2;
        for (size_t i = 0; i < 4; i++)
        {
            if (i == 0)
            {
                x1 = x2 = p[i]->x;
                y1 = y2 = p[i]->y;
                z1 = z2 = p[i]->z;
            }
            else
            {
                x1 = p[i]->x < x1 ? p[i]->x : x1;
                x2 = p[i]->x > x2 ? p[i]->x : x2;
                y1 = p[i]->y < y1 ? p[i]->y : y1;
                y2 = p[i]->y > y2 ? p[i]->y : y2;
                z1 = p[i]->z < z1 ? p[i]->z : z1;
                z2 = p[i]->z > z2 ? p[i]->z : z2;
            }
        }
        return Door(name_,
                       id_,
                       Point(x1, y1, z1),
                       Point(x2, y2, z1),
                       height_ > wd2.height_ ? height_ : wd2.height_,
                       thickness_ > wd2.thickness_ ? thickness_ : wd2.thickness_,
                       hostid,
                       BarrierType::DOOR);
    }

    Door &Door::operator=(const Door &wd)
    {
        name_ = wd.name_;
        id_ = wd.id_;
        start_ = wd.start_;
        end_ = wd.end_;
        length_ = wd.length_;
        height_ = wd.height_;
        thickness_ = wd.thickness_;
        type_ = wd.type_;
        eight_vertex_ = wd.eight_vertex_;
        vert_direc_ = wd.vert_direc_;
        u_ = wd.u_;
        n_ = wd.n_;
        u_l_ = wd.u_l_;
        u_r_ = wd.u_r_;
        z_up_ = wd.z_up_;
        z_low_ = wd.z_low_;
        hostid = wd.hostid;
        return *this;
    }

    pair<double, double> GetBarrierRange(const vector<HouseInwallBarrier> &wdlist, const Wall &wl, double pivotdist)
    {
        vector<pair<double, double>> intervals;

        // 所有门窗的区间
        for (const HouseInwallBarrier &wd : wdlist)
        {
            double t1 = ((wd.get_start() + wd.get_u() * wd.get_ul()) - wl.get_start()) * wl.get_u();
            double t2 = ((wd.get_start() + wd.get_u() * wd.get_ur()) - wl.get_start()) * wl.get_u();
            if (t1 > t2)
                std::swap(t1, t2);
            vector<pair<double, double>>::iterator it = intervals.begin();

            for (; it != intervals.end(); it++)
            {
                if (get<0>(*it) > t1)
                {
                    break;
                }
            }
            intervals.insert(it, make_pair(t1, t2));
        }

        if (!intervals.size())
        {
            return make_pair(wl.get_length() + 1, 0);
        }

        // 区间融合
        for (size_t k = 0; k < intervals.size() - 1;)
        {
            if (get<0>(intervals[k + 1]) < get<1>(intervals[k]))
            {
                double t1 = get<0>(intervals[k]);
                double t2 = MAX(get<1>(intervals[k]), get<1>(intervals[k + 1]));
                intervals[k] = make_pair(t1, t2);
                intervals.erase(intervals.begin() + k + 1);
            }
            else
                k++;
        }

        // 输出目标区间
        for (const pair<double, double> &item : intervals)
        {
            if (get<0>(item) <= pivotdist && pivotdist <= get<1>(item))
                return item;
        }

        return make_pair(wl.get_length() + 1, 0);
    }

    std::string GetBarrierTypeStr(BarrierType type)
    {
        switch (type)
        {
        case BarrierType::WALL:
            return "Wall";
        case BarrierType::BEAM:
            return "Beam";
        case BarrierType::BEARING:
            return "Bearing Wall";
        case BarrierType::WINDOW:
            return "Window";
        case BarrierType::DOOR:
            return "Door";
        case BarrierType::POWER_BOX:
            return "PowerBox";
        case BarrierType::WEAK_BOX:
            return "WeakBox";
        case BarrierType::OTHER_BARRIER:
            return "Barrier";
        default:
            return "Barrier";
        };
    }

}