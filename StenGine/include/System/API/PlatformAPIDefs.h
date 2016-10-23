#pragma once

#ifdef _DEBUG
#define BUILD_DEBUG 1
#else
#define BUILD_DEBUG 0
#endif

#ifdef NDEBUG
#define BUILD_RELEASE 1
#else
#define BUILD_RELEASE 0
#endif