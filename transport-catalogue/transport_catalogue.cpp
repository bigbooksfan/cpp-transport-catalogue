#include "transport_catalogue.h"

#include <stdexcept>
#include <algorithm>

#include "input_reader.h"

void transport_system::transport_catalogue::AddStop(
	const std::string& name, const double lat, const double lon) {
	if (stops_.count(name) != 0)
		throw std::logic_error("Stop name is not unique");
	stops_.insert(Stop{ name, { lat, lon} });
}

void transport_system::transport_catalogue::AddBus(
	const std::string& name, const std::string& raw_desc) {
	if (buses_.count(name) != 0)
		throw std::logic_error("Bus name is not unique");
	buses_.insert(Bus{ name, raw_desc });
}

const transport_system::transport_catalogue::Stop& 
transport_system::transport_catalogue::FindStop(
	const std::string& name) const {
	if (stops_.count(name) == 0)
		throw std::logic_error("Search of non-existent stop");
	return *stops_.find(name);
}

const transport_system::transport_catalogue::Bus& 
transport_system::transport_catalogue::FindBus(
	const std::string& name) const {
	if (buses_.count(name) == 0)
		throw std::logic_error("Search of non-existent bus");
	return *buses_.find(name);
}

void transport_system::transport_catalogue::PutBusesOnStop(const Bus& bus) {
	for (Stop* stop : dummy_.way_) {
		if (buses_on_stop_.count(stop) == 0) {
			buses_on_stop_.emplace(stop, std::unordered_set<Bus*>{});
		}
		buses_on_stop_.at(stop).insert(const_cast<Bus*>(&bus));
	}
}

transport_system::transport_catalogue::Stop* 
transport_system::transport_catalogue::LinkTextToStop(const std::string& in) {
	std::unordered_set<Stop>::iterator It = stops_.find(in);
	if (It == stops_.end()) {
		throw std::logic_error("Non-existent Stop");
	}
	const transport_system::transport_catalogue::Stop* cpt = &(*It);
	transport_system::transport_catalogue::Stop* pt =
		const_cast<transport_system::transport_catalogue::Stop*>(cpt);
	return pt;
}

const std::set<std::string> transport_system::transport_catalogue::GetBusesOfStop(const std::string StopName) {
	std::set<std::string> ret;

	std::unordered_set<Stop, StringHasher<Stop>>::iterator It = stops_.find(StopName);
	if (It != stops_.end()) {
		const transport_system::transport_catalogue::Stop* cpt = &(*It);
		transport_system::transport_catalogue::Stop* pt =
			const_cast<transport_system::transport_catalogue::Stop*>(cpt);
		if (buses_on_stop_.count(pt)) {
			for (const Bus* i : buses_on_stop_.at(pt)) {
				ret.insert(i->name_);
			}
		}
	}

	return ret;
}

const transport_system::transport_catalogue::Route& transport_system::transport_catalogue::GetRoute(const std::string& BusName) {
	std::unordered_map<std::string, transport_system::transport_catalogue::Route>::iterator It
		= routes_.find(BusName);
	
	return It != routes_.end() ? It->second : empty_;
}

size_t transport_system::transport_catalogue::CalcUniques(const Route& route) {
	std::set<Stop*> each;
	for (const Stop* stop : route.way_) {
		const Stop* tmp1 = stop;
		Stop* tmp = const_cast<Stop*>(tmp1);
		each.insert(tmp);
	}
	return each.size();
}

void transport_system::transport_catalogue::CalcDistances(transport_system::transport_catalogue::Route& route) {
	for (size_t i = 1, j = route.way_.size(); i < j; ++i) {
		transport_system::transport_catalogue::Stop*& a = route.way_[i - 1];
		transport_system::transport_catalogue::Stop*& b = route.way_[i];

		double dist = DistBetween(a, b);
		double exact_dist = 
			(exact_distances_.count(a) && 
				exact_distances_.at(a).count(b)) ?
			exact_distances_.at(a).at(b) : dist;

		route.geo_length_ += geo_distances_.at(a).at(b);
		route.length_ += dist;
		route.exact_length_ += exact_dist;
	}
}

void transport_system::transport_catalogue::ClarifyDistance(const std::string_view stop1, const std::string_view stop2, double dist) {
	Stop* from = LinkTextToStop(std::string(stop1));
	Stop* to = LinkTextToStop(std::string(stop2));

	exact_distances_[from][to] = dist;
	distances_[from][to] = dist;
	distances_[to][from] = dist;
}

double transport_system::transport_catalogue::DistBetween(
	transport_system::transport_catalogue::Stop*& a, 
	transport_system::transport_catalogue::Stop*& b) {

	if (distances_.count(a) == 0 || distances_.at(a).count(b) == 0) {
		double dist = geo::ComputeDistance(a->coords_, b->coords_);
		distances_[a][b] = dist;
		distances_[b][a] = dist;
	}
	
	if (geo_distances_.count(a) == 0 || geo_distances_.at(a).count(b) == 0) {
		double dist = geo::ComputeDistance(a->coords_, b->coords_);
		geo_distances_[a][b] = dist;
		geo_distances_[b][a] = dist;
	}

	return distances_.at(a).at(b);
}

bool transport_system::transport_catalogue::Stop::operator<(const Stop& other) const {
	return name_ < other.name_;
}

bool transport_system::transport_catalogue::Stop::operator==(const Stop& other) const {
	return name_ == other.name_;
}

bool transport_system::transport_catalogue::Bus::operator<(const Bus& other) const {
	return name_ < other.name_;
}

bool transport_system::transport_catalogue::Bus::operator==(const Bus& other) const {
	return name_ == other.name_;
}

const std::unordered_set<transport_system::transport_catalogue::Bus
	, transport_system::transport_catalogue::StringHasher<transport_system::transport_catalogue::Bus>>&
	transport_system::transport_catalogue::GetBuses() {

	return buses_;
}

const std::unordered_map<std::string
	, transport_system::transport_catalogue::Route>&
	transport_system::transport_catalogue::GetRoutes() {

	return routes_;
}

void transport_system::transport_catalogue::SetDummy
(transport_system::transport_catalogue::Route& dummy) {
	dummy_ = dummy;
}

void transport_system::transport_catalogue::InsertRoute(const std::string& bus_name) {
	routes_.insert({ bus_name, dummy_ });
}