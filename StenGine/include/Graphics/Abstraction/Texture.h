#pragma once
#include <memory>

namespace StenGine
{

class TextureImpl
{
public:
	virtual ~TextureImpl() = default;
	virtual void* GetTexture() = 0;
	virtual void GetDimension(uint32_t &width, uint32_t &height) = 0;
};

using Texture = std::shared_ptr<TextureImpl>;

}
