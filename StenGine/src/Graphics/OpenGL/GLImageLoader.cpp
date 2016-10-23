#pragma warning(disable: 4996)

#include "Graphics/OpenGL/GLImageLoader.h"
#include "gli/gli.hpp"

namespace StenGine
{

GLuint CreateGLTextureFromFile(const char* filename, uint32_t* width, uint32_t* height)
{
	gli::texture Texture = gli::load(filename);
	if (Texture.empty())
		return 0;

	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
	GLenum Target = GL.translate(Texture.target());

	GLuint TextureName = 0;
	glCreateTextures(Target, 1, &TextureName);
	//glBindTexture(Target, TextureName);
	glTextureParameteri(TextureName, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(TextureName, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
	glTextureParameteri(TextureName, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
	glTextureParameteri(TextureName, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
	glTextureParameteri(TextureName, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
	glTextureParameteri(TextureName, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);
	glTextureParameteri(TextureName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(TextureName, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glm::tvec3<GLsizei> const Extent(Texture.extent());
	GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());

	switch (Texture.target())
	{
	case gli::TARGET_1D:
		glTextureStorage1D(
			TextureName, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x);
		break;
	case gli::TARGET_1D_ARRAY:
	case gli::TARGET_2D:
	case gli::TARGET_CUBE:
		glTextureStorage2D(
			TextureName, static_cast<GLint>(Texture.levels()), Format.Internal,
			Extent.x, Texture.target() == gli::TARGET_2D || Texture.target() == gli::TARGET_CUBE ? Extent.y : FaceTotal);
		break;
	case gli::TARGET_2D_ARRAY:
	case gli::TARGET_3D:
	case gli::TARGET_CUBE_ARRAY:
		glTextureStorage3D(
			TextureName, static_cast<GLint>(Texture.levels()), Format.Internal,
			Extent.x, Extent.y,
			Texture.target() == gli::TARGET_3D ? Extent.z : FaceTotal);
		break;
	default:
		assert(0);
		break;
	}

	GLuint pbo;
	glCreateBuffers(1, &pbo);
	glNamedBufferData(pbo, Texture.size(), nullptr, GL_STREAM_DRAW);
	void* pboData = glMapNamedBuffer(pbo, GL_WRITE_ONLY);

	memcpy(pboData, Texture.data(), Texture.size());

	glUnmapNamedBuffer(pbo);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);

	uint32_t pboSize = 0;

	for (std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
	{
		for (std::size_t Face = 0; Face < Texture.faces(); ++Face)
		{
			for (std::size_t Level = 0; Level < Texture.levels(); ++Level)
			{
				GLsizei const LayerGL = static_cast<GLsizei>(Layer);
				glm::tvec3<GLsizei> Extent(Texture.extent(Level));
				Target = gli::is_target_cube(Texture.target())
					? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
					: Target;

				switch (Texture.target())
				{
				case gli::TARGET_1D:
					if (gli::is_compressed(Texture.format()))
						glCompressedTextureSubImage1D(
							TextureName, static_cast<GLint>(Level), 0, Extent.x,
							Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
							(void*)pboSize);
					else
						glTextureSubImage1D(
							TextureName, static_cast<GLint>(Level), 0, Extent.x,
							Format.External, Format.Type,
							(void*)pboSize);
					break;
				case gli::TARGET_1D_ARRAY:
				case gli::TARGET_2D:
					//case gli::TARGET_CUBE:
					if (gli::is_compressed(Texture.format()))
						glCompressedTextureSubImage2D(
							TextureName, static_cast<GLint>(Level),
							0, 0,
							Extent.x,
							Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
							Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
							(void*)pboSize);
					else
						glTextureSubImage2D(
							TextureName, static_cast<GLint>(Level),
							0, 0,
							Extent.x,
							Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
							Format.External, Format.Type,
							(void*)pboSize);
					break;
				case gli::TARGET_2D_ARRAY:
				case gli::TARGET_3D:
				case gli::TARGET_CUBE_ARRAY:
				case gli::TARGET_CUBE:
					if (gli::is_compressed(Texture.format()))
						glCompressedTextureSubImage3D(
							TextureName, static_cast<GLint>(Level),
							0, 0, Texture.target() == gli::TARGET_CUBE ? Face : 0,
							Extent.x, Extent.y,
							(Texture.target() == gli::TARGET_3D || Texture.target() == gli::TARGET_CUBE) ? Extent.z : LayerGL,
							Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
							(void*)pboSize);
					else
						glTextureSubImage3D(
							TextureName, static_cast<GLint>(Level),
							0, 0, Texture.target() == gli::TARGET_CUBE ? Face : 0,
							Extent.x, Extent.y,
							(Texture.target() == gli::TARGET_3D || Texture.target() == gli::TARGET_CUBE) ? Extent.z : LayerGL,
							Format.External, Format.Type,
							(void*)pboSize);
					break;
				default: assert(0); break;
				}

				pboSize += Texture.size(Level);
			}
		}
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glDeleteBuffers(1, &pbo);

	if (width) *width = Texture.extent(0).x;
	if (height) *height = Texture.extent(0).y;

	return TextureName;
}

GLuint CreateGLTextureArrayFromFiles(std::vector<std::wstring>& filenames, uint32_t* width, uint32_t* height)
{
	std::vector<gli::texture> Textures;
	uint32_t totalSize = 0;

	for (uint32_t i = 0; i < filenames.size(); ++i)
	{
		gli::texture tex = gli::load(std::string(filenames[i].begin(), filenames[i].end()));
		totalSize += tex.size();
		Textures.push_back(std::move(tex));
	}

	assert(Textures.size() > 0);

	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format const Format = GL.translate(Textures[0].format(), Textures[0].swizzles());
	glm::tvec3<GLsizei> const Extent(Textures[0].extent());

	GLuint texArray = 0;
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texArray);
	glTextureStorage3D(texArray, static_cast<GLint>(Textures[0].levels()), Format.Internal, Extent.x, Extent.y, Textures.size());
	glTextureParameteri(texArray, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
	glTextureParameteri(texArray, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
	glTextureParameteri(texArray, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
	glTextureParameteri(texArray, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);
	glTextureParameteri(texArray, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(texArray, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	GLuint pbo;
	glCreateBuffers(1, &pbo);
	glNamedBufferData(pbo, totalSize, nullptr, GL_STREAM_DRAW);
	uint8_t* pboData = (uint8_t*)glMapNamedBuffer(pbo, GL_WRITE_ONLY);

	uint32_t pboOffset = 0;
	for (uint32_t i = 0; i < Textures.size(); ++i)
	{
		memcpy(pboData + pboOffset, Textures[i].data(0, 0, 0),  Textures[i].size(0));
		pboOffset += Textures[i].size(0);
	}

	glUnmapNamedBuffer(pbo);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);

	pboOffset = 0;
	for (std::size_t Layer = 0; Layer < Textures[0].layers(); ++Layer)
		for (std::size_t Face = 0; Face < Textures[0].faces(); ++Face)
			//for (std::size_t Level = 0; Level < Textures[0].levels(); ++Level)
			{
				GLsizei const LayerGL = static_cast<GLsizei>(Layer);
				if (gli::is_compressed(Textures[0].format()))
				{
					//glCompressedTextureSubImage3D(
					//	texArray, static_cast<GLint>(Level),
					//	0, 0, Textures[0].target() == gli::TARGET_CUBE ? Face : 0,
					//	Extent.x, Extent.y,
					//	(Textures[0].target() == gli::TARGET_3D || Textures[0].target() == gli::TARGET_CUBE) ? Extent.z : LayerGL,
					//	Format.Internal, static_cast<GLsizei>(Textures[0].size(Level)),
					//	(void*)pboOffset);
				}
				else
				{
					glTextureSubImage3D(
						texArray, 0/*static_cast<GLint>(Level)*/,
						0, 0, Textures[0].target() == gli::TARGET_CUBE ? Face : 0,
						Extent.x, Extent.y,
						Textures.size(),
						Format.External, Format.Type,
						(void*)pboOffset);
				}

				//pboOffset += Textures[0].size(Level);
			}

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glDeleteBuffers(1, &pbo);

	glGenerateTextureMipmap(texArray);

	if (width) *width = Textures[0].extent(0).x;
	if (height) *height = Textures[0].extent(0).y;
	return texArray;
}

}