#pragma once

#include <cmath>
inline const double EPSILON = 1e-6;

namespace tr_cat {
    namespace geo {
        struct Coordinates {
            double lat;
            double lng;
            bool operator==(const Coordinates& other) const {
                return std::abs(lat - other.lat) < EPSILON && std::abs(lng - other.lng) < EPSILON;
            }
            bool operator!=(const Coordinates& other) const {
                return !(*this == other);
            }
        };

        inline double ComputeDistance(Coordinates from, Coordinates to) {
            if (from == to) {
                return 0;
            }
            static const double dr = 3.1415926535 / 180.;
            static const double earth_rad = 6371000.0;

            return std::acos(std::sin(from.lat * dr) * std::sin(to.lat * dr)
                + std::cos(from.lat * dr) * std::cos(to.lat * dr) * std::cos(std::abs(from.lng - to.lng) * dr)) * earth_rad;
        }
    }       // namespace geo
}           // namespace tr_cat