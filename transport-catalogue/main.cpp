#include <cassert>
#include <fstream>
#include <iostream>

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"

//#define TEST_MODE

#ifdef TEST_MODE
#include "test.h"
#endif

int main() {
    using namespace std::string_literals;
    using namespace tr_cat;

#ifdef TEST_MODE
    tests::Test();
#endif

    tr_cat::aggregations::TransportCatalogue catalog;
    tr_cat::interface::JsonReader reader(catalog);
    tr_cat::interface::Process(reader);

    return 0;
}