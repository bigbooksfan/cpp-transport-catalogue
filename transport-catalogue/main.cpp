#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"
//#include "tests.h"

#include <cassert>
#include <fstream>
#include <iostream>

int main() {
    //tests::Test();    
    tr_cat::aggregations::TransportCatalogue catalog;
    tr_cat::interface::JsonReader reader(catalog);
    tr_cat::interface::Process(reader);
    //tr_cat::render::MapRenderer renderer (catalog, reader.GetRenderSettings(), std::cout);
    //renderer.Render();
}