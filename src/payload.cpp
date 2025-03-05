#include "nalchi/payload.h"

#include "nalchi/bit_stream.hpp"

#include "aligned_alloc.hpp"
#include "math.hpp"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdlib>

using ref_count_t = std::atomic_int32_t;

NALCHI_FLAT_API nalchi_payload nalchi_allocate_payload(uint32_t size)
{
    nalchi_payload payload{};

    if (size != 0)
    {
        // Alignment should be the max of:
        // * `bit_stream_writer`'s word write measure, to avoid unaligned write to payload
        // * atomic reference count, to avoid tearing of ref count
        static constexpr std::size_t ALLOC_ALIGNMENT = std::max({
            alignof(nalchi::bit_stream_writer::word_type),
            alignof(ref_count_t),
        });

        // If this was not true, write to payload with `bit_stream_writer` would be unaligned.
        // In that case, we should call `std::align()` to manually align the buffer.
        // But that's slow, and we don't need that right now.
        static_assert(sizeof(ref_count_t) % ALLOC_ALIGNMENT == 0);

        // Calculate the required space for (ref count + actual payload)
        // Actual payload size should be ceiled to `bit_stream_writer`'s word's multiple,
        // to avoid buffer overrun on last scratch write in `bit_stream_writer`.
        const std::size_t alloc_size =
            sizeof(ref_count_t) + nalchi::ceil_to_multiple_of<sizeof(nalchi::bit_stream_writer::word_type)>(size);

        // We actually don't need `std::aligned_alloc()`.
        // `std::malloc()` is already sufficiently aligned.
        static_assert(alignof(std::max_align_t) >= ALLOC_ALIGNMENT);
        void* raw_space = std::malloc(alloc_size);

        if (raw_space)
        {
            // Use the front space as a ref count.
            ref_count_t* ref_count = ::new (static_cast<void*>(raw_space)) ref_count_t;

#if __cplusplus < 202002L // Explicit zero init required before C++20
            ref_count->store(0, std::memory_order_relaxed);
#else
            ((void)ref_count); // Suppress the unused variable warning.
#endif

            // Point to the actual payload space.
            payload.ptr = static_cast<void*>((std::byte*)raw_space + sizeof(ref_count_t));
            payload.size = size;
        }
    }

    return payload;
}

NALCHI_FLAT_API void nalchi_force_deallocate_payload(nalchi_payload payload)
{
    // Ref count should exist before the payload.
    ref_count_t* ref_count = reinterpret_cast<ref_count_t*>((std::byte*)payload.ptr - sizeof(ref_count_t));

    // Destroy the ref count. (Is this actually necessary?)
    ref_count->~ref_count_t();

    // Free the space (ref count is the real alloc address)
    std::free(ref_count);
}
