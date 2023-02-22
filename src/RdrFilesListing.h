#pragma once
#include "IOFile.h"
#include "SkyCommons/SkyFile.h"

#include <codecvt>
#include <locale>
#include <fstream>


void EnumRdrFiles(SkyVector<SkyString>& files)
{
	std::shared_ptr<char[]> listBuff;
	uint32 listSize = 0;
	bool sysLoaded = LoadFile("files_list_rdr.txt", listSize, listBuff);
	if (sysLoaded)
	{
		char* pList = listBuff.get();
		char* endList = pList + listSize - 2;
		char* currentLine = pList;
		for (; pList < endList; pList++)
		{
			if (pList[0] == 0x0D && pList[1] == 0x0A)
			{
				pList[0] = 0x00;
				pList += 2;
				files.push_back(currentLine);
				currentLine = pList;
			}
		}
	}
	else
	{
		std::ofstream out("files_list_rdr.txt");

		std::vector<CSkyFile::ElementOfDir> elems;
		CSkyFile::EnumDirectories(L"C:\\RDR1_to_V\\UNPACK", elems, true);

		using convert_type = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_type, wchar_t> converter;

		for (const auto& i : elems)
		{
			if (i.m_type == CSkyFile::ElementType::FILE)
			{
				SkyString iFile8 = converter.to_bytes(i.m_path);

				if (strstr(iFile8.c_str(), ".cpu")) {
					out << iFile8.c_str() << std::endl;
					files.push_back(iFile8);
				}
			}
		}
	}

}
