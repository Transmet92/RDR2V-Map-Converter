#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <SkyCommons/SkyCommons.h>
#include <SkyCommons/SkyPtr.h>
#include <SkyCommons/SkyFunctional.h>
#include <SkyCommons/SkyFile.h>

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <Psapi.h>
#include <iostream>
#include <vector>

#ifdef _WIN32
#define APIENTRY __stdcall
#endif

#include <fstream>
#include <dbghelp.h>
#include <winternl.h>
#include <shlobj_core.h>


class CBuffer
{
private:
	char* m_Buffer = 0;
	uint64 m_Size = 0;

public:
	CBuffer() {};
	CBuffer(char* buffer, uint64 size) : m_Buffer(buffer), m_Size(size) {};
	CBuffer(uint64 size) {
		m_Buffer = new char[size];
	}

	~CBuffer() {
		if (m_Buffer)
			delete[] m_Buffer;
	}

	char* Get() const { return m_Buffer; };
	uint64 Size() const { return m_Size; };

	bool IsValid() const { return m_Buffer != 0; };
};
