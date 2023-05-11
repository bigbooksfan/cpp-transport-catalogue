#include "map_renderer.h"


namespace tr_cat {
    namespace render {
        using namespace aggregations;
        using namespace svg;

        using namespace std::string_literals;

        void MapRenderer::Render(std::ostream& out) {
            Document doc_to_render;
            auto coords = move(CollectCoordinates());
            SphereProjector project(coords.begin(), coords.end(), settings_.width, settings_.height, settings_.padding);

            std::set<std::string_view> stops_in_buses = move(RenderBuses(project, doc_to_render));
            RenderStops(project, doc_to_render, stops_in_buses);

            doc_to_render.Render(out);
        }

        std::unordered_set<geo::Coordinates, CoordinatesHasher> MapRenderer::CollectCoordinates() const {
            std::unordered_set<geo::Coordinates, CoordinatesHasher> result;
            for (std::string_view bus_name : tr_cat_) {
                std::optional<const Bus*> bus_finded = tr_cat_.GetBusInfo(bus_name);
                if (!bus_finded) {
                    throw std::logic_error("Catalog error. No Bus info"s + static_cast<std::string> (bus_name));
                }
                for (const Stop* stop : (*bus_finded)->stops) {
                    result.insert(stop->coordinates);
                }
            }
            return result;
        }

        std::pair<std::unique_ptr<Text>, std::unique_ptr<Text>> MapRenderer::AddBusLabels(SphereProjector& project, int index_color,
            const Stop* stop, std::string_view name) {
            Text bus_name_underlabel, bus_name_label;
            bus_name_underlabel.SetData(static_cast<std::string>(name)).SetPosition(project(stop->coordinates))
                .SetOffset(settings_.bus_label_offset).SetFontSize(static_cast<uint32_t>(settings_.bus_label_font_size))
                .SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetStrokeWidth(settings_.underlayer_width)
                .SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color)
                .SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND);

            bus_name_label.SetData(static_cast<std::string>(name)).SetPosition(project(stop->coordinates))
                .SetOffset(settings_.bus_label_offset).SetFontSize(static_cast<uint32_t>(settings_.bus_label_font_size))
                .SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetFillColor(settings_.color_palette[index_color]);

            return { std::make_unique<Text>(bus_name_underlabel), std::make_unique<Text>(bus_name_label) };
        }

        std::set<std::string_view> MapRenderer::RenderBuses(SphereProjector& project, Document& doc_to_render) {
            int index_color = 0;
            int color_counts = static_cast<int>(settings_.color_palette.size());
            std::vector<std::unique_ptr<Object>> bus_lines;
            std::vector<std::unique_ptr<Object>> bus_labels;
            bus_lines.reserve(tr_cat_.size());
            bus_labels.reserve(bus_lines.capacity() * 4);
            std::set<std::string_view> stops_in_buses;

            for (std::string_view bus_name : tr_cat_) {

                index_color %= color_counts;

                const Bus* bus = *tr_cat_.GetBusInfo(bus_name);
                if (bus->stops.empty()) {
                    continue;
                }

                std::unique_ptr<Polyline> line = std::make_unique<Polyline>(Polyline().SetFillColor("none"s)
                    .SetStrokeColor(settings_.color_palette[index_color]).SetStrokeWidth(settings_.line_width)
                    .SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND));

                std::unique_ptr<Text> bus_label_start, bus_underlabel_start,
                    bus_label_finish, bus_underlabel_finish;
                tie(bus_underlabel_start, bus_label_start) = AddBusLabels(project, index_color, bus->stops.front(), bus_name);
                if (!bus->is_ring && (bus->stops.front() != bus->stops[bus->stops.size() / 2])) {
                    tie(bus_underlabel_finish, bus_label_finish) = move(AddBusLabels(project,
                        index_color, bus->stops[bus->stops.size() / 2], bus_name));
                }

                for (const Stop* stop : bus->stops) {
                    line->AddPoint(project(stop->coordinates));
                    stops_in_buses.insert(stop->name);
                }

                bus_lines.push_back(std::move(line));
                bus_labels.push_back(std::move(bus_underlabel_start));
                bus_labels.push_back(std::move(bus_label_start));
                if (bus_label_finish && bus_underlabel_finish) {
                    bus_labels.push_back(std::move(bus_underlabel_finish));
                    bus_labels.push_back(std::move(bus_label_finish));
                }
                ++index_color;
            }

            for (auto& pointer : bus_lines) {
                doc_to_render.AddPtr(std::move(pointer));
            }
            for (auto& pointer : bus_labels) {
                doc_to_render.AddPtr(std::move(pointer));
            }
            return stops_in_buses;
        }

        void MapRenderer::RenderStops(SphereProjector& project, svg::Document& doc_to_render, std::set<std::string_view> stops_in_buses) {
            std::vector<std::unique_ptr<Circle>> stop_points;
            std::vector<std::unique_ptr<Text>> stop_labels;
            stop_points.reserve(stops_in_buses.size());
            stop_labels.reserve(stops_in_buses.size() * 2);

            for (std::string_view stop_name : stops_in_buses) {
                const Stop* stop = *(tr_cat_.GetStopInfo(stop_name));
                Point coords = project(stop->coordinates);

                std::unique_ptr<Circle> stop_point = std::make_unique<Circle>(Circle().SetCenter(coords)
                    .SetRadius(settings_.stop_radius)
                    .SetFillColor("white"s));

                std::unique_ptr<Text> stop_underlabel = std::make_unique<Text>(Text().SetPosition(coords)
                    .SetData(static_cast<std::string>(stop_name)).SetOffset(settings_.stop_label_offset)
                    .SetFontSize(static_cast<uint32_t>(settings_.stop_label_font_size)).SetFontFamily("Verdana"s)
                    .SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(StrokeLineJoin::ROUND));
                std::unique_ptr<Text> stop_label = std::make_unique<Text>(Text().SetPosition(coords)
                    .SetData(static_cast<std::string>(stop_name)).SetOffset(settings_.stop_label_offset)
                    .SetFontSize(static_cast<uint32_t>(settings_.stop_label_font_size)).SetFontFamily("Verdana"s)
                    .SetFillColor("black"s));

                stop_points.push_back(std::move(stop_point));
                stop_labels.push_back(std::move(stop_underlabel));
                stop_labels.push_back(std::move(stop_label));
            }

            for (auto& pointer : stop_points) {
                doc_to_render.AddPtr(std::move(pointer));
            }
            for (auto& pointer : stop_labels) {
                doc_to_render.AddPtr(std::move(pointer));
            }
        }
    }       // namespace render
}           // namespace tr_cat