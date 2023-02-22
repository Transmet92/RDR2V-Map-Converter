#pragma once
#include "SkyCommons/SkyAtom.h"


extern std::atomic<uint64> g_FilesProcessed;

class CConversionWorker
{
public:
	SkyVector<SkyString> m_Files;

public:
	DWORD Worker();

	static DWORD WorkerAdapter(void* lp) { return ((CConversionWorker*)lp)->Worker(); }
};
