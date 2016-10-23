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

class Texture
{
public:
	Texture(uint32_t width, uint32_t height, void* srv);

	void* GetTexture();
	void GetDimension(uint32_t &width, uint32_t &height);

private:
	std::unique_ptr<TextureImpl> mImpl;
};

}
