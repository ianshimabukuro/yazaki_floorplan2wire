#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <tuple>

namespace ewd
{
	/**
	 * @brief 三维点类，同时也是向量
	 * 
	 */
	class Point
	{
	public:
		double x;
		double y;
		double z;

		Point(double x_val=0.0, double y_val=0.0, double z_val=0.0);
		Point(const Point &p);
		~Point();
		

		double norm() const;
		double distance(const Point &v) const;
		Point cross(const Point &v) const;
        double dot(const Point &v) const;
		Point normalized() const;

		Point operator+(const Point &v) const;
		Point operator-(const Point &v) const;
		Point operator*(const double &t) const;
		friend Point operator*(double t, const Point& p) {return p*t;}
		double operator*(const Point &v) const;
		Point operator/(const double &t) const;
		double operator/(const Point &v) const;
		double operator[](const int &i) const;
		Point& operator=(const Point &v);
		bool operator==(const Point& v) const;
		bool operator!=(const Point& v) const;

		/**
		 * @brief 判断当前向量与另一向量是否平行
		 * 
		 * @param v 向量
		 * @param REL_ERR 精度
		 * @return true 
		 * @return false 
		 */
		bool IsParallel(const Point &v, double REL_ERR = 0.01) const;

		/**
		 * @brief 判断当前向量与另一向量是否同向
		 * 
		 * @param v 向量
		 * @param REL_ERR 精度
		 * @return true 
		 * @return false 
		 */
		bool IsSameDirection(const Point &v, double REL_ERR = 0.01) const;

		
		/**
		 * @brief 判断当前向量与另一向量是否弱平行
		 * 弱平行即 差不多平行 ，比如(0,0,1)与(0,0,1.1)弱平行
		 * @param v 另一向量
		 * @param REL_ERR 精度
		 * @param WEAK_PARALLEL_ERR 弱平行误差界（需要小于1/sqrt(2)，约0.7）
		 * @return true 
		 * @return false 
		 */
		bool IsWeakParallel(const Point &v, double REL_ERR = 0.01,double WEAK_PARALLEL_ERR = 0.3) const;
		
		/**
		 * @brief 判断当前向量与另一向量是否弱同向
		 * 弱同向即 差不多同向 ，比如(0,0,1)与(0,0,1.1)弱同向
		 * @param v 另一向量
		 * @param REL_ERR 精度
		 * @param WEAK_PARALLEL_ERR 弱同向误差界（需要小于1/sqrt(2)，约0.7）
		 * @return true 
		 * @return false 
		 */
		bool IsWeakSameDirection(const Point &v, double REL_ERR = 0.01,double WEAK_PARALLEL_ERR = 0.3) const;
		

		friend std::ostream &operator<<(std::ostream &out, const Point &p)
		{
			out << "[" << p.x << "," << p.y << "," << p.z << "]";
			return out;
		}
		
	};
	

	/**
	 * @brief 直线种类
	 * 
	 */
	enum class LineType
	{
		SEGMENT,
		RAY,
		LINE
	};

	enum class LineRelation
	{
		COINCIDENT,
		INTERSECTING,
		DISJOINT
	};
	
	/**
	 * @brief 判断两线段是否相交
	 * 
	 * @param pnt1 第一线段的起点
	 * @param pnt2 第一线段的终点
	 * @param v1 第二线段的起点
	 * @param v2 第二线段的终点
	 * @param ABS_ERR 绝对误差
	 * @param REL_ERR 精度
	 * @param WEAK_PARALLEL_ERR 判断弱平行的精度
	 * @return true 
	 * @return false 
	 */
	bool IfLineIntersct(const Point &pnt1, const Point &pnt2, const Point &v1, const Point &v2, double ABS_ERR = 10, double REL_ERR = 0.01, double WEAK_PARALLEL_ERR = 0.3);
	
	/**
	 * @brief 计算一点在另一组正交基下的坐标
	 * 
	 * @param pnt 被计算点
	 * @param x 第一个基向量
	 * @param y 第二个基向量
	 * @param z 第三个基向量
	 * @return Point 
	 */
	Point ChangeCoordinate(const Point &pnt, const Point &x, const Point &y, const Point &z);
	
	bool LineContainPoint(const Point &start, const Point & direct, LineType type,const Point &point, double& t);

	LineRelation LineIntersectLine(const Point & p1, const Point& d1, LineType type1,
								   const Point& p2, const Point& d2, LineType type2, 
								   double& t1, double& t2);
}
