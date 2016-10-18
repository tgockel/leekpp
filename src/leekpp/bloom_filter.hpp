#pragma once

#include <cmath>
#include <type_traits>

#include "assert.hpp"
#include "mixer.hpp"
#include "storage.hpp"

namespace leekpp
{

/** \addtogroup Filter
 *  \{
**/

/** Parameters for a Bloom filter.
 *
 * For math-related functions, these letters are used for the variable names:
 *  - \f$k\f$: hash count
 *  - \f$m\f$: bit vector length
 *  - \f$n\f$: number of elements
 *  - \f$p\f$: false positive rate
**/
struct bloom_filter_params
{
    /** \f$m\f$ -- the number of bits the \c bloom_filter should have. **/
    std::size_t bit_count;

    /** \f$k\f$ -- the number of bits to set. **/
    std::size_t num_hashes;

    bloom_filter_params() = default;

    constexpr bloom_filter_params(std::size_t bit_count, std::size_t num_hashes) noexcept :
            bit_count(bit_count),
            num_hashes(num_hashes)
    { }

    /** Calculate the expected false positive rate if the number of \a elements were put into a Bloom filter with these
     *  parameters.
     *
     *  \f[ p = \left[1 - \left(1 - \frac{1}{m}\right)^{kn}\right]^k \f]
    **/
    double expected_fpr(std::size_t elements) const
    {
        auto inner = std::pow(1.0 - 1.0 / bit_count, num_hashes * elements);
        return std::pow(1.0 - inner, num_hashes);
    }

    /** Estimate the number of elements in a filter.
     *
     *  \f[ n' = -\frac{m}{k} \ln\left(1 - \frac{X}{m}\right) \f]
     *
     *  \param set_bits The number of bits in the filter which are set to 1.
    **/
    std::size_t estimated_count(std::size_t set_bits) const
    {
        return -double(bit_count) / num_hashes * std::log(1.0 - double(set_bits) / bit_count);
    }

    /** Create an "ideally sized" Bloom filter for the desired parameters. "Ideal" here means the output \c bit_count
     *  (\f$m\f$) yields a Bloom filter with an FPR close to \c desired_fpr (\f$p\f$) for the \c expected_elements
     *  (\f$n\f$) with a \c num_hashes (\f$k\f$) which minimizes the FPR.
     *
     *  \f[ m = -\frac{n \ln p}{(\ln 2)^2} \f]
     *  \f[ k = \frac{m}{n} \ln 2 \f]
     *
     *  \param desired_fpr The goal false positive rate of the Bloom filter if \a expected_elements are inserted.
     *  \param expected_elements The number of elements you wish to insert into the Bloom filter.
    **/
    static bloom_filter_params create_ideal(double desired_fpr, std::size_t expected_elements)
    {
        LEEK_ASSERT(0.0 < desired_fpr && desired_fpr < 1.0,
                    invalid_argument,
                    ("desired_fpr=%.4d is not in range (0.0..1.0)", desired_fpr)
                   );
        LEEK_ASSERT(expected_elements != 0,
                    invalid_argument,
                    ("Cannot create a Bloom filter parameters with expected_elements=0")
                   );

        auto num = -double(expected_elements) * std::log(desired_fpr);
        auto den = std::pow(std::log(2.0), 2.0);
        auto bit_count = std::size_t(num / den);

        auto num_hashes = std::round(std::log(2.0) * double(bit_count) / expected_elements);

        return bloom_filter_params(bit_count, num_hashes);
    }
};

/** A probabilistic data structure which is extremely space efficient, but has the disadvantage of having false
 *  positives.
 *
 *  \tparam T The type of value this Bloom filter is meant to store.
 *  \tparam TMixer A mixing function -- must meet the requirements of a Mixer (see \c basic_mixer).
 *  \tparam TStorage A block-based container to store the elements -- must meet the requirements of a Storage (see
 *   \c basic_storage).
 *
 *  \see https://en.wikipedia.org/wiki/Bloom_filter
**/
template <typename T,
          typename TMixer   = basic_mixer<T>,
          typename TStorage = basic_storage<>
         >
class basic_bloom_filter
{
public:
    using value_type   = T;
    using mixer_type   = TMixer;
    using storage_type = TStorage;
    using block_type   = typename storage_type::block_type;
    using size_type    = std::size_t;
    
public:
    /** Create an instance using \a params. **/
    explicit basic_bloom_filter(const bloom_filter_params& params) :
            _data(params.bit_count),
            _params(params)
    {
        clear();
    }

    /** Creates a \c basic_bloom_filter from the parameters created with \c bloom_filter_params::create_ideal, rounding
     *  up the bit count to \c mixer_type::block_bits.
    **/
    static basic_bloom_filter create_ideal(double desired_fpr, std::size_t expected_elements)
    {
        auto params = bloom_filter_params::create_ideal(desired_fpr, expected_elements);
        if (mixer_type::block_bits > 0 && params.bit_count % mixer_type::block_bits != 0)
            params.bit_count += mixer_type::block_bits - (params.bit_count % mixer_type::block_bits);
        return basic_bloom_filter(params);
    }

    /** Create an instance using an already-created \a storage, which is not cleared. There is a degree of trust that
     *  the \a params and \a storage are compatible.
     *
     *  \throws std::invalid_argument if \a params specifies a \c bit_count that cannot fit in \a storage. If this
     *   exception is actually thrown depends on the \c LEEK_ASSERT settings.
    **/
    explicit basic_bloom_filter(const bloom_filter_params& params, storage_type storage) :
            _data(std::move(storage)),
            _params(params)
    {
        LEEK_ASSERT(params.bit_count <= storage.bit_count(),
                    invalid_argument,
                    ("Parameters cannot fit into storage -- params.bit_count=%zd storage.bit_count=%zd",
                     params.bit_count,
                     storage.bit_count()
                    )
                   );
    }

    /** Get the parameters used for this Bloom filter. **/
    const bloom_filter_params& params() const
    {
        return _params;
    }

    /** Get the contents of this Bloom filter. **/
    const storage_type& data() const
    {
        return _data;
    }

    /** Test for the likely presence of \a x in this filter instance. Keep in mind that a Bloom filter might erroneously
     *  test positively for presence when \a x was never inserted due to false positives. However, this function will
     *  \e never return \c 0 for a value that was actually inserted (no false negatives).
     *
     *  \returns \c 0 if the value is not present in this filter; \c 1 if it looks like the value is present.
    **/
    size_type count(const value_type& x) const
    {
        mixer_type mixer(x, _data.bit_count());
        return count_impl(mixer);
    }

    /** Insert the value \a x into this filter. Inserting the same value multiple times has no effect. **/
    void insert(const value_type& x)
    {
        mixer_type mixer(x, _data.bit_count());
        insert_impl(mixer);
    }

    /** Reset the contents of this filter to nothing. **/
    void clear()
    {
        _data.clear();
    }

private:
    template <typename UMixer>
    typename std::enable_if<UMixer::block_bits == 0, size_type>::type
    count_impl(UMixer& mixer) const
    {
        for (std::size_t count = 0; count < _params.num_hashes; ++count)
        {
            auto bit_idx = mixer();
            auto block_idx = bit_idx / (8 * sizeof(block_type));
            auto block_bit = bit_idx % (8 * sizeof(block_type));
            if (!(_data[block_idx] & (block_type(1) << block_bit)))
                return 0;
        }
        return 1;
    }

    template <typename UMixer>
    typename std::enable_if<UMixer::block_bits == 0, void>::type
    insert_impl(UMixer& mixer)
    {
        for (std::size_t count = 0; count < _params.num_hashes; ++count)
        {
            auto bit_idx = mixer();
            auto block_idx = bit_idx / (8 * sizeof(block_type));
            auto block_bit = bit_idx % (8 * sizeof(block_type));
            _data.set_mask(block_idx, block_type(1) << block_bit);
        }
    }

    template <typename UMixer>
    typename std::enable_if<(UMixer::block_bits > 0), size_type>::type
    count_impl(UMixer& mixer) const
    {
        static_assert(UMixer::block_bits / (sizeof(block_type) * 8),
                      "storage_type::block_type not compatible with mixer_type bit alignment"
                     );
        constexpr auto storage_blocks_in_mixer_block = UMixer::block_bits / (sizeof(block_type) * 8);
        block_type blocks[storage_blocks_in_mixer_block];
        bool block_loaded[storage_blocks_in_mixer_block];
        for (std::size_t idx = 0; idx < storage_blocks_in_mixer_block; ++idx)
            block_loaded[idx] = false;

        std::size_t base_bit_offset = mixer.base_offset();
        std::size_t base_block_offset = base_bit_offset / (sizeof(block_type) * 8);
        for (std::size_t count = 0; count < _params.num_hashes; ++count)
        {
            auto inner_bit_idx = mixer() - base_bit_offset;
            auto inner_block_idx = inner_bit_idx / (8 * sizeof(block_type));
            auto inner_block_bit = inner_bit_idx % (8 * sizeof(block_type));
            if (!block_loaded[inner_block_idx])
            {
                blocks[inner_block_idx] = _data[base_block_offset + inner_block_idx];
                block_loaded[inner_block_idx] = true;
            }

            if (!(blocks[inner_block_idx] & (block_type(1) << inner_block_bit)))
                return 0;
        }
        return 1;
    }

    template <typename UMixer>
    typename std::enable_if<(UMixer::block_bits > 0), void>::type
    insert_impl(UMixer& mixer)
    {
        static_assert(UMixer::block_bits / (sizeof(block_type) * 8),
                      "storage_type::block_type not compatible with mixer_type bit alignment"
                     );
        constexpr auto storage_blocks_in_mixer_block = UMixer::block_bits / (sizeof(block_type) * 8);
        block_type blocks[storage_blocks_in_mixer_block];
        for (std::size_t idx = 0; idx < storage_blocks_in_mixer_block; ++idx)
            blocks[idx] = block_type(0);

        // This always gets subtracted in the for loop, which seems like extra work. When looking at the assembly, it
        // seems to always get optimized to just `(_rng() % KAlignBits)` with the basic_cache_aligned_mixer.
        std::size_t base_bit_offset = mixer.base_offset();
        std::size_t base_block_offset = base_bit_offset / (sizeof(block_type) * 8);

        for (std::size_t count = 0; count < _params.num_hashes; ++count)
        {
            auto inner_bit_idx = mixer() - base_bit_offset;
            auto inner_block_idx = inner_bit_idx / (8 * sizeof(block_type));
            auto inner_block_bit = inner_bit_idx % (8 * sizeof(block_type));
            blocks[inner_block_idx] |= block_type(1) << inner_block_bit;
        }

        for (std::size_t idx = 0; idx < storage_blocks_in_mixer_block; ++idx)
            if (blocks[idx] != block_type(0))
                _data.set_mask(base_block_offset + idx, blocks[idx]);
    }

private:
    storage_type        _data;
    bloom_filter_params _params;
};

template <typename T>
using bloom_filter = basic_bloom_filter<T>;

template <typename T>
using cache_aligned_bloom_filter = basic_bloom_filter<T, basic_cache_aligned_mixer<T>>;

template <typename T, typename TMixer = basic_mixer<T>>
using thread_safe_bloom_filter = basic_bloom_filter<T, TMixer, thread_safe_storage>;

/** \} **/

}
