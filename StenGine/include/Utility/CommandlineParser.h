#pragma once
#include "System/SingletonClass.h"
#include <cstdint>

namespace StenGine
{

class CommandlineParser : public AbstractSingletonClass<CommandlineParser>
{
public:
	virtual void Init(const wchar_t* cmdline) = 0;
	virtual const wchar_t* GetCommand(const wchar_t* cmd, uint32_t index = 0) = 0;

	DECLARE_ABSTRACT_SINGLETON_CLASS(CommandlineParser)
};

}