/// @file
/// @brief Shared payload flat API.

#pragma once

#include "nalchi/shared_payload.hpp"

#include "nalchi/export.hpp"

/// @brief Allocates a shared payload that can be used to send some data.
/// @note You should check if `ptr` is `nullptr` or not
/// to see if the allocation has been successful.
/// @param size Space in bytes to allocate.
/// @return Shared payload instance that might hold allocated buffer.
NALCHI_FLAT_API nalchi::shared_payload nalchi_shared_payload_allocate(nalchi::shared_payload::alloc_size_t size);

/// @brief Force deallocates the shared payload without sending it.
/// @note If you send the payload, nalchi takes the ownership of the payload and releases it automatically. \n
/// So, you should @b not call this if you already sent the payload. \n \n
/// Calling this is only necessary when you have some exceptions in your program
/// that prevents sending the allocated payload.
/// @param payload Shared payload to force deallocate.
NALCHI_FLAT_API void nalchi_shared_payload_force_deallocate(nalchi::shared_payload payload);

/// @brief Gets the requested allocation size of the payload.
/// @return Size of the payload in bytes.
NALCHI_FLAT_API auto nalchi_shared_payload_size(const nalchi::shared_payload payload)
    -> nalchi::shared_payload::alloc_size_t;

/// @brief Gets the payload size that's ceiled to `bit_stream_writer::word_size`,
/// which is guaranteed to be safe to access.
///
/// As this size is ceiled to multiple of `bit_stream_writer::word_size`,
/// it can be bigger than the requested allocation size. \n
/// This is to maintain compatibility with `bit_stream_writer`.
/// @return Size of the payload in bytes.
NALCHI_FLAT_API auto nalchi_shared_payload_word_ceiled_size(const nalchi::shared_payload payload)
    -> nalchi::shared_payload::alloc_size_t;

/// @brief Gets the actual allocated size,
/// which includes hidden ref count & size fields.
///
/// This is meant to be only used by the internal API.
/// @return Size of the allocated space in bytes.
NALCHI_FLAT_API auto nalchi_shared_payload_internal_alloc_size(const nalchi::shared_payload payload)
    -> nalchi::shared_payload::alloc_size_t;

/// @brief Check if this payload used `bit_stream_writer` to fill its content.
///
/// If this is `true`, the send size will be automatically
/// ceiled to multiple of `bit_stream_reader::word_size` when you send it. \n
/// This is to maintain compatibility with `bit_stream_reader` on the receiving side.
/// @return Whether the payload used `bit_stream_writer` or not.
NALCHI_FLAT_API bool nalchi_shared_payload_used_bit_stream(const nalchi::shared_payload payload);
