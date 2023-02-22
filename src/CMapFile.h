#pragma once
#include <Windows.h>
#include "SkyCommons/types.h"


static int64 GetFileSz(HANDLE hFile)
{
	LARGE_INTEGER size = { 0, 0 };
	GetFileSizeEx(hFile, &size);
	return size.QuadPart;
}

class CMapFile
{
public:
	uint8* m_Map = 0;
	HANDLE m_hMap = INVALID_HANDLE_VALUE;
	HANDLE m_hFile = INVALID_HANDLE_VALUE;
	int64 m_Size;

public:
	CMapFile() {};
	~CMapFile() { Close(); };


	bool CreateMap(const wchar_t* file, bool write, uint64 size)
	{
		m_hFile = CreateFileW(file, GENERIC_READ | (write ? GENERIC_WRITE : 0), 0, 0, CREATE_ALWAYS, 0, 0);
		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER largeSz = {};
			largeSz.QuadPart = size;
			SetFilePointerEx(m_hFile, largeSz, 0, FILE_BEGIN);
			SetEndOfFile(m_hFile);

			m_Size = size;
			if (m_Size != 0)
			{
				m_hMap = CreateFileMappingW(m_hFile, 0, (write ? PAGE_READWRITE : PAGE_READONLY), 0, 0, 0);
				if (m_hMap != INVALID_HANDLE_VALUE)
				{
					m_Map = (uint8*)MapViewOfFile(m_hMap, FILE_MAP_READ | (write ? FILE_MAP_WRITE : 0), 0, 0, 0);
					if (m_Map == 0)
					{
						CloseHandle(m_hMap);
						CloseHandle(m_hFile);
						return false;
					}
				}
				else {
					CloseHandle(m_hFile);
					return false;
				}
			}
			else {
				CloseHandle(m_hFile);
				return false;
			}
		}
		else
			return false;

		return true;
	}

	bool OpenMap(const wchar_t* file, bool write)
	{
		m_hFile = CreateFileW(file, GENERIC_READ | (write ? GENERIC_WRITE : 0), 0, 0, OPEN_ALWAYS, 0, 0);
		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			m_Size = GetFileSz(m_hFile);
			if (m_Size != 0)
			{
				m_hMap = CreateFileMappingW(m_hFile, 0, (write ? PAGE_READWRITE : PAGE_READONLY), 0, 0, 0);
				if (m_hMap != INVALID_HANDLE_VALUE)
				{
					m_Map = (uint8*)MapViewOfFile(m_hMap, FILE_MAP_READ | (write ? FILE_MAP_WRITE : 0), 0, 0, 0);
					if (m_Map == 0)
					{
						CloseHandle(m_hMap);
						CloseHandle(m_hFile);
						return false;
					}
				}
				else {
					CloseHandle(m_hFile);
					return false;
				}
			}
			else {
				CloseHandle(m_hFile);
				return false;
			}
		}
		else
			return false;

		return true;
	}


	uint8* Get() const { return m_Map; };
	uint64 Size() const { return m_Size; };

	void Close()
	{
		if (m_Map) {
			UnmapViewOfFile((LPCVOID*)m_Map);
			m_Map = 0;
		}

		if (m_hMap != INVALID_HANDLE_VALUE) {
			CloseHandle(m_hMap);
			m_hMap = INVALID_HANDLE_VALUE;
		}

		if (m_hFile != INVALID_HANDLE_VALUE) {
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
	}
};