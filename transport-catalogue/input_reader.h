#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "transport_catalogue.h"

namespace transport_system {
	namespace InputReader {

		class input_reader {
		private:				// fields
			const size_t num_;
			transport_catalogue* tr_cat_;

			// HINT : vector < pair < stop From , raw string > >
			std::vector<std::pair<std::string, std::string>> DistSpecifiersBuffer_;

			const std::string StopKeyWord = "Stop ";
			const std::string BusKeyWord = "Bus ";
			const std::string NameDelimeter = ":";
			const std::string StopCoordDelimeter = ",";
			const std::string ExactDistanceDelimeter = ",";
			const std::string ExactDistanceStopsDelimeter = "m to ";
			const char BusLineDelimeter = '-';
			const char BusCircleDelimeter = '>';

			

		public:					// constructors
			input_reader() = delete;
			explicit input_reader(size_t num, transport_catalogue* tr_cat);

		public:					// methods

		private:				// methods
			void ParseRoutes();
			transport_system::transport_catalogue::Route 
				BasicParse(const std::string& raw_desc, const char delim);
			transport_system::transport_catalogue::Route
				ParseLinearRoute(const std::string& raw_desc);
			transport_system::transport_catalogue::Route
				ParseCircularRoute(const std::string& raw_desc);
			
			void ReadLine(std::istream& in = std::cin);
			void ReadStop(std::string& str);
			void ReadBus(std::string& str);
			static inline void TrimSpaces(std::string& s);
			void SpecifyDistance(std::string_view curr_stop, std::string_view str);


		};

	}			// namespace InputReader
}				// namespace transport_system