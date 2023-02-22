#include "CConversionWorker.h"
#include "IOFile.h"
#include "rmcDrawable_rdr_to_v.h"
#include "rage_fragType.h"
#include "grcTexture_rdr_to_v.h"
#include "CThreadShared.h"
#include "CMapBuilder.h"
#include "CSgaSectorsInfo.h"
#include "ConversionsConstParameters.h"


using namespace ::rage;
using namespace rage;


enum FileVersion : int {
	eFV_DRAWABLE = 133,
	eFV_TEXDICT = 10,
	eFV_FRAGMENT = 138, // xft
	eFV_FRAGMENT2 = 1, // xfd
	eFV_META_MAP = 134,
	eFW_BOUNDS_DICT = 36,
	eFV_PROC_MAP = 18, // grass batch instancefile
	eFV_SPD_XSP = 116, // tree instance placement map
};



enum PathType {
	PATH_PROPS,
	PATH_GENERAL_DRAWABLE,
};
SkyString BuildPath(const SkyString& path, PathType pathType)
{
	SkyString out = path;
	if (pathType == PATH_GENERAL_DRAWABLE)
		out += "General\\";

	else if (pathType == PATH_PROPS)
		out += "Props\\";

	return out;
}


std::atomic<uint64> g_FilesProcessed = { 0 };

DWORD CConversionWorker::Worker()
{
	srand(time(0));

	CThreadShared& threadShared = *ThreadShared;
	g_LocalThreadBuffer = new char[LOCAL_THREAD_BUFFER_SIZE];
	g_LocalThreadBuffer2 = new char[LOCAL_THREAD_BUFFER_SIZE];

	for (const auto& f : m_Files)
	{
		const char* pF = f.c_str();
		if (!strstr(pF, ".cpu")) {
			g_FilesProcessed++;
			continue;
		}

		// if (!strstr(f.c_str(), "0x758B178C") &&
		// 	!strstr(f.c_str(), "0xCDF88D98"))
		//  	continue;

		char* pRSCClue = (char*)strstr(pF, "\\RSC-");
		if (pRSCClue)
		{
			char gpuFilepath[MAX_PATH + 512];
			lstrcpyA(gpuFilepath, pF);
			lstrcpyA(strstr(gpuFilepath, ".cpu"), ".gpu");
			pRSCClue += 5;
			int fileVersion = atoi(pRSCClue);


			if (fileVersion == eFV_DRAWABLE ||fileVersion == eFV_FRAGMENT)
			{
				std::shared_ptr<char[]> sysBuff;
				std::shared_ptr<char[]> gfxBuff;
				uint32 sysSize = 0;
				uint32 gfxSize = 0;
				bool sysLoaded = LoadFile(pF, sysSize, sysBuff);
				bool gfxLoaded = LoadFile(gpuFilepath, gfxSize, gfxBuff);

				char* filenamePath = (char*)(pF + strlen(pF) - 1);
				for (; *filenamePath != '\\'; filenamePath--) {};
				filenamePath++;
				
				using namespace rdr::rage;

				// DRAWABLE
				if (fileVersion == eFV_DRAWABLE)
				{
					SkyPrint->print("Load rdr::rage::rmcDrawable (%s)\n", pF);

					// DRAWABLE PROCESSING
					Drawables drawables;
					int drawablesCount = rdr::rage::LoadDrawable(drawables, pF, sysBuff.get(), sysSize, gfxBuff.get(), gfxSize);

					if (true && drawablesCount == 0)
						SkyPrint->print("[rdr::rmcDrawable] No drawable detected in : %s  (magic: 0x%p)\n", pF, *(uint32*)sysBuff.get());

					for (auto& i : drawables.m_Drawables)
					{
						atSingleton<CMapBuilder>::Get()->AddDrawable(i, drawables.m_EmbedTex, MAP_SCALE, MAP_OFFSET, filenamePath);
					}
				}


				// FRAGMENT DRAWABLE
				else if (fileVersion == eFV_FRAGMENT)
				{
					SkyPrint->print("Load rdr::rage::fragDrawable (%s)\n", pF);

					// DRAWABLE PROCESSING
					Drawables drawables;
					int drawablesCount = rdr::rage::LoadFragDrawable(drawables, pF, sysBuff.get(), sysSize, gfxBuff.get(), gfxSize);

					if (true && drawablesCount == 0)
						SkyPrint->print("[rdr::fragDrawable] No drawable detected in : %s  (magic: 0x%p)\n", pF, *(uint32*)sysBuff.get());

					for (auto& i : drawables.m_Drawables)
					{
						atSingleton<CMapBuilder>::Get()->AddDrawable(i, drawables.m_EmbedTex, MAP_SCALE, MAP_OFFSET, filenamePath);
					}
				}


				// TEXTURE DICTIONNARY PROCESSING
				else if (fileVersion == eFV_TEXDICT)
				{
					// outList << pF << std::endl;
					// rdr::rage::TXD thisTxd = rdr::rage::LoadTextureDictionnary(
					// 	pF, sysBuff.get(), sysSize, gfxBuff.get(), gfxSize
					// );
					// for (const auto& i : thisTxd)
					// {
					// 	auto iHash = (uint8*)&i.m_Hash;
					// 
					// 	char bufOut[512];
					// 	sprintf_s(bufOut, "0x%02hhX%02hhX%02hhX%02hhX:%s",
					// 		iHash[0], iHash[1], iHash[2], iHash[3], i.m_Texname.c_str());
					// 
					// 	outList << bufOut << std::endl;
					// 	printf("%s\n", bufOut);
					// }
					// outList << "\n" << std::endl;
					// printf("\n");
				}
			}



			else if (fileVersion == eFV_META_MAP)
			{
				SkySharedPtr<char[]> sysBuff;
				uint32 sysSize = 0;
				bool sysLoaded = LoadFile(pF, sysSize, sysBuff);

				if (sysLoaded)
				{
					SkyPrint->print("Load rdr::rage::CSgaSector of props (%s)\n", pF);
					atSingleton<CMapDataStoreMgr>::GetRef().LoadMetaMap(sysBuff, sysSize, pF);
				}
			}

			else if (fileVersion == eFV_PROC_MAP)
			{
				std::shared_ptr<char[]> sysBuff;
				std::shared_ptr<char[]> gfxBuff;
				uint32 sysSize = 0;
				uint32 gfxSize = 0;
				bool sysLoaded = LoadFile(pF, sysSize, sysBuff);
				bool gfxLoaded = LoadFile(gpuFilepath, gfxSize, gfxBuff);

				if (sysLoaded && gfxLoaded)
				{
					SkyPrint->print("Load rdr::rage::CSgaSector of grass (%s)\n", gpuFilepath);
					LoadProcMap(pF, sysBuff, sysSize, gfxBuff, gfxSize);
				}
			}

			else if (fileVersion == eFV_SPD_XSP)
			{
				SkySharedPtr<char[]> sysBuff;
				uint32 sysSize = 0;
				bool sysLoaded = LoadFile(pF, sysSize, sysBuff);

				if (sysLoaded)
				{
					SkyPrint->print("Load rdr::rage::fiTokenizer of tree (%s)\n", pF);
					LoadSpdMap(sysBuff, sysSize, pF);
				}
			}
		}


		g_FilesProcessed++;
	}


	delete[] g_LocalThreadBuffer;
	delete[] g_LocalThreadBuffer2;

	return 0;
}
