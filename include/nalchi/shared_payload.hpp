#pragma once

#include <atomic>
#include <cstdint>

#include "nalchi/export.hpp"

namespace nalchi
{

/// @brief Shared payload to store data to send.
///
/// The payload is "shared" when it is used for multicast or broadcast.
/// @note As `ptr` has a hidden reference count & alloc size fields on front,
/// you @b CAN'T just use your own buffer as a `ptr`; \n
/// You need to call `nalchi_allocate_shared_payload()` to allocate the `shared_payload`.
struct shared_payload
{
    using ref_count_t = std::atomic<std::int32_t>;
    using alloc_size_t = std::uint32_t;

    void* ptr; ///< Pointer to the payload, allocated by nalchi.

    /// @brief Allocates a shared payload that can be used to send some data.
    /// @note You should check if `ptr` is `nullptr` or not
    /// to see if the allocation has been successful.
    /// @param size Space in bytes to allocate.
    /// @return Shared payload instance that might hold allocated buffer.
    NALCHI_API static shared_payload allocate(alloc_size_t size);

    /// @brief Force deallocates the shared payload without sending it.
    /// @note If you send the payload, nalchi takes the ownership of the payload and releases it automatically. \n
    /// So, you should @b NOT call this if you already sent the payload. \n \n
    /// Calling this is only necessary when you have some exceptions in your program
    /// that prevents sending the allocated payload.
    /// @param payload Shared payload to force deallocate.
    NALCHI_API static void force_deallocate(shared_payload payload);

    /// @brief Gets the requested allocation size of the payload.
    /// @return Size of the payload in bytes.
    NALCHI_API auto get_size() const -> alloc_size_t;

    /// @brief Gets the aligned size of the payload,
    /// which is guaranteed to be safe to access.
    ///
    /// This size can be bigger than the requested allocation size. \n
    /// This is to maintain compatibility with `bit_stream_writer`.
    /// @return Size of the payload in bytes.
    NALCHI_API auto get_aligned_size() const -> alloc_size_t;

    /// @brief Gets the actual allocated size,
    /// which includes hidden ref count & size fields.
    ///
    /// This is meant to be only used by the internal API.
    /// @return Size of the allocated space in bytes.
    NALCHI_API auto get_internal_alloc_size() const -> alloc_size_t;
};

} // namespace nalchi
