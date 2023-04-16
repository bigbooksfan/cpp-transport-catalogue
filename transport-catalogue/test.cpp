#include "test.h"


using namespace std::string_literals;

namespace tr_cat {
    namespace tests {
        namespace detail {

            void AssertImpl(const bool value, const std::string& expr, const std::string& file, const std::string& func,
                unsigned line, const std::string& hint) {
                if (!value) {
                    std::cerr << std::boolalpha;
                    std::cerr << file << "("s << line << "): "s << func << ": "s << "ASSERT("s << expr << ") failed."s;
                    if (!hint.empty()) {
                        std::cerr << " Hint: "s << hint << std::endl;;
                    }
                    abort();
                }
            }
        }// detail



        void TestOutput(const std::string& file_in, const std::string& file_out, const std::string& file_example) {
            std::ifstream inf{ file_in };
            std::ofstream outf{ file_out };
            aggregations::TransportCatalogue catalog;
            interface::JsonReader reader(catalog, inf, outf);
            reader.ReadDocument();
            reader.ParseDocument();
            reader.AddStops();
            reader.AddDistances();
            reader.AddBuses();
            reader.GetAnswers();
            reader.PrintAnswers();
            inf.close();
            outf.close();
            ASSERT_HINT(reader.TestingFilesOutput(file_out, file_example), "Output files not equal"s);
        }

        void TestRenderSpeed(const std::string& file_in, const std::string& file_out) {
            std::cerr << "Testing Render "s << file_in << std::endl << std::endl;
            LOG_DURATION("TOTAL"s);
            aggregations::TransportCatalogue catalog;
            std::ifstream inf{ file_in };
            std::ofstream outf{ file_out };
            interface::JsonReader reader(catalog, inf, outf);
            {
                LOG_DURATION("BASE FILLING"s);

                {
                    LOG_DURATION("    READING         "s);
                    reader.ReadDocument();
                }

                {
                    LOG_DURATION("    PARSING         "s);
                    reader.ParseDocument();
                }

                {
                    LOG_DURATION("    ADD STOPS       "s);
                    reader.AddStops();
                }

                {
                    LOG_DURATION("    ADD DISTANCES   ");
                    reader.AddDistances();
                }

                {
                    LOG_DURATION("    ADD BUSES       "s);
                    reader.AddBuses();
                }
            }
            render::MapRenderer render(catalog, reader.GetRenderSettings(), outf);
            {
                LOG_DURATION("RENDERING"s);
                {
                    LOG_DURATION("    DRAWING         "s);
                    render.Render();
                }
            }
            std::cerr << "-----------------------------------\n\n"s;
        }

        void TestCatalogSpeed(const std::string& file_in, const std::string& file_out, const std::string&) {
            LOG_DURATION("TOTAL"s);
            aggregations::TransportCatalogue catalog;
            std::ifstream inf{ file_in };
            std::ofstream outf{ file_out };
            interface::JsonReader reader(catalog, inf, outf);
            {
                LOG_DURATION("BASE FILLING"s);

                {
                    LOG_DURATION("    READING         "s);
                    reader.ReadDocument();
                }

                {
                    LOG_DURATION("    PARSING         "s);
                    reader.ParseDocument();
                }

                {
                    LOG_DURATION("    ADD STOPS       "s);
                    reader.AddStops();
                }

                {
                    LOG_DURATION("    ADD DISTANCES   ");
                    reader.AddDistances();
                }

                {
                    LOG_DURATION("    ADD BUSES       "s);
                    reader.AddBuses();
                }
            }
            {
                LOG_DURATION("ANSWERS  "s);
                {
                    LOG_DURATION("    GET ANSWERS     "s);
                    reader.GetAnswers();
                }
                {
                    LOG_DURATION("    PRINT           "s);
                    reader.PrintAnswers();
                }
            }
            std::cerr << "-----------------------------------\n\n"s;
        }

        void Test(const std::string file_in, const std::string file_out, std::string file_example = "empty"s) {
            std::cerr << std::endl << "========================================"s << std::endl;
            std::cerr << std::endl << "Testing "s << file_in << std::endl << std::endl;
            if (file_example != "empty"s) {
                RUN_TEST(TestOutput, file_in, file_out, file_example);
                std::cerr << std::endl << "========================================"s << std::endl;
            }
            std::cerr << std::endl << "Testing Speed "s << file_in << std::endl << std::endl;
            RUN_TEST(TestCatalogSpeed, file_in, file_out, file_example);
            std::cerr << std::endl << "========================================"s << std::endl;
            std::cerr << std::endl;
        }
    }       // namespace tests
}           // namespace tr_cat




