#include "nalchi/shared_payload.hpp"

#include "nalchi/bit_stream.hpp"

#include "aligned_alloc.hpp"
#include "math.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <memory>

namespace nalchi
{

NALCHI_API shared_payload shared_payload::allocate(int size)
{
    shared_payload payload{};

    if (size > 0)
    {
        // Alignment should be the max of:
        // * `bit_stream_writer`'s word write measure, to avoid unaligned write to payload
        // * atomic reference count, to avoid tearing of ref count
        constexpr std::size_t ALLOC_ALIGNMENT = std::max({
            alignof(ref_count_t),                  // to store ref count
            alignof(alloc_size_t),                 // to store allocated buffer size
            alignof(bit_stream_writer::word_type), // to store payload data
        });

        // If these were not true, write to payload with `bit_stream_writer` would be unaligned.
        // In that case, we should call `std::align()` to manually align the buffer.
        // But that's slow, and we don't need that right now.
        static_assert(sizeof(ref_count_t) % alignof(alloc_size_t) == 0);
        static_assert((sizeof(ref_count_t) + sizeof(alloc_size_t)) % alignof(bit_stream_writer::word_type) == 0);

        // Calculate the required space for (ref count + alloc size + actual payload)
        // Actual payload size should be ceiled to `bit_stream_writer`'s word's multiple,
        // to avoid buffer overrun on last scratch write in `bit_stream_writer`.
        const alloc_size_t alloc_size = static_cast<alloc_size_t>(
            sizeof(ref_count_t) + sizeof(alloc_size_t) +
            ceil_to_multiple_of<sizeof(bit_stream_writer::word_type)>(static_cast<std::uint32_t>(size)));

        // We actually don't need `std::aligned_alloc()`.
        // `std::malloc()` is already sufficiently aligned.
        static_assert(alignof(std::max_align_t) >= ALLOC_ALIGNMENT);
        void* raw_space = std::malloc(alloc_size);

        if (raw_space)
        {
            // Use the first space as a ref count.
            ref_count_t* ref_count = std::construct_at(reinterpret_cast<ref_count_t*>(raw_space));

#if __cplusplus < 202002L // Explicit zero init required before C++20
            ref_count->store(0, std::memory_order_relaxed);
#else
            ((void)ref_count); // Suppress the unused variable warning.
#endif

            // Use the second space to store alloc size.
            alloc_size_t* alloc_len = reinterpret_cast<alloc_size_t*>((std::byte*)raw_space + sizeof(ref_count_t));
            *alloc_len = alloc_size;

            // Point to the actual payload space.
            payload.ptr = static_cast<void*>((std::byte*)raw_space + sizeof(ref_count_t) + sizeof(alloc_size_t));
        }
    }

    return payload;
}

NALCHI_API void shared_payload::force_deallocate(shared_payload payload)
{
    // Ref count should exist before the payload, before the alloc size.
    ref_count_t* ref_count =
        reinterpret_cast<ref_count_t*>((std::byte*)payload.ptr - sizeof(alloc_size_t) - sizeof(ref_count_t));

    // Destroy the ref count. (Is this actually necessary?)
    std::destroy_at(ref_count);

    // Free the space (ref count is the real alloc address)
    std::free(ref_count);
}

NALCHI_API int shared_payload::get_payload_size() const
{
    const auto size = *reinterpret_cast<shared_payload::alloc_size_t*>(
                          (std::byte*)ptr - sizeof(shared_payload::alloc_size_t)) // Get the hidden alloc size field
                      - sizeof(shared_payload::ref_count_t) -
                      sizeof(shared_payload::alloc_size_t); // Remove header size

    return static_cast<int>(size);
}

} // namespace nalchi
