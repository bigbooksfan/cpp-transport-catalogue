#include <algorithm>

#include "request_handler.h"

namespace tr_cat {
    namespace interface {
        void RequestInterface::AddStops() {
            std::for_each(stops_.begin(), stops_.end(), [&](StopInput& stop) {catalog_.AddStop(stop.name, stop.coordinates); });
        }

        void RequestInterface::AddDistances() {
            for (auto& [lhs, stops] : distances_) {
                for (auto& [rhs, value] : stops) {
                    catalog_.AddDistance(lhs, rhs, value);
                }
            }
        }

        void RequestInterface::AddBuses() {
            std::for_each(buses_.begin(), buses_.end(), [&](BusInput& bus) {catalog_.AddBus(bus.name, bus.stops, bus.is_ring); });
        }

        void RequestInterface::GetAnswers() {
            for (const Stat& stat : stats_) {
                if (stat.type == "Bus"s) {
                    std::optional<const Bus*> bus = catalog_.GetBusInfo(stat.name);
                    if (!bus) {
                        answers_.push_back(stat.id);
                        continue;
                    }
                    answers_.push_back(BusOutput{ stat.id, *bus });

                }
                else if (stat.type == "Stop"s) {
                    std::optional<const Stop*> stop = catalog_.GetStopInfo(stat.name);
                    if (!stop) {
                        answers_.push_back(stat.id);
                        continue;
                    }
                    answers_.push_back(StopOutput{ stat.id, *stop });

                }
                else if (stat.type == "Map"s) {
                    answers_.push_back(MapOutput(stat.id, catalog_));

                }
                else if (stat.type == "Route"s) {
                    std::optional<const Stop*> from = catalog_.GetStopInfo(stat.from);
                    std::optional<const Stop*> to = catalog_.GetStopInfo(stat.to);
                    if (!from || !to) {
                        answers_.push_back(stat.id);
                        continue;
                    }
                    answers_.push_back(RouteOutput({ stat.id, *from, *to }));
                }
                else {
                    throw std::invalid_argument("Invalid Stat"s);
                }
            }
        }
        void Process(interface::RequestInterface& reader) {
            reader.ReadDocument();
            reader.ParseDocument();
            reader.AddStops();
            reader.AddDistances();
            reader.AddBuses();
            reader.CreateGraph();
            reader.GetAnswers();
            reader.PrintAnswers();
        }
    }           // namespace interface
}               // namespace tr_cat