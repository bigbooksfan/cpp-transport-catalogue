#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"
#include "tests.h"

#define TESTMODE

int main() {

#ifdef TESTMODE
	tests::AllTests();
#else
	std::string n;
	std::getline(std::cin, n);
	
	transport_system::transport_catalogue Catalogue(n);

	std::getline(std::cin, n);
	transport_system::output::stat_reader reader(n, &Catalogue);
#endif

	return 0;
}