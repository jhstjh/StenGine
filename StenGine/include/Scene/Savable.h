#ifndef __SAVABLE_H__
#define __SAVABLE_H__

#include "tinyxml2.h"

namespace StenGine
{

class Savable
{
public:
	virtual tinyxml2::XMLElement* Save(tinyxml2::XMLDocument &doc) const = 0;
};

}

#endif