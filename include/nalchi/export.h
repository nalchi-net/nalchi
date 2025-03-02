#pragma once

#if defined(NALCHI_BUILD_STATIC)

#define NALCHI_API

#elif defined(NALCHI_BUILD_EXPORT)

#if defined(_WIN32)
#define NALCHI_API __declspec(dllexport)
#else
#define NALCHI_API __attribute__((__visibility__("default")))
#endif

#else // NALCHI_BUILD_IMPORT

#if defined(_WIN32)
#define NALCHI_API __declspec(dllimport)
#else
#define NALCHI_API
#endif

#endif // NALCHI_BUILD

#if defined(__cplusplus)
#define NALCHI_FLAT_API extern "C" NALCHI_API
#else
#define NALCHI_FLAT_API NALCHI_API
#endif
