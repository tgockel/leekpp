#pragma once

#include <atomic>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace leekpp
{

/** \addtogroup Storage
 *  \{
 *
 *  A \e storage element is responsible for storing the bit vector of a Bloom filter.
 *
 *  ### Requirements
 *
 *   - `S` is a \e storage type
 *   - `s` is an instance of `S`
 *   - `B` is the type of block (some integral)
 *   - `b` is a loaded block (of type `B`)
 *   - `m` is a bitmask (of type `B`)
 *   - `bc` is a count of bits
 *   - `bi` is a block index
 *
 *  | Expression          | Notes                                                                                      |
 *  |:-------------------:|:-------------------------------------------------------------------------------------------|
 *  | `S(bc)`             | Create a storage with at least `bc` bits.                                                  |
 *  | `S::block_type`     | Member type of `B`.                                                                        |
 *  | `s[bi]` -> `b`      | Load the block at `bi`.                                                                    |
 *  | `s.set_mask(bi, m)` | Add the provided mask `m` to the block at `bi` (using a form of `or`).                     |
 *  | `s.clear()`         | Reset the contents of this storage to 0.                                                   |
**/

/** Provides dynamically-allocated block storage.
 *
 *  \tparam TBlock The type of block to store. This must be an integral type.
 *  \tparam TBackingStorage The "real" type to store data in.
**/
template <typename TBlock          = std::size_t,
          typename TBackingStorage = std::vector<TBlock>
         >
class basic_storage
{
public:
    using block_type           = TBlock;
    using backing_storage_type = TBackingStorage;
    using size_type            = typename backing_storage_type::size_type;
    using allocator_type       = typename backing_storage_type::allocator_type;

    static_assert(std::is_integral<block_type>::value, "TBlock must be an integral type.");
    
public:
    static constexpr size_type block_count(size_type bit_count)
    {
        return (bit_count + sizeof(block_type)*8 - 1) / 8 / sizeof(block_type);
    }
    
    explicit basic_storage(size_type bit_count, const allocator_type& alloc = allocator_type()) :
            _storage(block_count(bit_count), block_type(0), alloc),
            _bit_count(bit_count)
    { }
    
    size_type bit_count() const
    {
        return _bit_count;
    }
    
    size_type block_count() const
    {
        return _storage.size();
    }

    const block_type& operator[](size_type idx) const
    {
        return _storage[idx];
    }
    
    void set_mask(size_type block_idx, const block_type& mask)
    {
        _storage.at(block_idx) |= mask;
    }
    
    void clear()
    {
        _storage.assign(_storage.size(), block_type(0));
    }
    
private:
    backing_storage_type _storage;
    size_type            _bit_count;
};

/** \see basic_storage **/
using storage = basic_storage<>;

/** Similar to \c basic_storage, but bit operations are performed in a thread-safe manner.
 *
 *  \tparam TBlock The type of block to store. This must be an integral type.
 *  \tparam TBackingStorage The "real" type to store data in.
**/
template <typename TBlock          = std::size_t,
          typename TBackingStorage = std::vector<std::atomic<TBlock>>
         >
class basic_thread_safe_storage
{
public:
    using block_type           = TBlock;
    using backing_storage_type = TBackingStorage;
    using size_type            = typename backing_storage_type::size_type;
    using allocator_type       = typename backing_storage_type::allocator_type;

    static_assert(std::is_integral<block_type>::value, "TBlock must be an integral type.");

public:
    static constexpr size_type block_count(size_type bit_count)
    {
        return basic_storage<block_type>::block_count(bit_count);
    }

    explicit basic_thread_safe_storage(size_type bit_count, const allocator_type& alloc = allocator_type()) :
            _storage(block_count(bit_count), block_type(0), alloc),
            _bit_count(bit_count)
    { }

    size_type bit_count() const
    {
        return _bit_count;
    }

    size_type block_count() const
    {
        return _storage.size();
    }

    block_type operator[](size_type idx) const
    {
        return _storage[idx].load(std::memory_order_relaxed);
    }

    void set_mask(size_type block_idx, const block_type& mask)
    {
        _storage.at(block_idx).fetch_or(mask, std::memory_order_relaxed);
    }

    void clear()
    {
        for (auto& block : _storage)
            block.store(0, std::memory_order_relaxed);
    }

private:
    backing_storage_type _storage;
    size_type            _bit_count;
};

/** Simple thread-safe storage. **/
using thread_safe_storage = basic_thread_safe_storage<>;

/** \} **/

}
