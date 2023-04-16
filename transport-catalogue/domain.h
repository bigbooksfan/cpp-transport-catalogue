#pragma once

#include <string>
#include <vector>    
#include <set>

#include "geo.h"

namespace tr_cat {
    struct Stop;
    struct Bus {
        std::string name;
        std::vector<Stop*> stops;
        int unique_stops = 0;
        int distance = 0;
        double curvature = 0.0;
        bool is_ring = false;
    };
    struct Stop {
        std::string name;
        geo::Coordinates coordinates = { 0, 0 };
        std::set<std::string_view> buses;
    };
}       // namespace tr_cat