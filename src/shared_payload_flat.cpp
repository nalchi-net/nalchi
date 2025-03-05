#include "nalchi/shared_payload_flat.hpp"

#include "nalchi/shared_payload.hpp"

NALCHI_FLAT_API nalchi::shared_payload nalchi_shared_payload_allocate(int size)
{
    return nalchi::shared_payload::allocate(size);
}

NALCHI_FLAT_API void nalchi_shared_payload_force_deallocate(nalchi::shared_payload payload)
{
    return nalchi::shared_payload::force_deallocate(payload);
}

NALCHI_FLAT_API int nalchi_shared_payload_get_payload_size(const nalchi::shared_payload* self)
{
    return self->get_payload_size();
}
