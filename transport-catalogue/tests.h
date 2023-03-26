#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <numeric>
#include <fstream>

#include "input_reader.h"

namespace tests {

    using std::string;
    using std::cerr;

    using namespace std;            // need it here to avoid massive amount of ""s errors

    template <typename T, typename U>
    void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file, const string& func, unsigned line, const string& hint) {
        if (t != u) {
            cerr << boolalpha;
            cerr << file << "(" << line << "): " << func << ": ";
            cerr << "ASSERT_EQUAL(" << t_str << ", " << u_str << ") failed: ";
            cerr << *t << " != " << *u << ".";
            if (!hint.empty()) {
                cerr << " Hint: " << hint;
            }
            cerr << std::endl;
            abort();
        }
    }

    void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
        const string& hint) {
        if (!value) {
            cerr << file << "(" << line << "): " << func << ":\n";
            cerr << "ASSERT(" << expr_str << ") failed.\n";
            if (!hint.empty()) {
                cerr << "Hint: " << hint;
            }
            cerr << std::endl;
            abort();
        }
    }

    template <typename T>
    void RunTestImpl(const T& func, const string& func_name) {
        func();
        cerr << func_name << " OK\n";
    }

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

#define RUN_TEST(func) RunTestImpl((func), #func)

    void ReadStop() {
        {       // exceptions
            
        }
        {       // read Stop, create it and check coords
            std::vector<std::string> rows(9);
            rows[0] = "8\n";
            rows[1] = "Stop Tolstopaltsevo: 55.611087, 37.208290\n";
            rows[2] = "Stop Marushkino: 55.595884, 37.209755\n";
            rows[3] = "Stop Rasskazovka: 55.632761, 37.333324\n";
            rows[4] = "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700\n";
            rows[5] = "Stop Biryusinka: 55.581065, 37.648390\n";
            rows[6] = "Stop Universam: 55.587655, 37.645687\n";
            rows[7] = "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n";
            rows[8] = "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n";
            std::string acc = std::accumulate(
                rows.begin(), rows.end(),
                std::string("")
            );
            std::istringstream testdata(acc);
            std::cin.rdbuf(testdata.rdbuf());
            std::string placeholder;
            std::getline(std::cin, placeholder);

            transport_system::transport_catalogue a(placeholder);

            transport_system::geo::Coordinates getted = a.FindStop("Biryulyovo Passazhirskaya").coords_;
            transport_system::geo::Coordinates correct{ 55.580999 , 37.659164 };
            ASSERT_HINT(getted == correct, "Error in reading Stop or coordinates\n");
            getted = a.FindStop("Universam").coords_;
            correct = { 55.587655 , 37.645687 };
            ASSERT_HINT(getted == correct, "Error in reading Stop or coordinates\n");
        }
    }

    void ReadBuses() {
        {       // read Bus
            std::vector<std::string> rows(11);
            rows[0] = "10\n";
            rows[1] = "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n";
            rows[2] = "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n";
            rows[3] = "Stop Tolstopaltsevo: 55.611087, 37.208290\n";
            rows[4] = "Stop Marushkino: 55.595884, 37.209755\n";
            rows[5] = "Stop Rasskazovka: 55.632761, 37.333324\n";
            rows[6] = "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700\n";
            rows[7] = "Stop Biryusinka: 55.581065, 37.648390\n";
            rows[8] = "Stop Universam: 55.587655, 37.645687\n";
            rows[9] = "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n";
            rows[10] = "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n";
            std::string acc = std::accumulate(
                rows.begin(), rows.end(),
                std::string("")
            );
            std::istringstream testdata(acc);
            std::cin.rdbuf(testdata.rdbuf());
            std::string placeholder;
            std::getline(std::cin, placeholder);

            transport_system::transport_catalogue a(placeholder);

            std::string getted = a.FindBus("256").raw_route_;
            std::string correct = "Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye";
            ASSERT_HINT(getted.compare(correct) == 0, "Error in reading Bus");
            getted = a.FindBus("750").raw_route_;
            correct = "Tolstopaltsevo - Marushkino - Rasskazovka";
            ASSERT_HINT(getted.compare(correct) == 0, "Error in reading Bus");
        }
    }
    void ParseRoutes() {
        {           // linear route
            std::vector<std::string> rows(5);
            rows[0] = "4\n";
            rows[1] = "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n";
            rows[2] = "Stop Rasskazovka: 55.632761, 37.333324\n";
            rows[3] = "Stop Tolstopaltsevo: 55.611087, 37.208290\n";
            rows[4] = "Stop Marushkino: 55.595884, 37.209755\n";
            std::string acc = std::accumulate(
                rows.begin(), rows.end(),
                std::string("")
            );
            std::istringstream testdata(acc);
            std::cin.rdbuf(testdata.rdbuf());
            std::string placeholder;
            std::getline(std::cin, placeholder);

            transport_system::transport_catalogue a(placeholder);

            transport_system::transport_catalogue::Route getted = a.GetRoute("750");
            transport_system::transport_catalogue::Route correct;
            transport_system::transport_catalogue::Stop s1{ "Tolstopaltsevo", { 55.611087, 37.208290} };
            transport_system::transport_catalogue::Stop s2{ "Marushkino", { 55.595884, 37.209755} };
            transport_system::transport_catalogue::Stop s3{ "Rasskazovka", { 55.632761, 37.333324} };
            transport_system::transport_catalogue::Stop s4{ "Marushkino", { 55.611087, 37.208290} };
            transport_system::transport_catalogue::Stop s5{ "Tolstopaltsevo", { 55.595884, 37.209755} };
            correct.way_.reserve(5);
            correct.way_.push_back(&s1);
            correct.way_.push_back(&s2);
            correct.way_.push_back(&s3);
            correct.way_.push_back(&s4);
            correct.way_.push_back(&s5);
            correct.unique_stops_ = 3;
            correct.length_ = 20939.483046751142;

            bool b1 = getted.length_ == correct.length_;
            bool b2 = getted.unique_stops_ == correct.unique_stops_;
            bool b3 = getted.way_[0]->name_ == correct.way_[0]->name_;
            bool b4 = getted.way_[1]->name_ == correct.way_[1]->name_;
            bool b5 = getted.way_[2]->name_ == correct.way_[2]->name_;
            bool b6 = getted.way_[3]->name_ == correct.way_[3]->name_;
            bool b7 = getted.way_[4]->name_ == correct.way_[4]->name_;

            ASSERT_HINT(b1 && b2 && b3 && b4 && b5 && b6 && b7, 
                "Error in parse linear route");
        }
        {       // circular route
            std::vector<std::string> rows(7);
            rows[0] = "6\n";
            rows[1] = "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n";
            rows[2] = "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700\n";
            rows[3] = "Stop Biryusinka: 55.581065, 37.648390\n";
            rows[4] = "Stop Universam: 55.587655, 37.645687\n";
            rows[5] = "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n";
            rows[6] = "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n";
            std::string acc = std::accumulate(
                rows.begin(), rows.end(),
                std::string("")
            );
            std::istringstream testdata(acc);
            std::cin.rdbuf(testdata.rdbuf());
            std::string placeholder;
            std::getline(std::cin, placeholder);

            transport_system::transport_catalogue a(placeholder);

            transport_system::transport_catalogue::Route getted = a.GetRoute("256");
            transport_system::transport_catalogue::Route correct;
            transport_system::transport_catalogue::Stop s1{ "Biryulyovo Zapadnoye", { 55.574371, 37.651700} };
            transport_system::transport_catalogue::Stop s2{ "Biryusinka", { 55.581065, 37.651700} };
            transport_system::transport_catalogue::Stop s3{ "Universam", { 55.587655, 37.645687} };
            transport_system::transport_catalogue::Stop s4{ "Biryulyovo Tovarnaya", { 55.592028, 37.653656} };
            transport_system::transport_catalogue::Stop s5{ "Biryulyovo Passazhirskaya", { 55.580999, 37.659164} };
            correct.way_.reserve(6);
            correct.way_.push_back(&s1);
            correct.way_.push_back(&s2);
            correct.way_.push_back(&s3);
            correct.way_.push_back(&s4);
            correct.way_.push_back(&s5);
            correct.way_.push_back(&s1);
            correct.unique_stops_ = 5;
            correct.length_ = 4371.0172608446010;

            bool b1 = getted.length_ == correct.length_;
            bool b2 = getted.unique_stops_ == correct.unique_stops_;
            bool b3 = getted.way_[0]->name_ == correct.way_[0]->name_;
            bool b4 = getted.way_[1]->name_ == correct.way_[1]->name_;
            bool b5 = getted.way_[2]->name_ == correct.way_[2]->name_;
            bool b6 = getted.way_[3]->name_ == correct.way_[3]->name_;
            bool b7 = getted.way_[4]->name_ == correct.way_[4]->name_;
            bool b8 = getted.way_[5]->name_ == correct.way_[5]->name_;

            ASSERT_HINT(b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8, 
                "Error in parse circular route");
        }
    }

    void ExactDistances() {
        std::vector<std::string> rows(14);
        rows[0] = "13\n";
        rows[1] = "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n";
        rows[2] = "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino\n";
        rows[3] = "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n";
        rows[4] = "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka\n";
        rows[5] = "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino\n";
        rows[6] = "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n";
        rows[7] = "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n";
        rows[8] = "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n";
        rows[9] = "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n";
        rows[10] = "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n";
        rows[11] = "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n";
        rows[12] = "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n";
        rows[13] = "Stop Prazhskaya: 55.611678, 37.603831\n";
        std::string acc = std::accumulate(
            rows.begin(), rows.end(),
            std::string("")
        );
        std::istringstream testdata(acc);
        std::cin.rdbuf(testdata.rdbuf());
        std::string placeholder;
        std::getline(std::cin, placeholder);

        transport_system::transport_catalogue a(placeholder);

        rows.clear();
        rows.resize(7);
        rows[0] = "6\n";
        rows[1] = "Bus 256\n";
        rows[2] = "Bus 750\n";
        rows[3] = "Bus 751\n";
        rows[4] = "Stop Samara\n";
        rows[5] = "Stop Prazhskaya\n";
        rows[6] = "Stop Biryulyovo Zapadnoye\n";

        acc = std::accumulate(
            rows.begin(), rows.end(),
            std::string("")
        );
        rows.clear(); 
        std::istringstream testdata1(acc);
        std::cin.rdbuf(testdata1.rdbuf());
        std::getline(std::cin, placeholder);

        transport_system::output::stat_reader reader(placeholder, &a, true);
        rows.resize(6);
        rows[0] = "Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature\n";
        rows[1] = "Bus 750: 7 stops on route, 3 unique stops, 27400 route length, 1.30853 curvature\n";
        rows[2] = "Bus 751: not found\n";
        rows[3] = "Stop Samara: not found\n";
        rows[4] = "Stop Prazhskaya: no buses\n";
        rows[5] = "Stop Biryulyovo Zapadnoye: buses 256 828\n";
    
        for (int i = 0; i < 6; ++i) {
            ASSERT_HINT(a.outrows_[i] == rows[i], "Error in Exact Distance compute");
        }

    }

    void FileTest(std::string fname_prefix) {
        std::ifstream input("tests/" + fname_prefix + "_input.txt");

        // data from file
        std::string str;
        std::getline(input, str);
        size_t num = std::stoi(str);

        std::vector<std::string> rows(num + 1);
        rows[0] = str + '\n';
        for (size_t i = 0; i < num; ++i) {
            std::getline(input, str);
            rows[i + 1] = str + '\n';
        }

        std::string acc = std::accumulate(
            rows.begin(), rows.end(),
            std::string("")
        );
        std::istringstream testdata(acc);
        std::cin.rdbuf(testdata.rdbuf());
        std::string placeholder;
        std::getline(std::cin, placeholder);

        transport_system::transport_catalogue a(placeholder);

        // querries from file
        std::getline(input, str);
        num = std::stoi(str);

        rows.clear();
        rows.resize(num + 1);
        rows[0] = str + '\n';
        for (size_t i = 0; i < num; ++i) {
            std::getline(input, str);
            rows[i + 1] = str + '\n';
        }
        input.close();

        acc = std::accumulate(
            rows.begin(), rows.end(),
            std::string("")
        );
        rows.clear();
        std::istringstream testdata1(acc);
        std::cin.rdbuf(testdata1.rdbuf());
        std::getline(std::cin, placeholder);

        transport_system::output::stat_reader reader(placeholder, &a, true);

        // results from file
        std::ifstream input1("tests/" + fname_prefix + "_output.txt");

        // data from file
        rows.resize(num);
        for (size_t i = 0; i < num; ++i) {
            std::getline(input1, str);
            rows[i] = str + '\n';
        }

        for (size_t i = 0; i < num; ++i) {
            ASSERT_HINT(a.outrows_[i] == rows[i], "Error in tests from file");
        }
        std::cerr << "file tests/" + fname_prefix + "_output.txt DONE\n";
    }

    void FileTests2() {
        std::ifstream input("tests/tsC_case1_input.txt");

        // data from file
        std::string str;
        std::getline(input, str);
        size_t num = std::stoi(str);

        std::vector<std::string> rows(num + 1);
        rows[0] = str + '\n';
        for (size_t i = 0; i < num; ++i) {
            std::getline(input, str);
            rows[i + 1] = str + '\n';
        }

        std::string acc = std::accumulate(
            rows.begin(), rows.end(),
            std::string("")
        );
        std::istringstream testdata(acc);
        std::cin.rdbuf(testdata.rdbuf());
        std::string placeholder;
        std::getline(std::cin, placeholder);

        transport_system::transport_catalogue a(placeholder);

        // querries from file
        std::getline(input, str);
        num = std::stoi(str);

        rows.clear();
        rows.resize(num + 1);
        rows[0] = str + '\n';
        for (size_t i = 0; i < num; ++i) {
            std::getline(input, str);
            rows[i + 1] = str + '\n';
        }
        input.close();

        acc = std::accumulate(
            rows.begin(), rows.end(),
            std::string("")
        );
        rows.clear();
        std::istringstream testdata1(acc);
        std::cin.rdbuf(testdata1.rdbuf());
        std::getline(std::cin, placeholder);

        transport_system::output::stat_reader reader(placeholder, &a, true);

        // results from file
        std::ifstream input1("tests/tsC_case1_output1.txt");

        // data from file
        rows.resize(num);
        for (size_t i = 0; i < num; ++i) {
            std::getline(input1, str);
            rows[i] = str + '\n';
        }

        for (size_t i = 0; i < num; ++i) {
            ASSERT_HINT(a.outrows_[i] == rows[i], "Error in tests from file");
        }
        std::cerr << "file tests/tsC_case1_output1.txt DONE\n";
    }

    void FileTests() {
        FileTest("tsA_case1");
        FileTest("tsA_case2");   
        FileTest("tsB_case1");
        FileTest("tsB_case2");        
    }

    void AllTests() {
        RUN_TEST(ReadStop);
        RUN_TEST(ReadBuses);
        RUN_TEST(ParseRoutes);
        RUN_TEST(ExactDistances);
        RUN_TEST(FileTests);
        RUN_TEST(FileTests2);
    }

}		// namespace tests