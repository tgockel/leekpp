#include "test.hpp"

#include <leekpp/bloom_filter.hpp>

namespace leekpp_tests
{

void run_test()
{
    auto filter = leekpp::bloom_filter<std::size_t>::create_ideal(0.05, 10000);
}

}
