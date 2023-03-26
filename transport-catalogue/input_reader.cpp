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
}

void transport_system::InputReader::input_reader::ReadLine() {
	std::string tmp;
	std::getline(std::cin, tmp);
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
