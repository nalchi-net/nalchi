/// @file
/// @brief Payload related functionality.

#pragma once

#include "nalchi/export.h"

#include <stdint.h>

/// @brief Payload to send or receive.
/// @note As `ptr` has a hidden reference count on front,
/// you @b can't just use your own buffer as a `ptr`; \n
/// You need to call `nalchi_allocate_payload()` to allocate the `nalchi_payload`.
struct nalchi_payload
{
    void* ptr;     ///< Pointer to the payload, allocated by nalchi.
    uint32_t size; ///< Size of the payload.
};

/// @brief Allocates a payload that can be used to send some data.
/// @note You should check if `ptr` is `nullptr` or not
/// to see if the allocation has been successful.
/// @param size Space in bytes to allocate.
/// @return Payload instance that might hold allocated buffer.
NALCHI_FLAT_API nalchi_payload nalchi_allocate_payload(uint32_t size);

/// @brief Force deallocates the payload without sending it.
/// @note If you send the payload, nalchi takes the ownership of the payload and releases it automatically. \n
/// So, you should @b NOT call this if you already sent the payload. \n \n
/// Calling this is only necessary when you have some exceptions in your program
/// that prevents sending the allocated payload.
/// @param payload Payload to force deallocate.
NALCHI_FLAT_API void nalchi_force_deallocate_payload(nalchi_payload payload);
