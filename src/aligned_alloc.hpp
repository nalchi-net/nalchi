#pragma once

#ifdef _MSC_VER // MSVCRT

#define NALCHI_ALIGNED_ALLOC(alignment, size) _aligned_malloc(size, alignment)
#define NALCHI_ALIGNED_FREE(ptr) _aligned_free(ptr)

#else // Standard

#define NALCHI_ALIGNED_ALLOC(alignment, size) std::aligned_alloc(alignment, size)
#define NALCHI_ALIGNED_FREE(ptr) std::free(ptr)

#endif
