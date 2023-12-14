#pragma once
#include <vector>
#include <map>
#include <set>
#include "base/point.h"
#include "base/graph.h"
#include "barrier.h"


namespace ewd
{
	using std::map;
	using std::set;
	using std::string;
	using std::vector;

	class Device{
    public:
		string id;
		Point location;	// 位置
		string attach_wall_id; // 附着墙ID
		string name;  // 设备类型

        Device() {}
        /**
         * @brief Construct a new Device object
         * 
         * @param id 
         * @param name 
         * @param location 
         * @param attach_wall_id 
         */
        Device(string id, string name, Point location, string attach_wall_id) :
            id(id), name(name), location(location), attach_wall_id(attach_wall_id){
        }
        ~Device() {}
	};

    struct Config
    {
        double floor_height = 3300.0;
        double offset_door = 0.0;

        double connect_threshold = 200;
        double mini_radius = 50.0;
        double live_wire_unit_cost = 1.0;
		double neutral_wire_unit_cost = 1.0;
		double earth_wire_unit_cost = 1.0;
		double through_wall_conduit_unit_cost = 1.0;
		double conduit_unit_cost = 1.0;
		double in_groove_conduit_unit_cost = 1.0;
    };

    class GraphConstructor
    {
    public:
        double ABS_ERR = 10;
        double REL_ERR = 0.01;

        double floor_height = 3300.0;
        double offset_door = 0.0;

        double connect_threshold = 200;
		double mini_radius = 0.0;
		double live_wire_unit_cost = 1.0;
		double neutral_wire_unit_cost = 1.0;
		double earth_wire_unit_cost = 1.0;
		double through_wall_conduit_unit_cost = 1.0;
		double conduit_unit_cost = 1.0;
		double in_groove_conduit_unit_cost = 1.0;

        std::vector<Wall> walls_;
        std::map<std::string, size_t> wall_id_map_;
        std::vector<Door> doors_;

        GeometricGraph g;
        Device PSB;
        size_t PSB_index;
		std::vector<Device> devices;
		std::vector<size_t> devices_indices;

        GraphConstructor();
        ~GraphConstructor();

        void construct();

        void add_wall(const Wall &wl);
        void add_door(const Door &wd);
        void set_PSB(const Device& dev);
		void add_device(const Device& dev);

        void read_config(const Config& conf);

        void set_floor_height(double height);
        void set_offset_door(double off);
        void set_mini_radius(double r);
		void set_conduit_unit_cost(double c);
		void set_in_groove_conduit_unit_cost(double c);
		void set_through_wall_conduit_unit_cost(double c);
		void set_live_wire_unit_cost(double c);
		void set_neutral_wire_unit_cost(double c);
		void set_earth_wire_unit_cost(double c);
		void set_connect_threshold(double t);

        size_t num_vertex() const;
        size_t num_edge() const;
        Point vertex(size_t i) const;
        Edge edge(size_t k) const;

    // private:

        EdgeIndex add_edge(VertexIndex v1, VertexIndex v2, bool further_check=false);

        /**
         * @brief 给定某线段和该线段所在的墙体，获取该线段与所有其他墙体的碰撞范围
         * 
         * @param wl 线段所在墙体
         * @param start 线段起点
         * @param end 线段终点
         * @param offset 与墙体的间距
         * @param forcing 无用的参数，下次删
         * @return std::vector<std::tuple<double, double, std::string>> 
         *         输出序列的元素为碰撞范围的左右边界+墙体id，
         *         记左右边界为a,b，则碰撞范围为start + t * u, t in [a,b], u = (end - start) / ||end - start||
         */
        std::vector<std::tuple<double, double, std::string>> GetCrossPoints(const Wall &wl, const Point &start, const Point &end, double offset = 0.0, bool forcing = false) const;


        /**
         * @brief 判断线段是否与不可穿越的障碍物相交
         * 墙体间距的范围内，即认为相交
         * @param pnt0 线段起点
         * @param pnt1 线段终点
         * @param offset 与墙体间距
         * @param checkwindoor 是否检查与门窗相交
         * @return true 
         * @return false 
         */
        bool LnThroughNotPass(const Point &pnt0, const Point &pnt1, double offset = 0.0, bool checkwindoor=true) const;

        std::map<LineCuboidRelation,double> intersection_analysis(const Point& pnt1, const Point& pnt2) const ;

        int DoorProcess();
        bool CheckConnect(std::set<size_t> &pointnums) const;
        void WallsPreprocess();

		void calc_costs();
		void finalDeletingCheck();
        bool valid_point(const Point& p) const;
        void collect_wall_grid(std::vector<double>& xs, std::vector<double>& ys);
        void collect_door_grid(std::vector<double>& xs, std::vector<double>& ys);
        void collect_device_grid(std::vector<double>& xs, std::vector<double>& ys);
        void Hanan(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& zs);
    };

} 