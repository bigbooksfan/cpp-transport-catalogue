#pragma once

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <optional>
#include <algorithm>

#include "geo.h"
#include "domain.h"

namespace tr_cat {
    namespace aggregations {
        using namespace std::string_literals;

        class TransportCatalogue {
        private:            // nested classes
            class DistanceHasher {
            public:
                size_t operator() (const std::pair<const Stop*, const Stop*> element) const {
                    const size_t shift = (size_t)log2(1 + sizeof(Stop));
                    const size_t result = (size_t)(element.first) >> shift;
                    return result + ((size_t)(element.second) >> shift) * 37;
                }
            };

            class DistanceCompare {
            public:
                bool operator() (const std::pair<const Stop*, const Stop*> lhs, const std::pair<const Stop*, const Stop*> rhs) const {
                    return lhs.first == rhs.first && rhs.second == lhs.second;
                }
            };

        public:             // fields
            std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher, DistanceCompare> distances_;
            std::deque<Stop> stops_data_;
            std::deque<Bus> buses_data_;
            std::unordered_map<std::string_view, Stop*> stops_container_;
            std::unordered_map<std::string_view, Bus*> buses_container_;
            std::vector<std::string_view> buses_;
            size_t vertex_count_ = 0;

        public:             // methods
            void AddStop(std::string_view name, geo::Coordinates coords);
            void AddBus(std::string_view name, std::vector<std::string_view>& stops, const bool is_ring);
            void AddDistance(const std::string_view lhs, const std::string_view rhs, double distance);
            std::optional<const Bus*>  GetBusInfo(std::string_view name) const;
            std::optional<const Stop*> GetStopInfo(std::string_view name) const;
            size_t GetVertexCount() const { return vertex_count_; }
            int GetDistance(const Stop* lhs, const Stop* rhs) const;
            auto begin() const { return buses_.begin(); }
            auto end() const { return buses_.end(); }
            size_t size() const { return buses_.size(); }
            size_t empty() const { return buses_.empty(); }

        private:            // methods
            int ComputeRouteDistance(std::string_view name) const;
            double ComputeGeoRouteDistance(std::string_view name) const;
            Stop* FindStop(std::string_view name) const;
            Bus* FindBus(std::string_view name)const;
        };
    }       // namespace aggregations
}           // namespace tr_cat