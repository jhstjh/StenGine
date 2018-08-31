#include "stdafx.h"
#include "Utility/CommandlineParser.h"
#include <string>
#include <vector>
#include <cstring>
#include <unordered_map>

#pragma warning (disable: 4996) // strcpy

namespace StenGine
{

class CommandlineParserImpl : public CommandlineParser
{
public:
	void Init(const char* cmdline) final
	{
		char scratch[256];
		strcpy(scratch, cmdline);
		std::vector<std::string>* currCmd = nullptr;
		char* pch = nullptr;
		pch = strtok(scratch, " ");
		while (pch != nullptr)
		{
			if (pch[0] == L'-')
			{
				currCmd = &mCmds[std::string(pch)];
			}
			else
			{
				currCmd->push_back(std::string(pch));
			}
			pch = strtok(nullptr, " ");
		}
	}

	const char* GetCommand(const char* cmd, uint32_t index /* = 0 */) final
	{
		auto entry = mCmds.find(std::string(cmd));
		if (entry != mCmds.end())
		{
			if (index >= entry->second.size())
				return nullptr;
			return entry->second[index].c_str();
		}
		return nullptr;
	}

private:
	std::unordered_map<std::string, std::vector<std::string> > mCmds;
};

DEFINE_ABSTRACT_SINGLETON_CLASS(CommandlineParser, CommandlineParserImpl);

}