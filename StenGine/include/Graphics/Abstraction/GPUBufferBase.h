#pragma once

namespace StenGine
{

enum class BufferUsage
{
	IMMUTABLE,
	DYNAMIC,
	WRITE,
};

enum class BufferType
{
	GENERAL = 0x00L,
	VERTEX_BUFFER = 0x01L,
	INDEX_BUFFER = 0x02L,
	CONSTANT_BUFFER = 0x04L,
	SHADER_RESOURCE = 0x08L,
	UNORDERED_ACCESS = 0x80L,
};

}