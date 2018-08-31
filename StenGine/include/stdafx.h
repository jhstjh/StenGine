// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
// Windows Header Files:
#include <windows.h>
#include <guiddef.h>

// C RunTime Header Files
#include <algorithm>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <vector>
#include <stdint.h>
#include <functional>
#include <unordered_map>
#include <type_traits>
#include <unordered_set>

// TODO: reference additional headers your program requires here
#include "Graphics/D3DIncludes.h"
#include "System/API/PlatformAPIDefs.h"
#include "Math/MathDefs.h"
#include "Math/MathHelper.h"

#define UNUSED(x) __pragma(warning(suppress: 4100 4101)) x
