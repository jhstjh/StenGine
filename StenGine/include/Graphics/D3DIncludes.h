#ifndef __D3DINCLUDES__
#define __D3DINCLUDES__

#include <d3d11.h>
#include "Utility/Timer.h"
#include <assert.h>
#include <string>
#include <vector>
#include <deque>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <algorithm>

#include "DDSTextureLoader.h"

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