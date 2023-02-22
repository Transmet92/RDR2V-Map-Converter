#include "WorkRDR2V.h"
#include "RdrFilesListing.h"
#include "CConversionWorker.h"
#include "CTextureLinker.h"
#include "CMapBuilder.h"
// #include "fiPackFile.h"
#include "fiPackFile7.h"

#include <algorithm>
#include <random>
#include <D3DX10Math.h>
#include "ConversionsConstParameters.h"


// permet d'effectuer un filtrage post-énumeration
static bool FilterFile(const SkyString& i)
{
	// return strstr(i.c_str(), "joshuatree") || strstr(i.c_str(), ".xsi.");// || strstr(i.c_str(), "bearclaw.xsi");
	// return strstr(i.c_str(), "RSC-138") || strstr(i.c_str(), "RSC-134");
	// return strstr(i.c_str(), "RSC-116") ? true : false;
	// return strstr(i.c_str(), "RSC-133__") != 0 || strstr(i.c_str(), "RSC-134__") != 0;
	// return strstr(i.c_str(), "RSC-18") ? true : false;
	return true;
}



int main()
{
	SetConsoleTitleA("RDR1 Xbox 360  to  GTA V PC - map converter  [32 threads]   - Transmet");
	srand(time(0));
	
	atSingleton<CMapBuilder>::Get()->Init((uint64)16 * (uint64)1024 * (uint64)1024 * (uint64)1024);
	atSingleton<CMapBuilder>::Get()->PrebuildMapArea({ SizeAreas, SizeAreas, SizeAreas });

	atSingleton<CTextureLinker>::Get()->LoadFromFile("C:\\RDR1_to_V\\UNPACK\\list_texdict.txt", false);
	atSingleton<CTextureLinker>::Get()->LoadFromFile("C:\\RDR1_to_V\\UNPACK\\list_dwd_texdict.txt", true);

	// atSingleton<CMapBuilder>::Get()->BuildMap("C:\\Users\\Unknown8192\\Desktop\\LasVenturas\\");

	HANDLE ThreadsList[CONV_DRAWABLE_THREAD_COUNT];

	uint32 FilesCount = 0;
	{
		SkyVector<SkyString> Files;
		{
			SkyVector<SkyString> FilesTmp;
			EnumRdrFiles(FilesTmp);

			printf("Randomize files list to optimize multi-threading...\n");

			auto rng = std::default_random_engine{};
			std::shuffle(std::begin(FilesTmp), std::end(FilesTmp), rng);

			int it = 0;

			// filtrage post-enum
			for (const auto &i : FilesTmp)
			{
				if (FilterFile(i))
					Files.push_back(i);

				// if (it++ > (FilesTmp.size() / 32))
				// 	break;
			}
		}


		FilesCount = Files.size();
		uint32 FilesPerThread = FilesCount / CONV_DRAWABLE_THREAD_COUNT;
		uint32 FilesRest = FilesCount % CONV_DRAWABLE_THREAD_COUNT;

		CConversionWorker* workers = new CConversionWorker[CONV_DRAWABLE_THREAD_COUNT];

		for (int i = 0, iF = 0; i < CONV_DRAWABLE_THREAD_COUNT; i++)
		{
			uint32 iFilesToProcessCount = FilesPerThread;
			if (i + 1 == CONV_DRAWABLE_THREAD_COUNT)
				iFilesToProcessCount += FilesRest;

			uint32 limitCount = iF + iFilesToProcessCount;
			for (; iF < limitCount; iF++)
				workers[i].m_Files.push_back(Files[iF]);

			// démarre un worker sur un thread de 10 MiB de pile
			ThreadsList[i] = CreateThread(0, 10 * 1024 * 1024, CConversionWorker::WorkerAdapter, &workers[i], 0, 0);
		}
	}


	for (;;)
	{
		if (false) {
			printf("Files process %u/%u (%.2f)\n", g_FilesProcessed.load(), FilesCount,
				((float)g_FilesProcessed.load() / (float)FilesCount) * 100.f);
		}

		atSingleton<CTextureLinker>::Get()->DoWork();
		
		DWORD res = WaitForMultipleObjects(CONV_DRAWABLE_THREAD_COUNT, ThreadsList, TRUE, 1);
		if ((res >= WAIT_OBJECT_0) && (res < (WAIT_OBJECT_0 + CONV_DRAWABLE_THREAD_COUNT)))
		{
			if (true)
			{
				printf("\n\nBuilding GTA V RPF...");
				atSingleton<CMapBuilder>::Get()->BuildMap("C:\\RDR1_to_V\\");
			}

			printf("\n\nCONVERSION FINISHED!\n");
			Sleep(100000000);
		}

		Sleep(200);
	}

	return 0;
}
