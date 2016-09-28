#pragma once

#include <leekpp/bloom_filter_io.hpp>
#include <leekpp/storage_io.hpp>

#include <cstddef>
#include <iostream>
#include <iterator>
#include <random>
#include <set>
#include <stdexcept>

#include "test.hpp"

namespace leekpp_tests
{

class random_sequence_device
{
public:
    random_sequence_device& base()
    {
        return *this;
    }

    template <typename TForwardIterator>
    void generate(TForwardIterator first, TForwardIterator last)
    {
        using generated_value_type = typename std::iterator_traits<TForwardIterator>::value_type;
        std::uniform_int_distribution<generated_value_type> dist;
        for ( ; first != last; ++first)
            *first = dist(_rng);
    }

private:
    std::random_device _rng;
};

template <typename TBloomFilter>
void run_accuracy_test(double goal_fpr = 0.05, std::size_t element_count = 1000000, double tolerance_factor = 0.1)
{
    using value_type = typename TBloomFilter::value_type;
    using bloom_filter = TBloomFilter;

    std::mt19937 rng(random_sequence_device().base());
    std::uniform_int_distribution<value_type> dist;

    std::set<value_type> lossless;
    bloom_filter         filter = bloom_filter::create_ideal(goal_fpr, element_count);
    // Minor adjustment in FPR -- since bloom_filter_params::num_hashes is discrete, it will never perfectly hit the
    // original goal_fpr, so this aligns our new goal to be more accurate.
    goal_fpr = filter.params().expected_fpr(element_count);

    while (lossless.size() < element_count)
    {
        auto next = dist(rng);
        lossless.insert(next);
        filter.insert(next);
    }

    // Everything we inserted must appear in the filter
    for (value_type x : lossless)
    {
        TEST_ASSERT(1 == filter.count(x));
    }

    // FPR: Get a bunch of random numbers we didn't put into the filter and see how many are positive
    const std::size_t sample_count = element_count;
    std::size_t positives = 0;
    for (std::size_t x = 0; x < sample_count; ++x)
    {
        auto n = dist(rng);
        if (lossless.count(n))
        {
            --x;
            continue;
        }
        positives += filter.count(dist(rng));
    }
    auto fpr_tested = double(positives) / sample_count;
    std::cout << "positives=" << positives
              << " sample_count=" << sample_count
              << " FPR=" << (fpr_tested * 100) << '%'
              << std::endl;
    std::cout << filter << std::endl;
    TEST_ASSERT_WITHIN(goal_fpr, fpr_tested, goal_fpr * tolerance_factor);
}

}
