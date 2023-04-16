#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <strstream>

#include "transport_catalogue.h"
#include "log_duration.h"
#include "request_handler.h"
#include "json_reader.h"

namespace tr_cat {
    namespace tests {
        namespace detail {

            template <typename Type1, typename Type2>
            void AssertEqualImpl(const Type1& value1, const Type2 value2, const std::string& str_value1, const std::string& str_value2,
                const std::string& file, const std::string& func, unsigned line, const std::string& hint = ""s) {

                if (value1 != static_cast<Type1>(value2)) {
                    std::cerr << file << "("s << line << "): "s << func << ": ASSERT_EQUAL("s << str_value1 << ", "s << str_value2;
                    std::cerr << ") failed: "s << value1 << " != "s << value2 << "."s;
                    if (!hint.empty()) {
                        std::cerr << " Hint: "s << hint;
                    }
                    std::cerr << std::endl;
                    abort();
                }
            }

            void AssertImpl(const bool value, const std::string& expr, const std::string& file, const std::string& func,
                unsigned line, const std::string& hint = ""s);

            template <typename Func>
            void RunTestImpl(Func func, const std::string& func_name, const std::string& file_in,
                const std::string& file_out, const std::string& file_example) {
                func(file_in, file_out, file_example);
                std::cerr << func_name << " OK"s << std::endl;
            }

        } //detail

#define ASSERT_EQUAL(value1, value2) detail::AssertEqualImpl((value1), (value2), #value1, #value2, __FILE__, __FUNCTION__, __LINE__)
#define ASSERT_EQUAL_HINT(value1, value2, hint) detail::AssertEqualImpl((value1), (value2), #value1, #value2, __FILE__, __FUNCTION__, __LINE__, hint)

#define ASSERT(expr) detail::AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__)
#define ASSERT_HINT(expr, hint) detail::AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, hint)

#define RUN_TEST(func, file_in, file_out, file_example) detail::RunTestImpl((func), #func, (file_in), (file_out), (file_example))

        void TestOutput(const std::string& file_in, const std::string& file_out, const std::string& file_example);
        void TestRenderSpeed(const std::string& file_in, const std::string& file_out);
        void TestCatalogSpeed(const std::string& file_in, const std::string& file_out, const std::string&);
        void Test(const std::string file_in, const std::string file_out, const std::string file_example);
    }//tests
}//tr_cat
