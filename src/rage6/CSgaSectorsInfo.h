#pragma once
#include "rage_fwrap.h"
#include "SkyCommons/SkyCommons.h"
#include "SkyCommons/SkyMath.h"
#include <unordered_map>
#include <D3DX10Math.h>

namespace rdr
{
	namespace rage
	{
		// VTables: A40D7200
		class CEntityDef
		{
		public:
			uint32 m_VTable;
			Vector3c m_Unk0; // -1,0,0
			Vector3c m_Unk1; // -9999, -9999, -9999
			uint32 m_Unk2;
			uint16 m_Unk3;
			uint16 m_Unk4;
			uint32 m_Unk5;
			uint32 m_Unk6;
			uint32 m_Unk7;
			uint32 m_Unk8;
			uint32 m_Unk9;
			uint32 m_Unk10;
			uint32 m_Unk11;

			Vector4c m_Unk12;
			Vector4c m_Unk13;
			Vector4c m_Unk14;
			Vector4c m_Unk15;
			Vector4c m_Unk16;
			Vector4c m_Unk17;

			uint32 m_DrawableHash;
			uint32 m_Unk18;
			uint32 m_Unk19;
			uint32 m_Unk20;

			uint32 m_Unk21;
			uint32 m_Unk22;
			uint32 m_Unk23;
			uint32 m_Unk24;

			uint32 m_Unk25;
			uint32 m_Unk26;
			uint32 m_Unk27;
			uint32 m_Unk28;

			uint32 m_Unk29;
			uint32 m_Unk30;
			uint32 m_Unk31;
			uint32 m_Unk32;
		};



		// VTable: C4527000
		class CUnk0
		{
		public:
			uint32 m_VTable;
			uint32 m_Unk0;
			uint32 m_Unk1;
			uint32 m_Unk2;
			uint32 m_Unk3;
			uint32 m_Unk4;
			uint32 m_Unk5;
			uint32 m_Unk6;

			uint32 m_VTableCAND; // another class ?  FC4D7000
			ptr32<void> m_pUnk0;
			uint32 m_Unk7;
			ptr32<void> m_pUnk1;

			D3DXMATRIX m_UnkMatrix;

			uint32 m_VTableCAND2; // another class ?  FC4D6D00
			ptr32<void> m_pUnk2;
			ptr32<void> m_pUnk3;
			uint32 m_Unk8;
			uint32 m_Unk9;
			uint32 m_Unk10;
			ptr32<CEntityDef> m_pUnk4;
		};


		// utilisé pour les props (référence string)
		struct rage__EntityPlacement
		{
			Vector3c m_Pos;
			float m_Unk0;
			uint32 m_Unk1;
			uint32 m_Unk2;
			uint32 m_Unk3;
			uint32 m_Unk4;
			ptr32<char> m_pPropStrName;
			uint32 m_Unk5;
			uint16 m_Unk6;
			uint16 m_Unk7;
			uint32 m_Unk8;
		};

		struct EntityPlacement
		{
			Vector3c m_Pos;
			D3DXQUATERNION m_Rot;
			D3DXMATRIX m_Matrix;
			SkyString m_Filename;
			uint32 m_IndexInFile;
		};

		struct GrassMapEntry {
			SkySharedPtr<char[]> m_CPU;
			SkySharedPtr<char[]> m_GPU;
			uint32 m_szCPU;
			uint32 m_szGPU;
			SkyString m_Filename;
		};

		typedef SkyTuple<SkySharedPtr<char[]>, uint32, SkyString> MetaMapFile_t;
		struct HashCandidat_t {
			uint32 index;
			MetaMapFile_t file;
		};
		class CMapDataStoreMgr
		{
		public:
			CMTStore<MetaMapFile_t> m_MetaMapFiles;
			CMTStore<GrassMapEntry> m_GrassMapFiles;
			CMTStore<SkyTuple<SkySharedPtr<char[]>, uint32, SkyString>> m_SpdMapFiles;
			std::unordered_map<uint32, SkyVector<HashCandidat_t>> m_MetaMapHashTable;
			CSecure m_MutexHashTable;

		public:
			/*
				Effectue une recherche par hash brute et par detection des strings
				hashé dans les meta map.
				Analyse essentiellement heuristique.
			*/
			SkyVector<EntityPlacement> GetEntityPlacements(uint32 hash)
			{
				SkyVector<EntityPlacement> toret;

				if (false)
				{
					auto match = m_MetaMapHashTable.find(hash);
					if (match != m_MetaMapHashTable.end())
					{
						auto& candidats = match->second;

						// 1er étage d'analyse :  recherche par hashtable de strings précalculé
						for (const auto& c : candidats)
						{
							char* pData = std::get<0>(c.file).get();
							uint32 iSize = std::get<1>(c.file);

							// recherche des références de pointeur dans le fichier
							uint32 ptrToFind = InvEnd(c.index) | 0x50;
							for (int p = 0; p < iSize - 4; p += 4)
							{
								if (*(uint32*)(pData + p) == ptrToFind)
								{
									// vérifie le type de structure
									int indexStructBegin = (p - 0x20);
									if ((p - 0x20) > 0)
									{
										Vector3c pos = LittleEndian(*(const Vector3c*)(&pData[indexStructBegin]));
										if (pos.IsInBounds({ -16000.f, -16000.f, -16000.f }, { 16000.f, 16000.f, 16000.f }))
										{
											toret.push_back({
												{ pos.z, pos.x, pos.y },
												{ 0.f, 0.f, 0.f, 1.f },
												{},
												std::get<2>(c.file),
												(uint32)p
											});
										}
									}
								}
							}
						}
					}
				}

				// 2nd étage d'analyse: recherche du hash du prop et analyse de la structure
				m_MetaMapFiles.LockStore();
				for (const auto& i : m_MetaMapFiles)
				{
					char* pData = std::get<0>(i).get();
					uint32 iSize = std::get<1>(i);
					for (uint32 it = 16; it < iSize; it += 4)
					{
						char* pIT = pData + it;
						if (*(uint32*)(pIT) == hash)
						{
							auto pPos = *(Vector3c*)(pIT - (16 * 3));
							Vector3c gen;
							gen.x = InvEnd(pPos.x);
							gen.y = InvEnd(pPos.z);
							gen.z = InvEnd(pPos.y);
							gen.FlipXY();

							if (gen.IsInBounds({ -16000.f, -16000.f, -16000.f }, { 16000.f, 16000.f, 16000.f }))
							{
								auto pMatrix = (Vector4c*)(pIT - (16 * 6));

								D3DXMATRIX MatrixLE = {
									InvEnd(pMatrix[0].x), InvEnd(pMatrix[0].y), InvEnd(pMatrix[0].z), InvEnd(pMatrix[0].w),
									InvEnd(pMatrix[1].x), InvEnd(pMatrix[1].y), InvEnd(pMatrix[1].z), InvEnd(pMatrix[1].w),
									InvEnd(pMatrix[2].x), InvEnd(pMatrix[2].y), InvEnd(pMatrix[2].z), InvEnd(pMatrix[2].w),
									InvEnd(pMatrix[3].x), InvEnd(pMatrix[3].y), InvEnd(pMatrix[3].z), InvEnd(pMatrix[3].w)
								};

								D3DXMATRIX transposed;
								D3DXMatrixTranspose(&transposed, &MatrixLE);

								Vector3c euler = {};
								euler.x = asinf(-transposed.m[2][1]);
								if (cosf(euler.x) > 0.0001) {
									euler.y = atan2f(transposed.m[2][0], transposed.m[2][2]);
									euler.z = atan2f(transposed.m[0][1], transposed.m[1][1]);
								}
								else {
									euler.y = 0.0f;
									euler.z = atan2f(-transposed.m[1][0], transposed.m[0][0]);
								}

								Vector3c eulerFinal = { euler.z, euler.x, euler.y };

								D3DXQUATERNION quaternion;
								D3DXQuaternionRotationYawPitchRoll(&quaternion, eulerFinal.y, eulerFinal.x, eulerFinal.z);


								D3DXVECTOR3 outScale = {};
								D3DXQUATERNION outRot = {};
								D3DXVECTOR3 outTranslation = {};
								D3DXMatrixDecompose(&outScale, &outRot, &outTranslation, &MatrixLE);

								gen = { outTranslation.z, outTranslation.x, outTranslation.y };

								toret.push_back({ gen, quaternion, MatrixLE, std::get<2>(i), it });
							}
						}
					}
				}
				m_MetaMapFiles.UnlockStore();

				return toret;
			}


			void LoadMetaMap(SkySharedPtr<char[]> sys, uint32 size, SkyString filename)
			{
				bool isLoaded = false;
				MetaMapFile_t file = std::make_tuple(sys, size, filename);
				SkyString filenameOnly = GetFilenameFromPath(filename);
				m_MetaMapFiles.LockStore();
				for (const auto& i : m_MetaMapFiles)
				{
					if (size == std::get<1>(i) &&
						GetFilenameFromPath(std::get<2>(i)) == filenameOnly)
					{
						isLoaded = true;
						break;
					}
				}
				m_MetaMapFiles.push_back(file);
				m_MetaMapFiles.UnlockStore();

				if (!isLoaded)
				{
					// construit une table hashstring contenant les drawables candidats
					const int MIN_STR_LEN = 5;
					const char* pData = sys.get();
					const uint8* pUData = (const uint8*)pData;
					for (int i = 0; i < (size - MIN_STR_LEN); i++)
					{
						bool isOk = true;
						for (int u = 0;; u++)
						{
							int iR = (i + u);

							if (pUData[iR] == 0x00 && u > MIN_STR_LEN)
								break;

							if (iR >= size) {
								isOk = false;
								break;
							}

							if (!((pUData[iR] >= '0' && pUData[iR] <= '9') ||
								(pUData[iR] >= 'A' && pUData[iR] <= 'Z') ||
								(pUData[iR] >= 'a' && pUData[iR] <= 'z') ||
								(pData[iR] == '_')))
							{
								isOk = false;
								break;
							}
						}


						if (isOk)
						{
							uint32 thisHashString = JoaatHash(&pData[i]);
							m_MutexHashTable.Lock();
							{
								auto match = m_MetaMapHashTable.find(thisHashString);
								if (match != m_MetaMapHashTable.end())
								{
									match->second.push_back({ (uint32)i, file });
								}
								else
									m_MetaMapHashTable[thisHashString] = SkyVector<HashCandidat_t>({ { (uint32)i, file } });
							}
							m_MutexHashTable.Unlock();
						}
					}
				}
			}
		};




		static void LoadProcMap(SkyString filename, SkySharedPtr<char[]> cpu, uint32 szCPU, SkySharedPtr<char[]> gpu, uint32 szGPU)
		{
			auto& mapdatastore = atSingleton<CMapDataStoreMgr>::GetRef();
			mapdatastore.m_GrassMapFiles.LockStore();
			mapdatastore.m_GrassMapFiles.push_back({ cpu, gpu, szCPU, szGPU, filename });
			mapdatastore.m_GrassMapFiles.UnlockStore();
		}

		static void LoadSpdMap(SkySharedPtr<char[]> sys, uint32 size, SkyString filename)
		{
			auto& mapdatastore = atSingleton<CMapDataStoreMgr>::GetRef();
			mapdatastore.m_SpdMapFiles.LockStore();
			mapdatastore.m_SpdMapFiles.push_back(std::make_tuple(sys, size, filename));
			mapdatastore.m_SpdMapFiles.UnlockStore();
		}
	}
}