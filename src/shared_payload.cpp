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

constexpr shared_payload::alloc_size_t BIT_STREAM_USED_FLAG_MASK = shared_payload::alloc_size_t(1)
                                                                   << (8 * sizeof(shared_payload::alloc_size_t) - 1);
constexpr shared_payload::alloc_size_t PAYLOAD_SIZE_MASK = ~BIT_STREAM_USED_FLAG_MASK;

static_assert(GNS_MAX_MSG_SEND_SIZE <= PAYLOAD_SIZE_MASK,
              "Not enough space to store bit stream used flag in msb of payload size field");

} // namespace

NALCHI_API shared_payload shared_payload::allocate(alloc_size_t size)
{
    shared_payload payload{};

    if (0 < size && size <= GNS_MAX_MSG_SEND_SIZE)
    {
        // Alignment should be the max of:
        // * `bit_stream_writer`'s word write measure, to avoid unaligned write to payload
        // * atomic reference count, to avoid tearing of ref count
        // * payload size + bit stream used flag, to avoid tearing of it
        constexpr std::size_t ALLOC_ALIGNMENT = std::max({
            alignof(ref_count_t),                  // to store ref count
            alignof(alloc_size_t),                 // to store requested payload size + bit stream used flag
            alignof(bit_stream_writer::word_type), // to store payload data
        });

        // If these were not true, write to payload with `bit_stream_writer` would be unaligned.
        // In that case, we should call `std::align()` to manually align the buffer.
        // But that's slow, and we don't need that right now.
        static_assert(sizeof(ref_count_t) % alignof(alloc_size_t) == 0);
        static_assert((sizeof(ref_count_t) + sizeof(alloc_size_t)) % alignof(bit_stream_writer::word_type) == 0);

        // Calculate the required space for (ref count + payload size & bit stream used flag + actual payload)
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

            // Use the second space to store requested payload size + bit stream used flag.
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
    // Get the hidden ref count
    ref_count_t* ref_count_ptr = &payload.ref_count();

    // Destroy the ref count. (Is this actually necessary?)
    std::destroy_at(ref_count_ptr);

    // Free the space (ref count is the real alloc address)
    std::free(ref_count_ptr);
}

NALCHI_API auto shared_payload::size() const -> alloc_size_t
{
    // Get the hidden requested payload size + bit stream used flag
    const alloc_size_t raw_field = payload_size_and_bit_stream_used_flag();

    // Filter the payload size with the mask
    return raw_field & PAYLOAD_SIZE_MASK;
}

NALCHI_API auto shared_payload::word_ceiled_size() const -> alloc_size_t
{
    return ceil_to_multiple_of<sizeof(bit_stream_writer::word_type)>(size());
}

NALCHI_API auto shared_payload::internal_alloc_size() const -> alloc_size_t
{
    return static_cast<alloc_size_t>(sizeof(ref_count_t) + sizeof(alloc_size_t) + word_ceiled_size());
}

bool shared_payload::used_bit_stream() const
{
    // Get the hidden requested payload size + bit stream used flag
    const alloc_size_t raw_field = payload_size_and_bit_stream_used_flag();

    // Filter the bit stream used flag with the mask
    return (raw_field & BIT_STREAM_USED_FLAG_MASK) != 0;
}

void shared_payload::set_used_bit_stream(bool used)
{
    // Get the hidden requested payload size + bit stream used flag
    alloc_size_t& raw_field = payload_size_and_bit_stream_used_flag();

    // Set the bit stream used flag part
    if (used)
        raw_field |= BIT_STREAM_USED_FLAG_MASK;
    else
        raw_field &= ~BIT_STREAM_USED_FLAG_MASK;
}

NALCHI_API void shared_payload::add_to_message(SteamNetworkingMessage_t* msg, int logical_bytes_length)
{
    // If used bit stream, ceil the send size to `bit_stream_reader::word_type`.
    // Otherwise, the receiving side might read out-of-bound memory.
    if (used_bit_stream())
        logical_bytes_length = ceil_to_multiple_of<sizeof(bit_stream_reader::word_type)>(logical_bytes_length);

    increase_ref_count();

    // Add the payload to the message
    msg->m_pData = ptr;
    msg->m_cbSize = logical_bytes_length;
    msg->m_pfnFreeData = decrease_ref_count_and_deallocate_if_zero_callback;
}

void shared_payload::decrease_ref_count_and_deallocate_if_zero_callback(SteamNetworkingMessage_t* msg)
{
    shared_payload payload{.ptr = msg->m_pData};

    payload.decrease_ref_count_and_deallocate_if_zero();
}

void shared_payload::increase_ref_count()
{
    ref_count_t* ref_count_ptr = &ref_count();

    ref_count_ptr->fetch_add(1, std::memory_order_relaxed);
}

void shared_payload::decrease_ref_count_and_deallocate_if_zero()
{
    ref_count_t* ref_count_ptr = &ref_count();

    // if this was the last reference, deallocate self
    if (1 == ref_count_ptr->fetch_sub(1, std::memory_order_relaxed))
        force_deallocate(*this);
}

auto shared_payload::ref_count() -> ref_count_t&
{
    // Ref count should exist before the payload, before the payload size field.
    return *reinterpret_cast<ref_count_t*>((std::byte*)ptr - sizeof(alloc_size_t) - sizeof(ref_count_t));
}

auto shared_payload::ref_count() const -> const ref_count_t&
{
    // Ref count should exist before the payload, before the payload size field.
    return *reinterpret_cast<ref_count_t*>((std::byte*)ptr - sizeof(alloc_size_t) - sizeof(ref_count_t));
}

auto shared_payload::payload_size_and_bit_stream_used_flag() -> alloc_size_t&
{
    // This field should exist right before the payload.
    return *reinterpret_cast<alloc_size_t*>((std::byte*)ptr - sizeof(alloc_size_t));
}

auto shared_payload::payload_size_and_bit_stream_used_flag() const -> const alloc_size_t&
{
    // This field should exist right before the payload.
    return *reinterpret_cast<const alloc_size_t*>((std::byte*)ptr - sizeof(alloc_size_t));
}

} // namespace nalchi
