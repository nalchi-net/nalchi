#include "nalchi/shared_payload.hpp"

#include "nalchi/bit_stream.hpp"

#include "aligned_alloc.hpp"
#include "math.hpp"

#include <steam/steamnetworkingtypes.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <memory>

namespace nalchi
{

namespace
{
constexpr auto GNS_MAX_MSG_SEND_SIZE = k_cbMaxSteamNetworkingSocketsMessageSizeSend;
}

NALCHI_API shared_payload shared_payload::allocate(alloc_size_t size)
{
    shared_payload payload{};

    if (0 < size && size <= GNS_MAX_MSG_SEND_SIZE)
    {
        // Alignment should be the max of:
        // * `bit_stream_writer`'s word write measure, to avoid unaligned write to payload
        // * atomic reference count, to avoid tearing of ref count
        constexpr std::size_t ALLOC_ALIGNMENT = std::max({
            alignof(ref_count_t),                  // to store ref count
            alignof(alloc_size_t),                 // to store requested payload size
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
        const alloc_size_t alloc_size =
            static_cast<alloc_size_t>(sizeof(ref_count_t) + sizeof(alloc_size_t) +
                                      ceil_to_multiple_of<sizeof(bit_stream_writer::word_type)>(size));

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

            // Use the second space to store requested payload size.
            alloc_size_t* req_payload_len =
                reinterpret_cast<alloc_size_t*>((std::byte*)raw_space + sizeof(ref_count_t));
            *req_payload_len = size;

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

NALCHI_API auto shared_payload::get_size() const -> alloc_size_t
{
    // Get the hidden requested payload size field
    return *reinterpret_cast<alloc_size_t*>((std::byte*)ptr - sizeof(alloc_size_t));
}

NALCHI_API auto shared_payload::get_aligned_size() const -> alloc_size_t
{
    return ceil_to_multiple_of<sizeof(bit_stream_writer::word_type)>(get_size());
}

NALCHI_API auto shared_payload::get_internal_alloc_size() const -> alloc_size_t
{
    return static_cast<alloc_size_t>(sizeof(ref_count_t) + sizeof(alloc_size_t) + get_aligned_size());
}

} // namespace nalchi
