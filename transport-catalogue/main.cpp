#include <cassert>
#include <fstream>
#include <iostream>

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"

//#define TEST_MODE

int main() {
#ifdef TEST_MODE
    std::ifstream in;
    in.open("input_simple.txt");

    tr_cat::aggregations::TransportCatalogue catalog;
    tr_cat::interface::JsonReader reader(catalog, in, std::cout);
    tr_cat::interface::Process(reader);

/*    using namespace std::string_literals;
    tests::Test("test_files/input/test1_input.json"s, "test_files/output/test1_output.json"s, "test_files/example/test1_example.json"s);
    tests::TestRenderSpeed("test_files/input/test1_input.json", "test_files/maps/test1_map.svg"s);

    tests::Test("test_files/input/test2_input.json"s, "test_files/output/test2_output.json"s, "test_files/example/test2_example.json"s);
    tests::TestRenderSpeed("test_files/input/test2_input.json", "test_files/maps/test2_map.svg"s);

    tests::Test("test_files/input/test3_input.json"s, "test_files/output/test3_output.json"s, "test_files/example/test3_example.json"s);
    tests::TestRenderSpeed("test_files/input/test3_input.json", "test_files/maps/test3_map.svg"s);

    tests::Test("test_files/input/test4_input.json"s, "test_files/output/test4_output.json"s, "test_files/example/test4_example.json"s);
    tests::TestRenderSpeed("test_files/input/test4_input.json", "test_files/maps/test4_map.svg"s); */   
#else
    tr_cat::aggregations::TransportCatalogue catalog;
    tr_cat::interface::JsonReader reader(catalog);
    tr_cat::interface::Process(reader);
#endif
}