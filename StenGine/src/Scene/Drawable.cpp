#include "Scene/Drawable.h"
#include "Graphics/Abstraction/RendererBase.h"


Drawable::Drawable()
{
	auto drawHandler = 
	[this]()
	{
		GatherDrawCall();
	};

	Renderer::Instance()->AddDraw(drawHandler);

	auto shadowDrawHandler =
		[this]()
	{
		GatherShadowDrawCall();
	};

	Renderer::Instance()->AddShadowDraw(shadowDrawHandler);
}

// TODO remove draw handler