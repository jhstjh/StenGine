#ifndef __GL_IMAGE_LOADER_H_
#define __GL_IMAGE_LOADER_H_

#include "glew.h"
#include <stdint.h>
#include <string>
#include <vector>

namespace StenGine
{

GLuint CreateGLTextureFromFile(const char* filename, uint32_t* width = nullptr, uint32_t* height = nullptr);

GLuint CreateGLTextureArrayFromFiles(std::vector<std::wstring> &filenames, uint32_t* width = nullptr, uint32_t* height = nullptr);

}

#endif