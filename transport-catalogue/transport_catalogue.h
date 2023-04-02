#pragma once

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <functional>

#include "geo.h"

namespace transport_system {
	class transport_catalogue {
	public:			// structs
		struct Stop {
			std::string name_;
			geo::Coordinates coords_;

			// constructor for searching by the name with calling stops_.count(name)
			Stop(const std::string& name = "", const geo::Coordinates& coords = geo::Coordinates{})
				: name_(name), coords_(coords) { };

			bool operator<(const Stop& other) const;
			bool operator==(const Stop& other) const;
		};

		struct Bus {
			std::string name_;
			std::string raw_route_;

			Bus(const std::string& name = "", const std::string& raw_route = "")
				: name_(name), raw_route_(raw_route) { }

			bool operator<(const Bus& other) const;
			bool operator==(const Bus& other) const;
		};

		// HINT : struct { vector<sv> stops , unique_stops, length }
		struct Route {
			std::vector<Stop*> way_;
			size_t unique_stops_ = 0;
			double geo_length_ = 0.0;
			double length_ = 0.0;
			double exact_length_ = 0.0;
			double curvature_ = 1.0;
		};

		// T = struct Bus or struct Stop
		template <typename T>
		struct StringHasher {
			size_t operator()(const T& stop) const {
				return std::hash<std::string>{}(stop.name_);
			}
		};

	private:			// fields
		// HINT : u_set < struct Stop { string name , Coords } >
		std::unordered_set<Stop, StringHasher<Stop>> stops_;
		// HINT : u_set < struct Bus { name , raw route } >
		std::unordered_set<Bus, StringHasher<Bus>> buses_;
		// HINT : u_map < bus name , vector < string_view Stops> >
		std::unordered_map<std::string, Route> routes_;
		// HINT : u_map < Stop* , set < Bus* > >
		std::unordered_map<Stop*, std::unordered_set<Bus*>> buses_on_stop_;
		
		Route dummy_;
		Route empty_;

		// HINT : map < Stop1* , map < Stop2* , distance > >
		std::map<Stop*, std::map<Stop*, double>> geo_distances_;
		// HINT : map < Stop1* , map < Stop2* , distance > >
		std::map<Stop*, std::map<Stop*, double>> distances_;
		// HINT : map < Stop1* , map < Stop2* , distance > >
		std::map<Stop*, std::map<Stop*, double>> exact_distances_;

	public:				// field for TESTS
		std::vector<std::string> outrows_;

	public:				// constructors
		transport_catalogue() = default;

	public:				// methods
		void AddBus(const std::string& name, const std::string& raw_desc);
		void AddStop(const std::string& name, const double lat, const double lon);

		void ClarifyDistance(const std::string_view stop1, const std::string_view stop2, double dist);
		void SetDummy(Route& dummy);
		void InsertRoute(const std::string& bus_name);
		void PutBusesOnStop(const Bus& bus);

		// information out
		const Route& GetRoute(const std::string& BusName);
		const std::unordered_set<Bus, StringHasher<Bus>>& GetBuses();
		const std::unordered_map<std::string, Route>& GetRoutes();
		const std::set<std::string> GetBusesOfStop(const std::string StopName);

		const Stop& FindStop(const std::string& name) const;
		const Bus& FindBus(const std::string& name) const;

		Stop* LinkTextToStop(const std::string& in);
		size_t CalcUniques(const Route& route);
		void CalcDistances(Route& route);
		double DistBetween(Stop*& a, Stop*& b);				
	};
}			// namespace transport_system