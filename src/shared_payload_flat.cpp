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

NALCHI_FLAT_API auto nalchi_shared_payload_size(const nalchi::shared_payload payload)
    -> nalchi::shared_payload::alloc_size_t
{
    return payload.size();
}

NALCHI_FLAT_API auto nalchi_shared_payload_word_ceiled_size(const nalchi::shared_payload payload)
    -> nalchi::shared_payload::alloc_size_t
{
    return payload.word_ceiled_size();
}

NALCHI_FLAT_API auto nalchi_shared_payload_internal_alloc_size(const nalchi::shared_payload payload)
    -> nalchi::shared_payload::alloc_size_t
{
    return payload.internal_alloc_size();
}

NALCHI_FLAT_API bool nalchi_shared_payload_used_bit_stream(const nalchi::shared_payload payload)
{
    return payload.used_bit_stream();
}
