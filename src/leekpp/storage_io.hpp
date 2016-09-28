#pragma once

#include <iomanip>
#include <ostream>

#include "storage.hpp"

namespace leekpp
{

namespace detail
{

template <typename TChar, typename TCharTraits, typename TStorage>
void stream_storage(std::basic_ostream<TChar, TCharTraits>& os, const TStorage& storage)
{
    using size_type = typename TStorage::size_type;

    os << std::hex << std::setfill('0');
    for (size_type block_idx = 0; block_idx < storage.block_count(); ++block_idx)
    {
        const auto& blk = storage[block_idx];
        os << std::setw(2 * sizeof blk) << +blk;
    }
    os << std::setfill(' ') << std::dec;
}

}

/** \addtogroup Storage
 *  \{
**/

template <typename TChar, typename TCharTraits, typename TBlock, typename TContiguousStore>
std::basic_ostream<TChar, TCharTraits>&
operator<<(std::basic_ostream<TChar, TCharTraits>& os, const basic_storage<TBlock, TContiguousStore>& value)
{
    detail::stream_storage(os, value);
    return os;
}

template <typename TChar, typename TCharTraits, typename TBlock>
std::basic_ostream<TChar, TCharTraits>&
operator<<(std::basic_ostream<TChar, TCharTraits>& os, const basic_thread_safe_storage<TBlock>& value)
{
    detail::stream_storage(os, value);
    return os;
}

/** \} **/

}
