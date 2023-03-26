#include "transport_catalogue.h"

#include <stdexcept>
#include <algorithm>

#include "input_reader.h"

transport_system::transport_catalogue::transport_catalogue(std::string& number) {
	InputReader::input_reader reader(std::stoi(number), this);
	ParseRoutes();
}

transport_system::transport_catalogue::transport_catalogue(size_t number) {
	InputReader::input_reader reader(number, this);
	ParseRoutes();
}

void transport_system::transport_catalogue::AddStop(const std::string& name, const double lat, const double lon) {
	if (stops_.count(name) != 0)
		throw std::logic_error("Stop name is not unique");
	stops_.insert(Stop{ name, { lat, lon} });
}

void transport_system::transport_catalogue::AddBus(const std::string& name, const std::string& raw_desc) {
	if (buses_.count(name) != 0)
		throw std::logic_error("Bus name is not unique");
	buses_.insert(Bus{ name, raw_desc });
}

const transport_system::transport_catalogue::Stop& transport_system::transport_catalogue::FindStop(
	const std::string& name) const {
	if (stops_.count(name) == 0)
		throw std::logic_error("Search of non-existent stop");
	return *stops_.find(name);
}

const transport_system::transport_catalogue::Bus& transport_system::transport_catalogue::FindBus(
	const std::string& name) const {
	if (buses_.count(name) == 0)
		throw std::logic_error("Search of non-existent bus");
	return *buses_.find(name);
}

void transport_system::transport_catalogue::ParseRoutes() {
	for (const Bus& bus : buses_) {
		if (routes_.count(bus.name_) != 0)
			throw std::logic_error("Bus name is not unique");

		bool circular = bus.raw_route_.find(BusCircleDelimeter) != std::string::npos;
		bool linear = bus.raw_route_.find(BusLineDelimeter) != std::string::npos;

		if (circular && linear) {
			throw std::logic_error("Raw route is both linear and circular");
		}		
		if (!circular && !linear) {
			throw std::logic_error("Raw route is not linear or circular");
		}

		if (circular) {
			dummy_ = ParseCircularRoute(bus.raw_route_);
			routes_.insert({ bus.name_, dummy_});
		}
		else {		// linear route
			dummy_ = ParseLinearRoute(bus.raw_route_);
			routes_.insert({ bus.name_, dummy_ });
		}

		PutBusesOnStop(bus);
	}
}

void transport_system::transport_catalogue::PutBusesOnStop(const Bus& bus) {
	for (Stop* stop : dummy_.way_) {
		if (buses_on_stop_.count(stop) == 0) {
			buses_on_stop_.emplace(stop, std::unordered_set<Bus*>{});
		}
		buses_on_stop_.at(stop).insert(const_cast<Bus*>(&bus));
	}
}

transport_system::transport_catalogue::Stop* transport_system::transport_catalogue::LinkTextToStop(const std::string& in) {
	//if (stops_.count(in) == 0) {
	//	stops_.insert(in);
	//}
	std::unordered_set<Stop>::iterator It = stops_.find(in);
	if (It == stops_.end()) {
		throw std::logic_error("Non-existent Stop");
	}
	const transport_system::transport_catalogue::Stop* cpt = &(*It);
	transport_system::transport_catalogue::Stop* pt =
		const_cast<transport_system::transport_catalogue::Stop*>(cpt);
	return pt;
}

transport_system::transport_catalogue::Route transport_system::transport_catalogue::BasicParse(
	const std::string& raw_desc, const char delim) {

	size_t j = raw_desc.size();
	Route ret;
	ret.way_.reserve(j / 5);
	size_t bgn = 0, end = 0, sz = 0;

	for (size_t i = 0; i < j; ++i) {
		if (raw_desc[i] == delim || i == j - 1) {
			//++ret.unique_stops_;
			end = (i < j - 1) ? i : j + 1;
			sz = end - bgn - 1;
			ret.way_.push_back(LinkTextToStop(raw_desc.substr(bgn, sz)));
			bgn = end + 2;
		}
	}

	return ret;
}

transport_system::transport_catalogue::Route transport_system::transport_catalogue::ParseLinearRoute(
	const std::string& raw_desc) {

	Route ret = BasicParse(raw_desc, BusLineDelimeter);

	// adding tail
	ret.way_.reserve(ret.way_.size() * 2);
	ret.way_.insert(ret.way_.end(), ret.way_.rbegin() + 1, ret.way_.rend());
	ret.way_.shrink_to_fit();

	ret.unique_stops_ = CalcUniques(ret);
	CalcDistances(ret);
	ret.curvature_ = ret.exact_length_ / ret.geo_length_;
	return ret;
}

transport_system::transport_catalogue::Route transport_system::transport_catalogue::ParseCircularRoute(
	const std::string& raw_desc) {

	Route ret = BasicParse(raw_desc, BusCircleDelimeter);

	ret.way_.shrink_to_fit();
	ret.unique_stops_ = CalcUniques(ret);
	CalcDistances(ret);
	ret.curvature_ = ret.exact_length_ / ret.geo_length_;
	return ret;
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
