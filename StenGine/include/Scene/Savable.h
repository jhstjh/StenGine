#ifndef __SAVABLE_H__
#define __SAVABLE_H__

namespace StenGine
{

class Savable
{
public:
	virtual void save() = 0;
};

}

#endif