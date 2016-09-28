#pragma once

#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace leekpp_tests
{

void run_test();

template <typename... TArgs>
std::string format(const char* format, TArgs&&... args)
{
    char buffer[512];
    std::snprintf(buffer, sizeof buffer, format, std::forward<TArgs>(args)...);
    return buffer;
}

class assertion_error :
        public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

#define TEST_ASSERT(expr_)                                                                                             \
    while (!(expr_))                                                                                                   \
        throw leekpp_tests::assertion_error                                                                            \
        (                                                                                                              \
            leekpp_tests::format("Assertion failed in %s:%d -- %s", __FILE__, __LINE__, #expr_)                        \
        )                                                                                                              \

#define TEST_ASSERT_WITHIN(expected_, actual_, abs_error_)                                                             \
    while (std::abs((expected_) - (actual_)) > (abs_error_))                                                           \
        throw leekpp_tests::assertion_error                                                                            \
        (                                                                                                              \
            leekpp_tests::format("Assertion failed in %s:%d -- expected=%f actual=%f error=%f allowed_error=%f",       \
                                 __FILE__,                                                                             \
                                 __LINE__,                                                                             \
                                 double(expected_),                                                                    \
                                 double(actual_),                                                                      \
                                 std::abs(double(expected_) - double(actual_)),                                        \
                                 double(abs_error_)                                                                    \
                                )                                                                                      \
        )                                                                                                              \

}

int main(int, char**)
{
    try
    {
        leekpp_tests::run_test();
        return 0;
    }
    catch (const leekpp_tests::assertion_error& err)
    {
        std::cout << err.what() << std::endl;
        return -1;
    }
}
