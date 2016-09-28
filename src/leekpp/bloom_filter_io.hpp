#pragma once

#include <ostream>

#include "bloom_filter.hpp"

namespace leekpp
{

/** \addtogroup Filter
 *  \{
**/

template <typename TChar, typename TCharTraits>
std::basic_ostream<TChar, TCharTraits>&
operator<<(std::basic_ostream<TChar, TCharTraits>& os, const bloom_filter_params& params)
{
    return os << "(m=" << params.bit_count << ", k=" << params.num_hashes << ')';
}

template <typename TChar, typename TCharTraits, typename T, typename TMixer, typename TStorage>
std::basic_ostream<TChar, TCharTraits>&
operator<<(std::basic_ostream<TChar, TCharTraits>& os, const basic_bloom_filter<T, TMixer, TStorage>& value)
{
    os << "{params=" << value.params();
    os << " data="   << value.data();
    os << '}';
    return os;
}

/** \} **/

}
