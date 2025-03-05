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
NALCHI_FLAT_API nalchi::shared_payload nalchi_shared_payload_allocate(int size);

/// @brief Force deallocates the shared payload without sending it.
/// @note If you send the payload, nalchi takes the ownership of the payload and releases it automatically. \n
/// So, you should @b NOT call this if you already sent the payload. \n \n
/// Calling this is only necessary when you have some exceptions in your program
/// that prevents sending the allocated payload.
/// @param payload Shared payload to force deallocate.
NALCHI_FLAT_API void nalchi_shared_payload_force_deallocate(nalchi::shared_payload payload);

/// @brief Gets the size of the payload.
/// @return Size of the payload in bytes.
NALCHI_FLAT_API int nalchi_shared_payload_get_payload_size(const nalchi::shared_payload* self);
