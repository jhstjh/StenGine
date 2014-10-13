#ifndef __D3DINCLUDES__
#define __D3DINCLUDES__

#include <d3dx11.h>
//#include "d3dx11Effect.h"
#include <xnamath.h>
#include <dxerr.h>
#include <assert.h>

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                              \
				{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
						{                                                      \
			DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); \
						}                                                      \
				}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 


#endif