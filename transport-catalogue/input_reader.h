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

		public:					// constructors
			input_reader() = delete;
			explicit input_reader(size_t num, transport_catalogue* tr_cat);

		public:					// methods

		private:				// methods
			void ReadLine();
			void ReadStop(std::string& str);
			void ReadBus(std::string& str);
			static inline void TrimSpaces(std::string& s);
			void SpecifyDistance(std::string_view curr_stop, std::string_view str);
		};

	}			// namespace InputReader
}				// namespace transport_system