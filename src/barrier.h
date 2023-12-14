#pragma once
#include <string>
#include <tuple>
#include <vector>
#include "base/cuboid.h"
#include "base/point.h"

namespace ewd
{
	enum class BarrierType
	{
		WALL,	   // 普通墙
		BEARING,   // 承重墙
		BEAM,	   // 房梁
		WINDOW,	   // 窗
		DOOR,	   // 门
		POWER_BOX, // 弱电箱
		WEAK_BOX,  // 强电箱
		OTHER_BARRIER
	};

	class HouseBarrier
	{
	public:
		HouseBarrier(const std::string &sname,
					 const std::string &sid,
					 const Point &ps,
					 const Point &pe,
					 double h,
					 double thick,
					 BarrierType _type = BarrierType::WALL);
		HouseBarrier(const HouseBarrier &br);
		~HouseBarrier();

	protected:
		double ABS_ERR = 10;
		double REL_ERR = 0.01;
		double WEAK_PARALLEL_ERR = 0.3;
		std::string name_;
		std::string id_;
		Point start_;
		Point end_;
		double length_, height_, thickness_;
		BarrierType type_;
		std::vector<Point> eight_vertex_;

		Point vert_direc_ = Point(0, 0, 1);
		Point u_;
		Point n_;

		double offset_ = 0.0;
		std::shared_ptr<Cuboid> cu_ = nullptr;
		std::shared_ptr<Cuboid> offset_cu_ = nullptr;

		Point ChangeCoordinate(const Point &pnt) const;

		void establish8vertices();
		virtual void update_cu();
		virtual void update_offset_cu();

	public:
		void set_vert_direc(const Point &p);
		void set_offset(double off);

		const Cuboid &cu() const { return *cu_; }
		const Cuboid &offset_cu() const { return *offset_cu_; }

		std::string get_name() const;
		std::string get_id() const;
		Point get_start() const;
		Point get_end() const;
		double get_length() const;
		double get_height() const;
		double get_thickness() const;
		BarrierType get_type() const;
		Point get_u() const;
		Point get_n() const;
		Point get_vert_direc() const;
		std::vector<Point> GetCorners(double offset = 0.0, double descen = 0.0) const;
		std::vector<Point> GetCorners(double offset, double descen, double floorheight) const;
		bool allow_through() const;
		Point Project(const Point &pnt, double depth = 0.0) const;
		virtual std::pair<LineCuboidRelation, std::pair<double, double>> HousingIntersectLineSegment(const Point &pnt1, const Point &pnt2, double floorheight, double offset = 0.0, double expand = 0.0) const;
		virtual bool IsContainPoint(const Point &pnt, double floorheight, double offset = 0.0) const;
		virtual bool IsIntersectBarrier(const HouseBarrier &bar, double floorheight, double offset = 0.0) const;

		/**
		 * @brief 计算一线段沿某些方向平移后是否与该障碍物相交
		 * 然后将相交的平移方向删去
		 * @param pnt1 线段起点
		 * @param pnt2 线段终点
		 * @param floorheight 层高
		 * @param movable_direcs 可移动的方向
		 * @param expand 将线段沿两端的延长量（因与其他墙体相交而出现的量）
		 */
		void SubtleIntersectLine(const Point &pnt1, const Point &pnt2, double floorheight, std::vector<Point> &movable_direcs, double expand) const;
	};

	class Wall : public HouseBarrier
	{
	public:
		Wall(const std::string &sname,
			 const std::string &sid,
			 const Point &ps,
			 const Point &pe,
			 double h,
			 double thick,
			 BarrierType _type = BarrierType::WALL) : HouseBarrier(sname, sid, ps, pe, h, thick, _type){};
		Wall(const Wall &wl) : HouseBarrier(wl.name_, wl.id_, wl.start_, wl.end_, wl.height_, wl.thickness_, wl.type_){};
	};

	class HouseInwallBarrier : public HouseBarrier
	{
	public:
		HouseInwallBarrier(const std::string &sname,
						   const std::string &sid,
						   const Point &ps,
						   const Point &pe,
						   double h,
						   double thick,
						   const std::string &_hostid,
						   BarrierType _type) : HouseBarrier(sname, sid, ps, pe, h, thick, _type), hostid(_hostid)
		{
			u_l_ = 0;
			u_r_ = length_;
			z_low_ = start_.z;
			z_up_ = start_.z + height_;
		}

		void set_thickness(double t);

		/**
		 * @brief 设置u方向的碰撞范围左端点
		 * 默认为0
		 * @param _ul
		 */
		void set_ul(double _ul);

		/**
		 * @brief 设置u方向的碰撞范围右端点
		 * 默认为length_
		 * @param _ur
		 */
		void set_ur(double _ur);

		/**
		 * @brief 设置z方向的碰撞范围上端点
		 * 默认为start_.z+height_
		 * @param _zup
		 */
		void set_zup(double _zup);

		/**
		 * @brief 设置z方向的碰撞范围下端点
		 * 默认为start_.z
		 * @param _zlow
		 */
		void set_zlow(double _zlow);

		/**
		 * @brief 获取u方向的碰撞范围左端点
		 *
		 * @return double
		 */
		double get_ul() const;

		/**
		 * @brief 获取u方向的碰撞范围右端点
		 *
		 * @return double
		 */
		double get_ur() const;

		/**
		 * @brief 获取z方向的碰撞范围上端点
		 *
		 * @return double
		 */
		double get_zup() const;

		/**
		 * @brief 获取z方向的碰撞范围下端点
		 *
		 * @return double
		 */
		double get_zlow() const;
		std::string get_host() const;
		bool IsContainPoint(const Point &pnt, double floorheight, double offset = 0.0) const;

		std::pair<LineCuboidRelation, std::pair<double, double>> HousingIntersectLineSegment(const Point &pnt1, const Point &pnt2, double floorheight, double offset = 0.0, double expand = 0.0) const override;

		std::pair<LineCuboidRelation, std::pair<double, double>> IntvIntersectLineSegment(const Point &pnt1, const Point &pnt2, double floorheight, double offset = 0.0, double expand = 0.0) const;

	protected:
		double u_l_, u_r_, z_up_, z_low_;
		std::string hostid;
		std::shared_ptr<Cuboid> asym_cu_ = nullptr, offset_asym_cu_ = nullptr;
		void update_offset_cu() override;
	};

	class Door : public HouseInwallBarrier
	{
	public:
		/**
		 * @brief Construct a new Door object
		 * 
		 * @param sname 
		 * @param sid 
		 * @param ps 
		 * @param pe 
		 * @param h 
		 * @param thick 
		 * @param _hostid 
		 * @param _type 
		 */
		Door(const std::string &sname,
			 const std::string &sid,
			 const Point &ps,
			 const Point &pe,
			 double h,
			 double thick,
			 const std::string &_hostid,
			 BarrierType _type) : HouseInwallBarrier(sname, sid, ps, pe, h, thick, _hostid, _type) {}
		Door(const Door &wd) : HouseInwallBarrier(wd.name_, wd.id_, wd.start_, wd.end_, wd.height_, wd.thickness_, wd.hostid, wd.type_)
		{
			u_l_ = wd.u_l_, u_r_ = wd.u_r_, z_up_ = wd.z_up_, z_low_ = wd.z_low_;
		}
		Door GetUnionWindoor(const Door &wd2) const;
		Door &operator=(const Door &wd);
		Point get_inside_direc(const Point &pnt) const;
	};

	/**
	 * @brief 获取某些门窗在某墙体上u方向上的分布范围
	 * 只考虑包含pivotdist的区间
	 * @param wdlist 门窗列表
	 * @param wl 墙体
	 * @param pivotdist 关键点
	 * @return std::pair<double, double> 区间
	 */
	std::pair<double, double> GetBarrierRange(const std::vector<HouseInwallBarrier> &wdlist, const Wall &wl, double pivotdist);
	std::string GetBarrierTypeStr(BarrierType type);

}
