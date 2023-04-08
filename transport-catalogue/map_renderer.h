#include "svg.h"
#include "transport_catalogue.h"
#include "geo.h"

#include <ostream>
#include <algorithm>
#include <unordered_set>

namespace tr_cat {
    namespace render {

        inline bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }

        class SphereProjector {
        private:        // fields
            double padding_;
            double min_lon_ = 0;
            double max_lat_ = 0;
            double zoom_coeff_ = 0;

        public:
            // points_begin and points_end define the beginning and end of a range of geo::Coordinates elements
            template <typename PointInputIt>
            SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                            double max_width, double max_height, double padding)
                : padding_(padding)
            {
                // if the surface points of the sphere are not given, there is nothing to calculate
                if (points_begin == points_end) {
                    return;
                }

                // finding points with minimum and maximum longitude
                const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
                min_lon_ = left_it->lng;
                const double max_lon = right_it->lng;

                // finding points with minimum and maximum latitude
                const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
                const double min_lat = bottom_it->lat;
                max_lat_ = top_it->lat;

                // calculate the scaling factor along the x coordinate
                std::optional<double> width_zoom;
                if (!IsZero(max_lon - min_lon_)) {
                    width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
                }

                // calculate the scaling factor along the y coordinate
                std::optional<double> height_zoom;
                if (!IsZero(max_lat_ - min_lat)) {
                    height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
                }

                if (width_zoom && height_zoom) {
                    // The scaling coefficients for width and height are non-zero, 
                    // we take the minimum of them
                    zoom_coeff_ = std::min(*width_zoom, *height_zoom);
                } 
                else if (width_zoom) {
                    // width scaling factor is non-zero, use it
                    zoom_coeff_ = *width_zoom;
                } 
                else if (height_zoom) {
                    // height scaling factor is non-zero, use it
                    zoom_coeff_ = *height_zoom;
                }
            }

            // projects latitude and longitude into coordinates inside an SVG image
            svg::Point operator()(tr_cat::geo::Coordinates coords) const {
                return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
                };
            }
        };

        struct RenderSettings {
            double width = 0;
            double height = 0;

            double padding = 0;

            double line_width = 0;
            double stop_radius = 0;

            double bus_label_font_size = 0;
            svg::Point bus_label_offset {0, 0};

            double stop_label_font_size = 0;
            svg::Point stop_label_offset {0, 0};

            svg::Color underlayer_color;
            double underlayer_width = 0;
            std::vector<svg::Color> color_palette;
        };

        struct CoordinatesHasher {
            size_t operator() (const geo::Coordinates& coords) const {
                return std::hash<double>{}(coords.lat) + std::hash<double>{}(coords.lng) * 37;
            }
        };

        class MapRenderer {
        private:            // fields
            const aggregations::TransportCatalogue& tr_cat_;
            RenderSettings settings_;
            std::ostream& output_ = std::cout;

        public:             // constructors
            MapRenderer() = delete;
            MapRenderer(const aggregations::TransportCatalogue& catalog, const RenderSettings& settings)
            :tr_cat_(catalog), settings_(settings) {}
            MapRenderer(const aggregations::TransportCatalogue& catalog, const RenderSettings& settings, std::ostream& output)
            :tr_cat_(catalog), settings_(settings), output_(output) {}

            void Render();

        private:            // methods
            std::unordered_set<geo::Coordinates, CoordinatesHasher> CollectCoordinates () const;
            std::pair<std::unique_ptr<svg::Text>, std::unique_ptr<svg::Text>> 
                AddBusLabels(SphereProjector& project, int index_color, const Stop* stop, std::string_view name);
            std::set<std::string_view> RenderBuses(SphereProjector& project, svg::Document& doc_to_render);
            void RenderStops(SphereProjector& project, svg::Document& doc_to_render, std::set<std::string_view> stops_in_buses);
        };
    }           // namespace render
}               // namespace tr_cat