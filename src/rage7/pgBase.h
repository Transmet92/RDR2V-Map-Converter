#pragma once
#include <SkyCommons/SkyContainer.h>
#include <SkyCommons/SkyMap.h>
#include <SkyCommons/SkyTuple.h>
#include <zlib.h>
#include <iostream>
#include <memory>
#include "CThreadShared.h"


namespace rage
{
	class pgPtrList
	{
	public:
		SkyVector<uint64> m_PtrList;

	public:
		template<typename T>
		void Push(T pPtr) {
			if (pPtr && *(uint64*)pPtr)
				m_PtrList.push_back((uint64)pPtr);
		};

		template<typename T>
		void operator<<(T pPtr) { Push(pPtr); };

		template<typename T>
		void PushDirectly(T pPtr) {
			m_PtrList.push_back((uint64)pPtr);
		}
	};


	struct pgBlockDef {
		uint64 pBlock;
		uint32 size;
		pgPtrList ptrList;
	};

	class pgMap
	{
	public:
		// { block ptr, blockSize, blockPtrList }
		SkyVector<SkySharedPtr<pgBlockDef>> m_Map[2];

	public:
		bool DoesExist(uint64 pBlock, uint8 u)
		{
			for (const auto& i : m_Map[u]) {
				if (i->pBlock == pBlock)
					return true;
			}
			return false;
		}

		// Ajoute un block dans la cartographie et retourne une r�f�rence vers la liste des pointeurs du bloc
		pgPtrList* PushBlock(uint64 pBlock, uint32 blockSize, bool isGfx = false)
		{
			int indexMap = isGfx ? 1 : 0;
			if (!DoesExist(pBlock, indexMap)) {
				m_Map[indexMap].push_back(SkySharedPtr<pgBlockDef>(new pgBlockDef{ pBlock, blockSize, pgPtrList() }));
				return &(m_Map[indexMap].back()->ptrList);
			}
			return 0;
		}

		void PushString(const char* str, bool isGfx = false)
		{
			PushBlock((uint64)str, strlen(str) + 1, isGfx);
		}

		template<typename T>
		pgPtrList* Push(T* pBlock, bool isGfx = false) { return PushBlock((uint64)pBlock, sizeof(T), isGfx); }

		template<typename T>
		pgPtrList* PushWithoutVT(T* pBlock, bool isGfx = false) {
			return PushBlock(((uint64)pBlock) + sizeof(uint64), sizeof(T) - sizeof(uint64), isGfx);
		}
	};


	enum RSC_TYPES : uint32
	{
		eRSC_DRAWABLE = 165,
		eRSC_FRAGMENT = 162,
		eRSC_TXD = 13,
		eRSC_BOUNDS = 43,
		eRSC_META = 2, // PSO: ymap/ytyp etc..
	};

	struct RSC7Header
	{
		uint32 m_Magic;
		uint32 m_Version;
		uint32 m_SysFlags;
		uint32 m_GfxFlags;
	};


	struct TinyAllocator
	{
		HANDLE m_hHeap = 0;
		uint64 m_PhysicalAllocated = 0;
		uint64 m_AllocCount = 0;

		TinyAllocator()
		{
			m_hHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 1 * 1024 * 1024, 100 * 1024 * 1024);
			assert(m_hHeap != 0);
		}
		~TinyAllocator()
		{
			if (m_hHeap) {
				HeapDestroy(m_hHeap);
				m_hHeap = 0;
			}
			m_PhysicalAllocated = 0xDEADBEEFDEADBEEF;
		}

		void* Alloc(uint32 size)
		{
			void* allocated = HeapAlloc(m_hHeap, 0, size);
			if (allocated)
			{
				m_PhysicalAllocated += size;
				m_AllocCount++;
			}
			else
			{
				MessageBoxA(0, "ALLOCATION ERROR", "", 0);
				DWORD errId = GetLastError();
				Sleep(1);
			}

			return (void*)allocated;
		}

		template<typename T>
		T* Alloc() {
			T* allocated = (T*)new (Alloc(sizeof(T))) T;
			return allocated;
		}

		char* CloneStr(const SkyString& string) {
			auto toret = (char*)Alloc(string.size() + 1);
			strcpy(toret, string.c_str());
			return toret;
		}
	};


	typedef CBufferDyn<65536> pgBlockStream;



	// Classe d'encodage/décodage des drapeaux de pagination
	// des ressources pgBase
	class pgPageFlag
	{
	public:
		uint32 m_Flag;

		// union URscFlag {
		// 	uint8 shift : 4;
		// 	uint8 pgMul16 : 1;
		// 	uint8 pgMul8 : 1;
		// 	uint8 pgMul4 : 2;
		// 	uint8 pgMul2 : 4;
		// 	uint8 pgMul1 : 6;
		// 	uint8 pgDiv2 : 7;
		// 	uint8 pgDiv4 : 1;
		// 	uint8 pgDiv8 : 1;
		// 	uint8 pgDiv16 : 1;
		// 	uint8 version : 4;
		// } m_UFlag;

		// ** Neodynium
		// systemFlags |= (uint)((Version >> 4) & 0x0F) << 28;
		// systemFlags |= (uint)SystemPagesDiv16 << 27;
		// systemFlags |= (uint)SystemPagesDiv8 << 26;
		// systemFlags |= (uint)SystemPagesDiv4 << 25;
		// systemFlags |= (uint)SystemPagesDiv2 << 24;
		// systemFlags |= (uint)SystemPagesMul1 << 17;
		// systemFlags |= (uint)SystemPagesMul2 << 11;
		// systemFlags |= (uint)SystemPagesMul4 << 7;
		// systemFlags |= (uint)SystemPagesMul8 << 5;
		// systemFlags |= (uint)SystemPagesMul16 << 4;
		// systemFlags |= (uint)SystemPagesSizeShift;

	public:

		/*
		DUMP POUR LA VERSION 1290


		// Le flag est composé de 9 valeurs de 1 à 6 bits encodant un certain nombre de pages.
		// Les 4 bits de poids fort encodent une valeur décrivant le type de ressource
		// Les 4 bits de poids faible encodent un décalage binaire permettant de calculer la taille des pages (pageSize = baseSize << ((segmentFlag & 0xF) + 4))


		// effectue une allocation des segments de lecture de la resource
		__int64 __fastcall sub_141252540(__int64 pRscFlags, _BYTE *pReadSegmentsCntInfo_cand)
		{
		  unsigned int physFlag; // er9
		  unsigned int virtualFlag; // edx
		  unsigned int v6; // eax
		  __int64 TotalPagesCountInit; // rax

		  physFlag = *(_DWORD *)pRscFlags;
		
		  // calcul la somme de toutes les pages du segment physique
		  *pReadSegmentsCntInfo_cand = ((physFlag & 0x10) != 0)
									 + ((physFlag >> 5) & 3)
									 + ((physFlag >> 7) & 0xF)
									 + (HIBYTE(physFlag) & 1)
									 + ((physFlag & 0x2000000) != 0)
									 + ((physFlag & 0x4000000) != 0)
									 + ((physFlag & 0x8000000) != 0)
									 + ((*(_DWORD *)pRscFlags >> 11) & 0x3F)
									 + ((*(_DWORD *)pRscFlags >> 17) & 0x7F);
		  virtualFlag = *(_DWORD *)(pRscFlags + 4);
		
		  // calcul la somme de toutes les pages du segment virtuel
		  pReadSegmentsCntInfo_cand[1] = ((virtualFlag & 0x10) != 0)
									   + ((virtualFlag >> 5) & 3)
									   + ((virtualFlag >> 7) & 0xF)
									   + (HIBYTE(virtualFlag) & 1)
									   + ((virtualFlag & 0x2000000) != 0)
									   + ((virtualFlag & 0x4000000) != 0)
									   + ((virtualFlag & 0x8000000) != 0)
									   + ((virtualFlag >> 11) & 0x3F)
									   + ((virtualFlag >> 17) & 0x7F);
		  pReadSegmentsCntInfo_cand[2] = 0;
		  v6 = InitReadRequestForSegmentFromFlag(
				 *(_DWORD *)pRscFlags,
				 8192i64,
				 (__int64)pReadSegmentsCntInfo_cand,
				 0,
				 0x50000000i64);                        // segment physique
		  TotalPagesCountInit = InitReadRequestForSegmentFromFlag(
								  *(_DWORD *)(pRscFlags + 4),
								  8192i64,
								  (__int64)pReadSegmentsCntInfo_cand,
								  v6,
								  0x60000000i64);       // segment virtuel
		  *((_QWORD *)pReadSegmentsCntInfo_cand + 1) = 0i64;
		  *((_DWORD *)pReadSegmentsCntInfo_cand + 773) = 0;
		  *((_DWORD *)pReadSegmentsCntInfo_cand + 772) = 0;
		  return TotalPagesCountInit;
		}



		// calcul la somme
		__int64 __fastcall InitReadReqForSegmentFromFlag(unsigned int segmentFlag, __int64 baseSize, __int64 pReadSegment, unsigned int pagesCount, __int64 a5)
		{
		  __int64 *pageCountArr; // r8
		  __int64 v8; // rdi
		  unsigned __int64 pageSize; // r11
		  __int64 v10; // rdx
		  __int64 v11; // rax
		  __int64 v12; // rcx
		  __int64 pageCountArr0[11]; // [rsp+0h] [rbp-58h] BYREF


		  pageCountArr = pageCountArr0;
		  pageCountArr0[0] = (segmentFlag >> 4) & 1;
		  pageCountArr0[8] = (segmentFlag >> 27) & 1;
		  v8 = 9i64;
		  pageCountArr0[1] = (segmentFlag >> 5) & 3;
		  pageCountArr0[2] = (segmentFlag >> 7) & 0xF;
		  pageCountArr0[3] = (segmentFlag >> 11) & 0x3F;
		  pageCountArr0[4] = (segmentFlag >> 17) & 0x7F;
		  pageCountArr0[5] = HIBYTE(segmentFlag) & 1;
		  pageCountArr0[6] = (segmentFlag >> 25) & 1;
		  pageSize = baseSize << ((segmentFlag & 0xF) + 4);
		  pageCountArr0[7] = ((unsigned __int64)segmentFlag >> 26) & 1;
		  do
		  {
			v10 = *pageCountArr;
			if ( *pageCountArr )
			{
			  do
			  {
				v11 = pagesCount++;
				v12 = 3 * v11;
				*(_QWORD *)(pReadSegment + 8 * v12 + 24) = 0i64;
				*(_QWORD *)(pReadSegment + 8 * v12 + 16) = a5;// pointeur (de l'espace virtuel rage) vers début de page
				a5 += pageSize;
				*(_QWORD *)(pReadSegment + 8 * v12 + 32) = pageSize;// pointeur (de l'espace virtuel rage) vers fin de page
				--v10;
			  }
			  while ( v10 );
			  *pageCountArr = 0i64;
			}
			pageSize >>= 1;                             // réduire à la puissance de deux inferieur
			++pageCountArr;
			--v8;
		  }
		  while ( v8 );
		  return pagesCount;
		}
		*/

		void BuildPageFlags(uint32* pageCounts, uint32 baseShift)
		{
			m_Flag = baseShift & 0x0F;
			m_Flag += (pageCounts[0] & 0x01) << 4;
			m_Flag += (pageCounts[1] & 0x03) << 5;
			m_Flag += (pageCounts[2] & 0x0F) << 7;
			m_Flag += (pageCounts[3] & 0x3F) << 11;
			m_Flag += (pageCounts[4] & 0x7F) << 17;
			m_Flag += (pageCounts[5] & 0x01) << 24;
			m_Flag += (pageCounts[6] & 0x01) << 25;
			m_Flag += (pageCounts[7] & 0x01) << 26;
			m_Flag += (pageCounts[8] & 0x01) << 27;
		}

		uint32 GetSize() const
		{
			uint32 baseSize = 512 << (int)pgShift;
			uint32 pg0 = ((m_Flag >> 27) & 0x01) << 0;
			uint32 pg1 = ((m_Flag >> 26) & 0x01) << 1;
			uint32 pg2 = ((m_Flag >> 25) & 0x01) << 2;
			uint32 pg3 = ((m_Flag >> 24) & 0x01) << 3;
			uint32 pg4 = ((m_Flag >> 17) & 0x7F) << 4;
			uint32 pg5 = ((m_Flag >> 11) & 0x3F) << 5;
			uint32 pg6 = ((m_Flag >> 7) & 0x0F) << 6;
			uint32 pg7 = ((m_Flag >> 5) & 0x03) << 7;
			uint32 pg8 = ((m_Flag >> 4) & 0x01) << 8;
			uint32 pgShift = ((m_Flag >> 0) & 0x0F);
			return (pg0 + pg1 + pg2 + pg3 + pg4 + pg5 + pg6 + pg7 + pg8) * baseSize;
		}

		uint32 GetTypeVal() const { return (m_Flag >> 28) & 0x0F; };
		uint32 GetBaseShift() const { return (m_Flag & 0x0F); };
		uint32 GetBaseSize() const { return (512 << (int)GetBaseShift()); };

		void GetPagesCount(uint32* out) const
		{
			out[0] = ((m_Flag >> 4) & 0x01);
			out[1] = ((m_Flag >> 5) & 0x03);
			out[2] = ((m_Flag >> 7) & 0x0F);
			out[3] = ((m_Flag >> 11) & 0x3F);
			out[4] = ((m_Flag >> 17) & 0x7F);
			out[5] = ((m_Flag >> 24) & 0x01);
			out[6] = ((m_Flag >> 25) & 0x01);
			out[7] = ((m_Flag >> 26) & 0x01);
			out[8] = ((m_Flag >> 27) & 0x01);
		}

		uint32 GetCount() const
		{
			uint32 counts[9];
			GetPagesCount(counts);

			uint32 toret = 0;
			for (int i = 0; i < ARRAYSIZE(counts); i++)
				toret += counts[i];

			return toret;
		}
	};

	struct pgBinSerialized
	{
		pgBlockStream m_SysStream;
		pgBlockStream m_GfxStream;

		pgPageFlag m_SysFlag;
		pgPageFlag m_GfxFlag;
	};


	struct pgPagesInfo {
		uint32 m_Unk0;
		uint32 m_Unk1;
		uint8 m_SysPagesCount;
		uint8 m_GfxPagesCount;
		uint16 m_Unk2;
		uint32 m_Unk3;
		uint32 m_Unk4;
	};

	class pgBaseInfoPages
	{
	protected:
		// uint64 m_VT = 0x140571168;
		pgPagesInfo* m_pPageInfo = 0;

	public:
		void PushPageInfo(pgPtrList* pList) {
			pList->PushDirectly(&m_pPageInfo);
		}

		void SetPageInfoPtr(pgPagesInfo* pPageInf) {
			m_pPageInfo = pPageInf;
		}
	};


	class pgFakeBaseInfoPages
	{
	public:
		uint64 m_FakePgPtr = 0;

	public:
		void PushPageInfo(pgPtrList* pList) {}
	};


	// CLASSE DE BASE QU'HERITE CHAQUE RESOURCE RSC7 (ydr, yft, ytd, ymap etc...)
	// 
	// Elle permet la gestion de la pagination et particuli�rement du mapping de l'arbre
	// des r�f�rences de la classe (tous les pointeurs et les structures point�s par recursion).
	// Elle fournit des m�canismes de sauvegarde de classe permettant d'exporter l'instance
	// d'une classe rage::pgBase dans un fichier (RSC7: ydr, yft, ymap etc...)
	template<bool UsePageInfo = true>
	class pgBase : public std::conditional<UsePageInfo, pgBaseInfoPages, pgFakeBaseInfoPages>::type
	{
	public:
		virtual void Map(pgMap& map) = 0; // fonction de mapping recursive de la resource h�riti�re

		bool Fragment(pgBinSerialized& output)
		{
			pgMap map;
			Map(map);

			uint8 pgPagesInfoBuff[sizeof(pgPagesInfo) + (16 * 256)];
			ZeroMemory(pgPagesInfoBuff, sizeof(pgPagesInfoBuff));
			map.PushBlock((uint64)&pgPagesInfoBuff, sizeof(pgPagesInfoBuff));
			((pgBaseInfoPages*)this)->SetPageInfoPtr((pgPagesInfo*)&pgPagesInfoBuff);

			uint32 pageInfoBlockPosition = 0;

			// Map de transposition des blocs : original addr, {final position, physOrVirt}
			SkyMap<uint64, SkyTuple<uint32, uint8>> transposeMap[2];

			// Vecteur de transpointage : orig ptr val, new ptr pos, pointToPhysOrVirt, [pBlockContainer, sizeBlockContainer, blockContainerNewAddr]
			SkyVector<SkyTuple<uint64, uint32, uint8, uint64, uint32, uint32>> transpoint[2];

			pgPageFlag pagesFlags[2];
			for (int u = 0; u < 2; u++)
			{
				bool isVirt = u == 1;

				// CALCUL LA TAILLE DE PAGE MINIMALE PAR LE PLUS GROS BLOC
				uint32 biggestBlock = 0;
				uint32 totalBlocksSize = 0;
				for (const auto& i : map.m_Map[u]) {
					uint32 iMax = i->size;
					totalBlocksSize += iMax;
					totalBlocksSize += (16 - (totalBlocksSize % 16));
					if (iMax > biggestBlock) {
						biggestBlock = iMax;
					}
				}

				int pgMultSz = 1; // flag facteur de taille de page
				int pgSize = 8192; // (taille de base RSC: 8192)

				// arrondir � la taille de page minimum sup�rieur
				for (; pgSize < biggestBlock;)
					pgSize *= 2;

				for (;;)
				{
					transposeMap[u].clear();
					transpoint[u].clear();

					auto& uStream = isVirt ? output.m_GfxStream : output.m_SysStream;
					uStream.Release();
					uStream.Resize(5 * 1024 * 1024);

					int pageCount = 1;

					int pageCountId = 0;
					uint32 pageCounts[9];
					ZeroMemory(pageCounts, sizeof(pageCounts));
#undef max
#undef min
					int targetPageSize = std::max(65536 * pgMultSz, pgSize >> (!isVirt ? 5 : 2));
					int minPageSize = std::max(512 * pgMultSz, std::min(targetPageSize, pgSize) >> 4);

					uint32 baseShift = 0;
					uint32 baseSize = 512;
					for (; baseSize < minPageSize;) {
						baseShift++;
						baseSize *= 2;
						if (baseShift >= 0xF)
							break;
					}

					int baseSizeMax = baseSize << 8;
					int baseSizeMaxTest = pgSize;
					for (; baseSizeMaxTest < baseSizeMax;)
					{
						pageCountId++;
						baseSizeMaxTest *= 2;
					}
					pageCounts[pageCountId] = 1;

					uint32 currentPageStart = 0;
					uint32 currentPageSize = pgSize;
					uint32 currentTotalRest = totalBlocksSize;
					uint32 currentPageRest = pgSize;
					uint32 currentPosition = 0;

					SkyVector<pgBlockDef> BlocksStack;
					BlocksStack.reserve(map.m_Map[u].size());
					for (const auto& i : map.m_Map[u])
						BlocksStack.push_back(*i);

					for (;;)
					{
						for (auto i = BlocksStack.begin(); i != BlocksStack.end();)
						{
							bool isRootBlock = !isVirt && (currentPosition == 0);

							if (isRootBlock || i->size <= currentPageRest)
							{
								uint32 blockPos = currentPosition;
								{
									auto& buf = uStream;

									buf.Insert((char*)i->pBlock, i->size);
									currentPosition += i->size;

									uint32 backPos = currentPosition;

									// aligner sur 16 bytes
									if (currentPosition % 16 != 0)
										currentPosition += 16 - (currentPosition % 16);

									uint32 paddCount = currentPosition - backPos;
									buf.InsertRepeat((uint8)0x00, paddCount);
								}
								uint32 writted = (currentPosition - blockPos);

								transposeMap[u][i->pBlock] = std::make_tuple(blockPos, u);

								currentPageRest -= writted;
								currentTotalRest -= writted;

								if (i->pBlock == (uint64)&pgPagesInfoBuff)
									pageInfoBlockPosition = blockPos;

								for (const auto p : i->ptrList.m_PtrList)
								{
									uint64 transposedPtrPos = blockPos + (((uint64)p) - i->pBlock);

									uint64 originalPtrVal = *(uint64*)p;
									transpoint[u].push_back(std::make_tuple(originalPtrVal, transposedPtrPos, u, i->pBlock, i->size, blockPos));
								}

								i = BlocksStack.erase(i);
								continue;
							}

							i++;
						}


						uStream.InsertRepeat((uint8)0x00, currentPageRest);
						currentPageRest = 0;

						// ALLOCATE NEW PAGE
						if (BlocksStack.size() > 0)
						{
							currentPageStart += currentPageSize;
							currentPosition = currentPageStart;

							uint32 biggestBlockLength = 0;
							for (const auto& i : BlocksStack) {
								if (i.size > biggestBlockLength)
									biggestBlockLength = i.size;
							}

							for (; biggestBlockLength <= (currentPageSize >> 1);)
							{
								if (currentPageSize <= minPageSize)
									break;

								if (pageCountId >= 8)
									break;

								if ((currentPageSize <= targetPageSize) &&
									(currentTotalRest >= (currentPageSize - minPageSize)))
									break;

								currentPageSize >>= 1;
								pageCountId++;
							}

							currentPageRest = currentPageSize;
							pageCounts[pageCountId]++;
							pageCount++;
						}
						else
							break;
					}

					pagesFlags[u].BuildPageFlags(pageCounts, baseShift);
					if ((pageCount == pagesFlags[u].GetCount()) && (pagesFlags[u].GetSize() >= currentPosition))
						break;

					pgSize *= 2;
					pgMultSz *= 2;
				}
			}

			output.m_SysFlag = pagesFlags[0];
			output.m_GfxFlag = pagesFlags[1];

			output.m_SysStream.m_Data[pageInfoBlockPosition + 8] = pagesFlags[0].GetCount();
			output.m_SysStream.m_Data[pageInfoBlockPosition + 9] = pagesFlags[1].GetCount();



			// POSTOP: TRANSPOINTAGE DES REFERENCES
			for (int ty = 0; ty < 2; ty++)
			{
				for (const auto& i : transpoint[ty])
				{
					uint64 iOrigPtrVal = std::get<0>(i);
					uint8 pointTo = std::get<2>(i);
					auto& str = (pointTo == 1) ? output.m_GfxStream : output.m_SysStream;

					uint8 pointFrom = 0;
					uint64 newPtrAddr = 0;
					bool ptrIsResolved = false;

					auto iMatch = transposeMap[0].find(iOrigPtrVal);
					if (iMatch == transposeMap[0].end())
					{
						auto iMatch1 = transposeMap[1].find(iOrigPtrVal);
						if (iMatch1 != transposeMap[1].end())
						{
							pointFrom = std::get<1>(iMatch1->second);
							newPtrAddr = (uint64)std::get<0>(iMatch1->second);
							ptrIsResolved = true;
						}
					}
					else
					{
						pointFrom = std::get<1>(iMatch->second);
						newPtrAddr = (uint64)std::get<0>(iMatch->second);
						ptrIsResolved = true;
					}

					// essaye de résoudre le pointer du bloc local
					if (!ptrIsResolved)
					{
						pointFrom = 0;

						uint64 pBlockContainer = std::get<3>(i);
						uint32 blockContainerSize = std::get<4>(i);

						if (iOrigPtrVal >= pBlockContainer &&
							iOrigPtrVal <= (pBlockContainer + blockContainerSize - 7))
						{
							int64 offsetInBlock = iOrigPtrVal - pBlockContainer;
							newPtrAddr = std::get<5>(i) + offsetInBlock;
							ptrIsResolved = true;
						}
					}

					// essaye de résoudre le pointeur du bloc global
					if (ptrIsResolved)
					{
						uint64 newPtrVal = (pointFrom == 1 ? 0x60000000 : 0x50000000) | newPtrAddr;
						uint32 iNewPtrPos = std::get<1>(i);

						for (uint32 t = 0; t < sizeof(uint64); t++)
							str.m_Data[iNewPtrPos + t] = ((char*)&newPtrVal)[t];
					}
				}
			}

			return true;
		}


		int BuildRSC7(bool freeze, RSC_TYPES rscType, SkyVector<char>& out, SkyVector<char>* outGfx = 0)
		{
			pgBinSerialized build = {};
			if (Fragment(build))
			{
				uint32 version = (uint32)rscType;
				uint32 systemFlags = build.m_SysFlag.m_Flag + (((version >> 4) & 0xF) << 28);
				uint32 graphicsFlags = build.m_GfxFlag.m_Flag + (((version >> 0) & 0xF) << 28);

				out.reserve(2048);

				// magic
				const char RSC7_Magic[] = { 'R', 'S', 'C', '7' };
				out.insert(out.end(), RSC7_Magic, RSC7_Magic + sizeof(RSC7_Magic));
				out.insert(out.end(), (char*)&version, ((char*)&version) + sizeof(version));
				out.insert(out.end(), (char*)&systemFlags, ((char*)&systemFlags) + sizeof(systemFlags));
				out.insert(out.end(), (char*)&graphicsFlags, ((char*)&graphicsFlags) + sizeof(graphicsFlags));

				uint8* transBuf = (uint8*)g_LocalThreadBuffer;
				uint8* transBuf2 = (uint8*)g_LocalThreadBuffer2;


				{
					uint32 sysSize = build.m_SysStream.GetSize();
					uint32 gfxSize = build.m_GfxStream.GetSize();
					memcpy(transBuf2, build.m_SysStream.m_Data.data(), sysSize);

					if (gfxSize)
						memcpy(transBuf2 + sysSize, build.m_GfxStream.m_Data.data(), gfxSize);

					uint32 totalSize = sysSize + gfxSize;

					// ZeroMemory(transBuf, LOCAL_THREAD_BUFFER_SIZE);

					uint32 compressedSize = 0;
					z_stream stream;
					stream.next_in = (BYTE*)transBuf2;
					stream.avail_in = (uint32_t)totalSize;
					stream.next_out = (Bytef*)transBuf;
					stream.avail_out = (uint32_t)LOCAL_THREAD_BUFFER_SIZE;
					stream.zalloc = 0;
					stream.zfree = 0;
					int err = deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
					if (err != Z_OK)
						return -1;

					err = deflate(&stream, Z_FINISH);
					if (err == Z_OK || err == Z_STREAM_END)
					{
					}
					else
						return -1;

					err = deflateEnd(&stream);
					if (err != Z_OK)
						return -1;

					compressedSize = LOCAL_THREAD_BUFFER_SIZE - stream.avail_out;

					out.insert(out.end(), transBuf, transBuf + compressedSize + 16);


					// if (freeze)
					// {
					// 	HANDLE hOut = CreateFileA("C:\\Users\\Unknown8192\\Desktop\\Nouveau dossier (5)\\test.out.ydr",
					// 		GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
					// 	DWORD nullll = 0;
					// 	WriteFile(hOut, out.data(), out.size(), &nullll, 0);
					// 	FlushFileBuffers(hOut);
					// 	CloseHandle(hOut);
					// 
					// 	for (;;)
					// 	{
					// 		printf("RSC7 PTR: %p  ---  TransBuf: %p  --  CompressedSz: %u\n", out.data(), transBuf, compressedSize);
					// 		printf("TransBuf2: %p  --  Size: %u\n", transBuf2, totalSize);
					// 		Sleep(10);
					// 	}
					// }
				}

				return out.size();
			}

			return -1;
		}
	};
}
