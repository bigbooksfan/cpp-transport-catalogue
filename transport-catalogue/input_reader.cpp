#include "input_reader.h"

#include <stdexcept>

transport_system::InputReader::input_reader::input_reader(size_t num, transport_catalogue* tr_cat)
	: num_(num), tr_cat_(tr_cat) {
	for (size_t i = 0; i < num; ++i) {
		ReadLine();
	}
	for (auto& i : DistSpecifiersBuffer_) {
		SpecifyDistance(i.first, i.second);
	}
	DistSpecifiersBuffer_.clear();
	ParseRoutes();
}

void transport_system::InputReader::input_reader::ReadLine(std::istream& in) {
	std::string tmp;
	std::getline(in, tmp);
	//TrimSpaces(tmp);		// do i need it?

	if (tmp.compare(0, StopKeyWord.size(), StopKeyWord) == 0) {
		ReadStop(tmp);
		return;
	}
	if (tmp.compare(0, BusKeyWord.size(), BusKeyWord) == 0) {
		ReadBus(tmp);
		return;
	}
	throw std::logic_error("No BUS or STOP specifier in ReadLine()");		
}

void transport_system::InputReader::input_reader::ReadBus(std::string& str) {
	size_t sz = BusKeyWord.size();
	size_t EndOfName = str.find(NameDelimeter, sz);

	std::string name = str.substr(sz, EndOfName - sz);
	std::string raw_description = str.substr(EndOfName + 2);

	tr_cat_->AddBus(name, raw_description);
}

void transport_system::InputReader::input_reader::ReadStop(std::string& str) {
	size_t sz = StopKeyWord.size();
	size_t EndOfName = str.find(NameDelimeter, sz);
	size_t CoordDelim = str.find(StopCoordDelimeter, EndOfName);

	size_t DistanceSpecifier = str.find(
		ExactDistanceDelimeter,
		CoordDelim + StopCoordDelimeter.size());
	size_t ExactDistanceDelim = 
		DistanceSpecifier == std::string::npos ?
		str.size() : DistanceSpecifier;

	std::string name = str.substr(sz, EndOfName - sz);
	double lat = std::stod(str.substr(EndOfName + 2, CoordDelim));
	double lon = std::stod(str.substr(CoordDelim + 1, ExactDistanceDelim - CoordDelim));

	tr_cat_->AddStop(name, lat, lon);

	// distances specification
	if (DistanceSpecifier != std::string::npos) {
		DistSpecifiersBuffer_.push_back({ std::string(name), str.substr(ExactDistanceDelim + ExactDistanceDelimeter.size() + 1)});
	}
}

void transport_system::InputReader::input_reader::SpecifyDistance(std::string_view curr_stop, std::string_view str) {
	std::string_view other;
	size_t next_pos = str.find(ExactDistanceDelimeter);
	size_t b = 0;
	size_t e = next_pos == std::string_view::npos ? str.size() : next_pos;
	for ( ; b < str.size(); ) {
		size_t dist_delim = str.find(ExactDistanceStopsDelimeter, b);
		double dist = std::stod(std::string(str.substr(b, dist_delim - b)));		// std::from_chars()
		size_t name_b = dist_delim + ExactDistanceStopsDelimeter.size();
		std::string_view other(&str[name_b], e - name_b);

		tr_cat_->ClarifyDistance(curr_stop, other, dist);

		b = e + 2;
		next_pos = str.find(ExactDistanceDelimeter, b);
		e = next_pos == std::string_view::npos ? str.size() : next_pos;
	}
}

void transport_system::InputReader::input_reader::TrimSpaces(std::string& s) {
	// left trim
	s.erase(
		s.begin(), std::find_if(s.begin(), s.end(),
			[](unsigned char ch) {
				return !std::isspace(ch);
			}
	));

	// right trim
	s.erase(
		std::find_if(s.rbegin(), s.rend(),
			[](unsigned char ch) {
				return !std::isspace(ch);
			}
		).base(), s.end()
	);
}

void transport_system::InputReader::input_reader::ParseRoutes() {

	const std::unordered_map<std::string, transport_system::transport_catalogue::Route>& 
		routes = tr_cat_->transport_system::transport_catalogue::GetRoutes();

	for (const transport_system::transport_catalogue::Bus& bus : tr_cat_->GetBuses()) {
		if (routes.count(bus.name_) != 0)
			throw std::logic_error("Bus name is not unique");

		bool circular = bus.raw_route_.find(BusCircleDelimeter) != std::string::npos;
		bool linear = bus.raw_route_.find(BusLineDelimeter) != std::string::npos;

		if (circular && linear) {
			throw std::logic_error("Raw route is both linear and circular");
		}
		if (!circular && !linear) {
			throw std::logic_error("Raw route is not linear or circular");
		}

		transport_system::transport_catalogue::Route tmp;
		if (circular) {
			tmp = ParseCircularRoute(bus.raw_route_);
			tr_cat_->SetDummy(tmp);
			tr_cat_->InsertRoute(bus.name_);
		}
		else {		// linear route
			tmp = ParseLinearRoute(bus.raw_route_);
			tr_cat_->SetDummy(tmp);
			tr_cat_->InsertRoute(bus.name_);
		}

		tr_cat_->PutBusesOnStop(bus);
	}
}

transport_system::transport_catalogue::Route
transport_system::InputReader::input_reader::BasicParse(
	const std::string& raw_desc, const char delim) {

	size_t j = raw_desc.size();
	transport_system::transport_catalogue::Route ret;
	ret.way_.reserve(j / 5);
	size_t bgn = 0, end = 0, sz = 0;

	for (size_t i = 0; i < j; ++i) {
		if (raw_desc[i] == delim || i == j - 1) {
			end = (i < j - 1) ? i : j + 1;
			sz = end - bgn - 1;
			ret.way_.push_back(
				tr_cat_->transport_system::transport_catalogue::LinkTextToStop(
					raw_desc.substr(bgn, sz)));
			bgn = end + 2;
		}
	}
	return ret;
}

transport_system::transport_catalogue::Route
transport_system::InputReader::input_reader::ParseLinearRoute(
	const std::string& raw_desc) {

	transport_system::transport_catalogue::Route ret 
		= BasicParse(raw_desc, BusLineDelimeter);

	// adding tail
	ret.way_.reserve(ret.way_.size() * 2);
	ret.way_.insert(ret.way_.end(), ret.way_.rbegin() + 1, ret.way_.rend());
	ret.way_.shrink_to_fit();

	ret.unique_stops_ = tr_cat_->transport_system::transport_catalogue::CalcUniques(ret);
	tr_cat_->transport_system::transport_catalogue::CalcDistances(ret);
	ret.curvature_ = ret.exact_length_ / ret.geo_length_;
	return ret;
}

transport_system::transport_catalogue::Route
transport_system::InputReader::input_reader::ParseCircularRoute(
	const std::string& raw_desc) {

	transport_system::transport_catalogue::Route ret 
		= BasicParse(raw_desc, BusCircleDelimeter);

	ret.way_.shrink_to_fit();
	ret.unique_stops_ = tr_cat_->transport_system::transport_catalogue::CalcUniques(ret);
	tr_cat_->transport_system::transport_catalogue::CalcDistances(ret);
	ret.curvature_ = ret.exact_length_ / ret.geo_length_;
	return ret;
}