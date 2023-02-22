#pragma once
#include "SkyCommons/SkyCommons.h"
#include "SkyCommons/SkyAtom.h"
#include "SkyCommons/SkyContainer.h"
#include <unordered_map>

#define LOCAL_THREAD_BUFFER_SIZE 100 * 1024 * 1024

extern thread_local char* g_LocalThreadBuffer;
extern thread_local char* g_LocalThreadBuffer2;


class CThreadShared
{
public:
	std::unordered_map<uint32, bool> m_HashModelsProcessed;
	CSecure m_Critical;

public:

	// retourne true si le modèle a déjà été traité
	bool PushModel(uint32 hash)
	{
		if (!hash)
			return false;

		bool out = false;
		m_Critical.Lock();

		auto match = m_HashModelsProcessed.find(hash);
		if (match == m_HashModelsProcessed.end())
		{
			m_HashModelsProcessed[hash] = true;
		}
		else
			out = true;

		m_Critical.Unlock();
		return out;
	}
};
#define ThreadShared atSingleton<CThreadShared>::Get()
