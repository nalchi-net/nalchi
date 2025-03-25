#pragma once

#if defined(_WIN32)

#include <malloc.h>
#define NALCHI_ALLOCA _alloca

#else

#include <alloca.h>
#define NALCHI_ALLOCA alloca

#endif
