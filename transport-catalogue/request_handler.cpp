#include "request_handler.h"

#include <algorithm>

namespace tr_cat {
    namespace interface {
        void RequestInterface::AddStops() {
            std::for_each(stops_.begin(), stops_.end(), [&](StopInput& stop) {tr_cat_.AddStop(stop.name, stop.coordinates); });
        }

        void RequestInterface::AddDistances() {
            for (auto& [lhs, stops] : distances_) {
                for (auto& [rhs, value] : stops) {
                    tr_cat_.AddDistance(lhs, rhs, value);
                }
            }
        }

        void RequestInterface::AddBuses() {
            std::for_each(buses_.begin(), buses_.end(), [&](BusInput& bus) {tr_cat_.AddBus(bus.name, bus.stops, bus.is_ring); });
        }

        void RequestInterface::GetAnswers() {
            for (const Stat& stat : stats_) {
                if (stat.type == "Bus"s) {
                    std::optional<const Bus*> bus = tr_cat_.GetBusInfo(stat.name);
                    if (!bus) {
                        answers_.push_back(stat.id);
                        continue;
                    }
                    answers_.push_back(BusOutput{ stat.id, *bus });

                }
                else if (stat.type == "Stop"s) {
                    std::optional<const Stop*> stop = tr_cat_.GetStopInfo(stat.name);
                    if (!stop) {
                        answers_.push_back(stat.id);
                        continue;
                    }
                    answers_.push_back(StopOutput{ stat.id, *stop });

                }
                else if (stat.type == "Map"s) {
                    answers_.push_back(MapOutput(stat.id, tr_cat_));

                }
                else if (stat.type == "Route"s) {
                    std::optional<const Stop*> from = tr_cat_.GetStopInfo(stat.from);
                    std::optional<const Stop*> to = tr_cat_.GetStopInfo(stat.to);
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