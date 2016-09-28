#include "accuracy.hpp"
#include "test.hpp"

#include <leekpp/bloom_filter.hpp>

namespace leekpp_tests
{

void run_test()
{
    run_accuracy_test<leekpp::bloom_filter<std::size_t>>();
}

}
