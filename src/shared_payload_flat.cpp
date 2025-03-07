#include "nalchi/shared_payload_flat.hpp"

#include "nalchi/shared_payload.hpp"

NALCHI_FLAT_API nalchi::shared_payload nalchi_shared_payload_allocate(nalchi::shared_payload::alloc_size_t size)
{
    return nalchi::shared_payload::allocate(size);
}

NALCHI_FLAT_API void nalchi_shared_payload_force_deallocate(nalchi::shared_payload payload)
{
    return nalchi::shared_payload::force_deallocate(payload);
}

NALCHI_FLAT_API auto nalchi_shared_payload_get_size(const nalchi::shared_payload payload)
    -> nalchi::shared_payload::alloc_size_t
{
    return payload.get_size();
}

NALCHI_FLAT_API auto nalchi_shared_payload_get_aligned_size(const nalchi::shared_payload payload)
    -> nalchi::shared_payload::alloc_size_t
{
    return payload.get_aligned_size();
}

NALCHI_FLAT_API auto nalchi_shared_payload_get_internal_alloc_size(const nalchi::shared_payload payload)
    -> nalchi::shared_payload::alloc_size_t
{
    return payload.get_internal_alloc_size();
}
