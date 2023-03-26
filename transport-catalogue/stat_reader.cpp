#include <iostream>

#include "stat_reader.h"

transport_system::output::stat_reader::stat_reader(size_t num, transport_catalogue* tr_cat, bool is_test)
	: num_(num), tr_cat_(tr_cat), is_test_(is_test) { 
	CallExecution(num);
}

transport_system::output::stat_reader::stat_reader(std::string& num, transport_catalogue* tr_cat, bool is_test)
	: num_(std::stoi(num)), tr_cat_(tr_cat), is_test_(is_test) {
	CallExecution(std::stoi(num));
}

void transport_system::output::stat_reader::CallExecution(size_t num) {
	querries_.reserve(num_);
	if (is_test_) {
		tr_cat_->outrows_.reserve(num);
	}
	// read
	for (size_t i = 0; i < num; ++i) {
		ReadLine();
	}

	// print
	for (std::string& querry : querries_) {
		size_t sz = querry.size();

		if (querry[0] == BusKeyWord[0]) {
			size_t b = BusKeyWord.size();
			std::string_view tmp(&querry[b], sz - b);
			PrintBus(tmp);
		}
		if (querry[0] == StopKeyWord[0]) {
			size_t b = StopKeyWord.size();
			std::string_view tmp(&querry[b], sz - b);
			PrintStop(tmp);
		}
	}
}

void transport_system::output::stat_reader::ReadLine() {
	std::string tmp;
	std::getline(std::cin, tmp);

	if (tmp.compare(0, BusKeyWord.size(), BusKeyWord) != 0 &&
		tmp.compare(0, StopKeyWord.size(), StopKeyWord) != 0) {
		throw std::logic_error("Incorrect string in ReadLine()");
	}
	
	querries_.push_back(tmp);
}

void transport_system::output::stat_reader::PrintBus(std::string_view bus) {
	if (is_test_ == false) {
		PrintBusInner(std::cout, bus);
	}
	else {
		std::stringstream os;
		PrintBusInner(os, bus);
		tr_cat_->outrows_.push_back(os.str());
	}
}

void transport_system::output::stat_reader::PrintStop(std::string_view stop) {
	if (is_test_ == false) {
		PrintStopInner(std::cout, stop);
	}
	else {
		std::stringstream os;
		PrintStopInner(os, stop);
		tr_cat_->outrows_.push_back(os.str());
	}
}
