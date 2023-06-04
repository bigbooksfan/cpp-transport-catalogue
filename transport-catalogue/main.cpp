#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"

#include <cassert>
#include <fstream>
#include <iostream>

using namespace std;
using namespace tr_cat;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        aggregations::TransportCatalogue catalog;
        interface::JsonReader reader(catalog);
        reader.ReadDocument ();
        reader.ParseDocument ();
        reader.AddStops ();
        reader.AddDistances ();
        reader.AddBuses ();
        reader.CreateGraph();
        reader.Serialize (true);
    } else if (mode == "process_requests"sv) {
        aggregations::TransportCatalogue catalog;
        interface::JsonReader reader(catalog);
        reader.ReadDocument ();
        reader.ParseDocument ();
        reader.Deserialize (true);
        reader.GetAnswers ();
        reader.PrintAnswers ();
    } else {
        PrintUsage();
        return 1;
    }
}


/*
    tr_cat::aggregations::TransportCatalogue catalog;
    tr_cat::interface::JsonReader reader(catalog);
    tr_cat::interface::Process(reader);
    tests::Test("test_files/input/test1_input.json"s, "test_files/output/test1_output.json"s,
                "test_files/example/test1_example.json"s, "test_files/maps/test1_map.svg"s);

    tests::Test("test_files/input/test2_input.json"s, "test_files/output/test2_output.json"s, 
                "test_files/example/test2_example.json"s, "test_files/maps/test2_map.svg"s);

    tests::Test("test_files/input/test3_input.json"s, "test_files/output/test3_output.json"s, 
                "test_files/example/test3_example.json"s, "test_files/maps/test3_map.svg"s);

    tests::Test("test_files/input/test4_input.json"s, "test_files/output/test4_output.json"s, "test_files/example/test4_example.json"s);
    tests::TestRenderSpeed("test_files/input/test4_input.json", "test_files/maps/test4_map.svg"s);
    

    tests::TestCatalogSpeed("test_files/input/test5_input.json", "test_files/output/test5_output.json"s,
                             "test_files/maps/test5_map.svg"s);
*/
