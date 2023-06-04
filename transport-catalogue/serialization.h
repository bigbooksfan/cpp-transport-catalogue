#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace tr_cat {
    namespace serialize {
        class Serializator {
        private:        // fields
            aggregations::TransportCatalogue& catalog_;
            render::MapRenderer& renderer_;
            router::TransportRouter& transport_router_;
            std::filesystem::path path_to_serialize_;

        public:         // constructors
            Serializator(aggregations::TransportCatalogue& catalog, render::MapRenderer& renderer, router::TransportRouter& router)
                : catalog_(catalog), renderer_(renderer) , transport_router_(router) { }

        public:         // methods
            void SetPathToSerialize(const std::filesystem::path& path) { path_to_serialize_ = path; }
            size_t Serialize(bool with_graph = false) const;
            bool Deserialize(bool with_graph = false);
        };

    }   // namespace serialize
}       // namespace tr_cat