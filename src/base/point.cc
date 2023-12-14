#include "base/point.h"
#include <cmath>

constexpr auto ERR = 1.0e-2;
template <typename L, typename R>
auto Min(const L &l, const R &r) -> decltype(l < r ? l : r) { return l < r ? l : r; }
template <typename L, typename R>
auto Max(const L &l, const R &r) -> decltype(l > r ? l : r) { return l > r ? l : r; }
#define PI 3.14159265358979323846

using namespace std;

namespace ewd
{
	Point::Point(double dx, double dy, double dz)
	{
		x = dx;
		y = dy;
		z = dz;
	}

	Point::Point(const Point &p) : x(p.x), y(p.y), z(p.z) {}

	Point::~Point() {}

	double Point::norm() const
	{
		return sqrt(x * x + y * y + z * z);
	}
	double Point::distance(const Point &v) const
	{
		return sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z));
	}

	Point Point::normalized() const
	{
		return *this / this->norm();
	}

	Point Point::cross(const Point &v) const
	{
		double x1 = y * v.z - z * v.y;
		double x2 = z * v.x - x * v.z;
		double x3 = x * v.y - y * v.x;
		return Point(x1, x2, x3);
	}
    
    double Point::dot(const Point &v) const{
        return (this->x*v.x+this->y*v.y+this->z*v.z);
    }

	Point Point::operator+(const Point &v) const
	{
		return Point(x + v.x, y + v.y, z + v.z);
	}

	Point Point::operator-(const Point &v) const
	{
		return Point(x - v.x, y - v.y, z - v.z);
	}

	Point Point::operator*(const double &t) const
	{
		return Point(t * x, t * y, t * z);
	}

	double Point::operator*(const Point &v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	Point Point::operator/(const double &t) const
	{
		return Point(x / t, y / t, z / t);
	}

	double Point::operator/(const Point &v) const
	{
		return (*this) * v / v.norm() / v.norm();
	}

	Point& Point::operator=(const Point &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	double Point::operator[](const int &i) const
	{
		if (i == 0 || i == -3)
			return x;
		else if (i == 1 || i == -2)
			return y;
		else if (i == 2 || i == -1)
			return z;
		else
			return numeric_limits<double>::quiet_NaN();
	}

	bool Point::operator==(const Point& v) const {
		if (((*this) - v).norm() < ERR)
			return true;
		else
			return false;
		return false;
	}

	bool Point::operator!=(const Point& v) const {
		if (((*this) - v).norm() >= ERR)
			return true;
		else
			return false;
		return false;
	}

	bool Point::IsParallel(const Point &v, double REL_ERR) const
	{
		if (this->norm() < REL_ERR || v.norm() < REL_ERR)
		{
			return true;
		}

		return (this->normalized().distance(v.normalized()) < REL_ERR) || ((this->normalized() + v.normalized()).norm() < REL_ERR);
	}

	bool Point::IsSameDirection(const Point &v, double REL_ERR) const
	{
		return IsParallel(v,REL_ERR) && ((*this) / v) > 0;
	}

	bool Point::IsWeakParallel(const Point &v, double REL_ERR, double WEAK_PARALLEL_ERR) const
	{
		double norm0 = norm();
		double normv = v.norm();

		if ((norm0 < REL_ERR) || (normv < REL_ERR))
			return true;

		double normal_dist = 0.0;
		double neg_normal_dist = 0.0;

		normal_dist += (x / norm0 - v.x / normv) * (x / norm0 - v.x / normv);
		normal_dist += (y / norm0 - v.y / normv) * (y / norm0 - v.y / normv);
		normal_dist += (z / norm0 - v.z / normv) * (z / norm0 - v.z / normv);
		normal_dist = std::sqrt(normal_dist);

		neg_normal_dist += (x / norm0 + v.x / normv) * (x / norm0 + v.x / normv);
		neg_normal_dist += (y / norm0 + v.y / normv) * (y / norm0 + v.y / normv);
		neg_normal_dist += (z / norm0 + v.z / normv) * (z / norm0 + v.z / normv);
		neg_normal_dist = std::sqrt(neg_normal_dist);
		return (normal_dist < WEAK_PARALLEL_ERR) || (neg_normal_dist < WEAK_PARALLEL_ERR);
	}

	bool Point::IsWeakSameDirection(const Point &v, double REL_ERR, double WEAK_PARALLEL_ERR) const
	{
		return IsWeakParallel(v, REL_ERR, WEAK_PARALLEL_ERR) && ((*this) * v) > 0;
	}


	bool IfLineIntersct(const Point &pnt1, const Point &pnt2, const Point &v1, const Point &v2, double ABS_ERR, double REL_ERR, double WEAK_PARALLEL_ERR)
	{
		Point ep = pnt2 - pnt1;
		Point ev = v2 - v1;
		double pnt_x_min = Min(pnt1.x, pnt2.x);
		double pnt_x_max = pnt1.x + pnt2.x - pnt_x_min;
		double pnt_y_min = Min(pnt1.y, pnt2.y);
		double pnt_y_max = pnt1.y + pnt2.y - pnt_y_min;
		double pnt_z_min = Min(pnt1.z, pnt2.z);
		double pnt_z_max = pnt1.z + pnt2.z - pnt_z_min;
		double v_x_min = Min(v1.x, v2.x);
		double v_x_max = v1.x + v2.x - v_x_min;
		double v_y_min = Min(v1.y, v2.y);
		double v_y_max = v1.y + v2.y - v_y_min;
		double v_z_min = Min(v1.z, v2.z);
		double v_z_max = v1.z + v2.z - v_z_min;

		if (pnt_x_max < v_x_min - ABS_ERR ||
			pnt_x_min > v_x_max + ABS_ERR ||
			pnt_y_max < v_y_min - ABS_ERR ||
			pnt_y_min > v_y_max + ABS_ERR ||
			pnt_z_max < v_z_min - ABS_ERR ||
			pnt_z_min > v_z_max + ABS_ERR)
		{
			return false;
		}

		if (ep.norm() < REL_ERR)
		{
			Point tmp = pnt1 - v1;
			if ((tmp).IsParallel(ev,REL_ERR) && 0 <= tmp / ev && tmp / ev <= 1)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		if (ev.norm() < REL_ERR)
		{
			Point tmp = v1 - pnt1;
			if (ep.IsParallel(tmp,REL_ERR) && 0 <= tmp / ep && tmp / ep <= 1)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		if (ep.IsParallel(ev,REL_ERR))
		{
			if (ep.IsParallel(pnt2 - v1,REL_ERR) || ep.IsParallel(pnt1 - v1,REL_ERR) || ep.IsParallel(v2 - pnt1,REL_ERR))
			{
				// 共线
				double cand[4];
				cand[0] = (v1 - pnt1) / ep;
				cand[1] = (v2 - pnt1) / ep;
				cand[2] = (pnt1 - v1) / ev;
				cand[3] = (pnt2 - v1) / ev;
				for (size_t i = 0; i < 4; i++)
				{
					if (cand[i] <= 1 - REL_ERR && cand[i] >= REL_ERR)
					{
						return true;
					}
				}
				return false;
			}
			else
			{
				// 平行
				return false;
			}
		}

		Point n = ep.cross(ev).normalized();
		if (fabs(n * (pnt1 - v1)) < REL_ERR)
		{
			double cr1 = (v1 - pnt1).cross(ep) * (v2 - pnt1).cross(ep);
			double cr2 = (pnt1 - v1).cross(ev) * (pnt2 - v1).cross(ev);
			if (cr1 > 0 || cr2 > 0)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			// 异面
			return false;
		}
	}

	Point ChangeCoordinate(const Point &pnt, const Point &x, const Point &y, const Point &z)
	{
		return Point(pnt * x, pnt * y, pnt * z);
	}


	bool LineContainPoint(const Point &start, const Point& direc, LineType type, const Point &pnt, double& t)
	{
		Point p = pnt - start;
		if ( !p.IsParallel(direc, 1e-6))
			return false;
		t = p/direc;
		return !(((type != LineType::LINE) && (t < - 1e-6)) || 
				 ((type == LineType::SEGMENT) && (t> 1+1e-6)));
	}

	LineRelation LineIntersectLine(const Point & p1, const Point& d1, LineType type1,
								   const Point& p2, const Point& d2, LineType type2, 
								   double& t1, double& t2)
	{
		Point e1 = d1.normalized(), e2 = d2.normalized();
		if(e1.IsParallel(e2))
		{
			// if(!(p2-p1).IsParallel(e1))
			if((p2-p1).cross(e1).norm()>1e-2)
				return LineRelation::DISJOINT;
			double p2sofp1 = (p2-p1)*d1/(d1.norm() * d1.norm());
			double p2tofp1 = (p2+d2-p1)*d1/(d1.norm() * d1.norm());
			double p1sofp2 = (p1-p2)*d2/(d2.norm() * d2.norm());
			double p1tofp2 = (p1+d1-p2)*d2/(d2.norm() * d2.norm());
			if((p2sofp1 > -1e-6 || type1 == LineType::LINE)&&(p2sofp1 < 1+1e-6 || type1 != LineType::SEGMENT))
				return LineRelation::COINCIDENT;
			if((p2tofp1 > -1e-6 || type1 == LineType::LINE)&&(p2tofp1 < 1+1e-6 || type1 != LineType::SEGMENT))
				return LineRelation::COINCIDENT;
			if((p1sofp2 > -1e-6 || type2 == LineType::LINE)&&(p1sofp2 < 1+1e-6 || type2 != LineType::SEGMENT))
				return LineRelation::COINCIDENT;
			if((p1tofp2 > -1e-6 || type2 == LineType::LINE)&&(p1tofp2 < 1+1e-6 || type2 != LineType::SEGMENT))
				return LineRelation::COINCIDENT;
			return LineRelation::DISJOINT;
		}
			
		Point n = e1.cross(e2);
		if(fabs(n*(p1-p2)) < 1e-6)
		{
			Point proj2to1 = p1 + d1 * ((p2-p1)*d1/(d1.norm() * d1.norm()));
			if(proj2to1.distance(p2) < 1e-6)
			{
				t2 = 0;
				t1 = (p2-p1)*d1/(d1.norm() * d1.norm());
			}
			else
			{
				double cos = d1 * d2 / (d1.norm() * d2.norm());
				double sin = sqrt(1 - cos * cos);
				Point cross = proj2to1 + d1/d1.norm() * (proj2to1 - p2).norm() * cos/sin;
				t1 = (cross-p1)*d1/(d1.norm() * d1.norm());
				t2 = (cross-p2)*d2/(d2.norm() * d2.norm());
			}
			if((t1< -1e-6 && type1 != LineType::LINE) || (t1 > 1+1e-6 && type1 == LineType::SEGMENT))
				return LineRelation::DISJOINT;
			if((t2< -1e-6 && type2 != LineType::LINE) || (t2 > 1+1e-6 && type2 == LineType::SEGMENT))
				return LineRelation::DISJOINT;
			return LineRelation::INTERSECTING;
		}
		return LineRelation::DISJOINT;
	}
}