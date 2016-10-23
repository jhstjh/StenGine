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
	void Init(const wchar_t* cmdline) final
	{
		wchar_t scratch[256];
		wcscpy(scratch, cmdline);
		std::vector<std::wstring>* currCmd = nullptr;
		wchar_t* pch = nullptr;
		pch = wcstok(scratch, L" ");
		while (pch != nullptr)
		{
			if (pch[0] == L'-')
			{
				currCmd = &mCmds[std::wstring(pch)];
			}
			else
			{
				currCmd->push_back(std::wstring(pch));
			}
			pch = wcstok(nullptr, L" ");
		}
	}

	const wchar_t* GetCommand(const wchar_t* cmd, uint32_t index /* = 0 */) final
	{
		auto entry = mCmds.find(std::wstring(cmd));
		if (entry != mCmds.end())
		{
			if (index >= entry->second.size())
				return nullptr;
			return entry->second[index].c_str();
		}
		return nullptr;
	}

private:
	std::unordered_map<std::wstring, std::vector<std::wstring> > mCmds;
};

DEFINE_ABSTRACT_SINGLETON_CLASS(CommandlineParser, CommandlineParserImpl);

}