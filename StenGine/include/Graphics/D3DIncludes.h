#ifndef __D3DINCLUDES__
#define __D3DINCLUDES__

#include <d3d11.h>
#include "Utility/Timer.h"
#include <DirectXMath.h>
#include <assert.h>
#include <string>
#include <vector>
#include <deque>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <algorithm>

#if GRAPHICS_D3D11
#include "DDSTextureLoader.h"
#endif

#include "DirectXPackedVector.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

#define SafeDelete(x)	\
{						\
	if (x)				\
		delete (x);		\
	x = 0;				\
}


#define SafeDeleteArray(x)	\
{							\
	if (x)					\
		delete[] (x);		\
	x = 0;					\
}

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }

#define SafeRelease(x) { if (x) {x->Release(); x = 0; }}

#if (DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x) assert(SUCCEEDED(x))
#endif


#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

#endif