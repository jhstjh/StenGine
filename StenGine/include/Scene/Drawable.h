#pragma once

class Drawable
{
public:
	Drawable();

	virtual void GatherDrawCall() = 0;
	virtual void GatherShadowDrawCall() = 0;
};