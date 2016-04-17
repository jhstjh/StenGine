#ifndef __GL_IMAGE_LOADER_H_
#define __GL_IMAGE_LOADER_H_

#include "glew.h"
#include <stdint.h>

namespace StenGine
{

GLuint CreateGLTextureFromFile(const char* filename);

}

#endif