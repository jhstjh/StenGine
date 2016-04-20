#ifndef __GL_IMAGE_LOADER_H_
#define __GL_IMAGE_LOADER_H_

#include "glew.h"
#include <stdint.h>
#include <string>
#include <vector>

namespace StenGine
{

GLuint CreateGLTextureFromFile(const char* filename);

GLuint CreateGLTextureArrayFromFile(std::vector<std::wstring> &filenames);

}

#endif