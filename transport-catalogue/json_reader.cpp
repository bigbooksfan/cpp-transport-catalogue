#include "json_reader.h"

namespace tr_cat {
    namespace interface {
        using namespace std;

        void JsonReader::ReadDocument() {
            document_ = json::Load(input_);
        }

        void JsonReader::ParseDocument() {
            if (document_.GetRoot().IsNull()) {
                return;
            }
            auto& it = document_.GetRoot().AsMap();
            ParseBase(it.at("base_requests"s));
            if (it.count("stat_requests"s) && (it.at("stat_requests"s).IsArray())) {
                ParseStats(it.at("stat_requests"s));
            }
            if (it.count("render_settings"s)) {
                ParseRenderSettings(it.at("render_settings"s));
            }
        }

        void JsonReader::ParseBase(json::Node& base_node) {
            auto& base = base_node.AsArray();
            for (auto& element_node : base) {
                auto& element = element_node.AsMap();
                if (element.at("type"s).AsString() == "Stop"s) {

                    stops_.push_back({});

                    stops_.back().name = element.at("name"s).AsString();
                    stops_.back().coordinates.lat = element.at("latitude"s).AsDouble();
                    stops_.back().coordinates.lng = element.at("longitude"s).AsDouble();

                    if (element.count("road_distances"s)) {
                        auto& map_distances = element.at("road_distances"s).AsMap();
                        for (auto& [name, value] : map_distances) {
                            distances_[stops_.back().name].push_back({ name, value.AsInt() });
                        }
                    }

                }
                else if (element.at("type"s).AsString() == "Bus"s) {

                    buses_.push_back({});

                    buses_.back().name = element.at("name"s).AsString();
                    buses_.back().is_ring = element.at("is_roundtrip"s).AsBool();

                    auto& it = element.at("stops"s).AsArray();
                    buses_.back().stops.reserve(it.size());
                    for (json::Node& elem : it) {
                        buses_.back().stops.push_back(elem.AsString());
                    }

                }
                else {
                    throw invalid_argument("Unknown type"s);
                }
            }
        }

        void JsonReader::ParseStats(json::Node& stats_node) {
            auto& stats = stats_node.AsArray();
            stats_.reserve(stats.size());

            for (auto& element_node : stats) {
                auto& element = element_node.AsMap();
                if (element.count("name"s)) {
                    stats_.push_back({ element.at("id"s).AsInt(), element.at("type"s).AsString(), element.at("name"s).AsString() });
                }
                else {
                    stats_.push_back({ element.at("id"s).AsInt(), element.at("type").AsString(), {} });
                }
            }
        }

        void JsonReader::ParseRenderSettings(json::Node& settings_node) {

            auto& settings = settings_node.AsMap();

            if (settings.count("width"s)) {
                render_settings_.width = settings.at("width"s).AsDouble();
            }
            if (settings.count("height"s)) {
                render_settings_.height = settings.at("height"s).AsDouble();
            }
            if (settings.count("padding"s)) {
                render_settings_.padding = settings.at("padding"s).AsDouble();
            }
            if (settings.count("line_width"s)) {
                render_settings_.line_width = settings.at("line_width"s).AsDouble();
            }
            if (settings.count("stop_radius"s)) {
                render_settings_.stop_radius = settings.at("stop_radius"s).AsDouble();
            }
            if (settings.count("bus_label_font_size"s)) {
                render_settings_.bus_label_font_size = settings.at("bus_label_font_size"s).AsDouble();
            }
            if (settings.count("bus_label_offset"s)) {
                auto it = settings.at("bus_label_offset"s).AsArray();
                render_settings_.bus_label_offset = { it[0].AsDouble(), it[1].AsDouble() };
            }
            if (settings.count("stop_label_font_size"s)) {
                render_settings_.stop_label_font_size = settings.at("stop_label_font_size"s).AsDouble();
            }
            if (settings.count("stop_label_offset"s)) {
                auto it = settings.at("stop_label_offset"s).AsArray();
                render_settings_.stop_label_offset = { it[0].AsDouble(), it[1].AsDouble() };
            }

            auto get_color = [&](json::Node& key, svg::Color* field) {
                if (key.IsString()) {
                    *field = key.AsString();
                }
                else if (key.AsArray().size() == 3) {
                    *field = svg::Rgb({ key.AsArray()[0].AsInt(), key.AsArray()[1].AsInt(), key.AsArray()[2].AsInt() });
                }
                else if (key.AsArray().size() == 4) {
                    *field = svg::Rgba({ key.AsArray()[0].AsInt(), key.AsArray()[1].AsInt(), key.AsArray()[2].AsInt(),
                                                                                            key.AsArray()[3].AsDouble() });
                }
            };

            if (settings.count("underlayer_color"s)) {
                get_color(settings.at("underlayer_color"s), &render_settings_.underlayer_color);
            }
            if (settings.count("underlayer_width"s)) {
                render_settings_.underlayer_width = settings.at("underlayer_width"s).AsDouble();
            }

            if (settings.count("color_palette"s)) {
                auto& array = settings.at("color_palette"s).AsArray();
                render_settings_.color_palette.reserve(array.size());
                for (auto& node : array) {
                    render_settings_.color_palette.push_back({});
                    get_color(node, &render_settings_.color_palette.back());
                }
            }
        }

        void JsonReader::PrepareToPrint() {
            json::Array to_document;
            to_document.reserve(answers_.size());
            for (auto& answer : answers_) {
                to_document.push_back(move(visit(CreateNode{ render_settings_ }, answer)));
            }
            document_answers_ = move(to_document);
        }

        void JsonReader::PrintAnswers() {
            PrepareToPrint();
            json::Print(document_answers_, output_);
        }

        json::Node JsonReader::CreateNode::operator() (int value) {
            map <string, json::Node> answer = { {"request_id"s, value},
                                                {"error_message"s, "not found"s} };
            return answer;
        }

        json::Node JsonReader::CreateNode::operator() (StopOutput& value) {
            json::Array buses;
            for (string_view bus : value.stop->buses) {
                buses.push_back(move(static_cast<string>(bus)));
            }

            map <string, json::Node> answer = { {"buses"s, buses},
                                               {"request_id"s, value.id} };
            return answer;
        }

        json::Node JsonReader::CreateNode::operator() (BusOutput& value) {
            map <string, json::Node> answer = { {"request_id"s, value.id},
                                               {"curvature"s, value.bus->curvature},
                                               {"route_length"s, static_cast<double>(value.bus->distance)},
                                               {"stop_count"s, static_cast<int>(value.bus->stops.size())},
                                               {"unique_stop_count"s, value.bus->unique_stops} };
            return answer;
        }

        json::Node JsonReader::CreateNode::operator() (MapOutput& value) {
            ostringstream output;
            render::MapRenderer renderer(*value.catalog, settings_, output);
            renderer.Render();

            map<string, json::Node> answer = { {"request_id"s, value.id},
                                              {"map"s, output.str()} };
            return answer;
        }

        const render::RenderSettings& JsonReader::GetRenderSettings() const {
            return render_settings_;
        }

        bool NodeCompare(json::Node lhs, json::Node rhs) {
            if (lhs.IsArray() && rhs.IsArray()) {
                for (size_t i = 0; i < lhs.AsArray().size(); ++i) {
                    if (!NodeCompare(lhs.AsArray()[i], rhs.AsArray()[i])) {
                        cerr << "Arrays elements not equal"s << endl;
                        return false;
                    }
                }
                return true;
            }
            if (lhs.IsMap() && rhs.IsMap()) {
                for (auto& [key, value] : lhs.AsMap()) {
                    if (!NodeCompare(value, rhs.AsMap().at(key))) {
                        cerr << "Maps elements not equal"s << endl;
                        cerr << key << endl;
                        return false;
                    }
                }
                return true;
            }
            if (lhs.IsString() && rhs.IsString()) {
                string lhs_s = lhs.AsString();
                string rhs_s = rhs.AsString();
                for (auto it = lhs_s.find(' '); it != string::npos; it = lhs_s.find(' ')) {
                    lhs_s.erase(it, 1);
                }
                for (auto it = lhs_s.find('\n'); it != string::npos; it = lhs_s.find('\n')) {
                    lhs_s.erase(it, 1);
                }
                for (auto it = rhs_s.find(' '); it != string::npos; it = rhs_s.find(' ')) {
                    rhs_s.erase(it, 1);
                }
                for (auto it = rhs_s.find('\n'); it != string::npos; it = rhs_s.find('\n')) {
                    rhs_s.erase(it, 1);
                }
                for (size_t i = 0; i < lhs_s.size(); ++i) {
                    if (lhs_s[i] != rhs_s[i]) {
                        cerr << lhs_s[i] << endl << rhs_s[i] << endl << i << endl;
                        return false;
                    }
                }
                return true;
            }
            return lhs == rhs;
        }


        bool JsonReader::TestingFilesOutput(std::string filename_lhs, std::string filename_rhs) {
            json::Document lhs, rhs;
            {
                std::ifstream inf{ filename_lhs };
                lhs = json::Load(inf);
            }
            {
                std::ifstream inf{ filename_rhs };
                rhs = json::Load(inf);
            }
            return NodeCompare(lhs.GetRoot(), rhs.GetRoot());
        }

    }       // namespace interface
}           // namespace tr_cat