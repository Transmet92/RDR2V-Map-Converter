#include "CMapBuilder.h"
#include "rmcDrawable_rdr_to_v.h"
#include "fiPackFile7.h"
#include "CSgaSectorsInfo.h"
#include "SkyCommons/SkyLogs.h"
#include "VegetationGenerator.h"
#include "rage__grassBatch_rdr.h"
#include "rage__fiTreeBinaryDb_rdr.h"
#include "inttypes.h"

using namespace rdr::rage;



void CMapBuilder::Init(uint64 WorkFileSize)
{
	/*
		CONSTRUCTION DU FILE MAPPING DE TRAVAIL
		Les modèles seront copier dans celui-ci le temps que la conversion sois terminé.
		Une fois terminé, les modèles sont récuperer du file map et copier dans les RPF
		des zones lors du processus de fusion.
	*/
	// char deskPath[MAX_PATH];
	// SHGetSpecialFolderPathA(HWND_DESKTOP, deskPath, CSIDL_DESKTOP, FALSE);
	// strcat_s(deskPath, "rdr_builder.tmp");

	/*
	m_WorkHFile = CreateFileA(
		"C:\\Users\\Unknown8192\\Desktop\\RDR_V\\TMP\\rdr_builder.tmp",
		GENERIC_WRITE | GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, 0);

	m_WorkMapFile = CreateFileMappingA(m_WorkHFile, 0, PAGE_READWRITE,
		((DWORD*)&WorkFileSize)[1], ((DWORD*)&WorkFileSize)[0], "RDR_BUILDER");

	m_MappedWorkFile = (uint8*)MapViewOfFile(m_WorkMapFile, FILE_MAP_ALL_ACCESS, 0, 0, WorkFileSize);
	*/

	// CLEANING UP TMP FILES TEXTURES
	const SkyString16 pathToClean = L"C:\\Users\\Unknown8192\\Desktop\\LasVenturas\\TMP";
	CSkyFile::EnumDirectoryCallback(pathToClean.c_str(), [&](const CSkyFile::ElementOfDir& elem)
	{
		if (elem.m_type == CSkyFile::ElementType::FILE)
		{
			SkyString16 fileToDel = pathToClean;
			fileToDel += L"\\";
			fileToDel += elem.m_path;
			DeleteFileW(fileToDel.c_str());
		}
	}, L"*.dds");
}

void CMapBuilder::PrebuildMapArea(Vector3c metric)
{
	m_Props = new CArea();

	m_MapBound.Min.x = -16000.f;
	m_MapBound.Min.y = -16000.f;
	m_MapBound.Min.z = -1000.f;

	m_MapBound.Max.x = 16000.f;
	m_MapBound.Max.y = 16000.f;
	m_MapBound.Max.z = 4000.f;

	uint32 areaId = 0;
	for (float x = m_MapBound.Min.x; x < m_MapBound.Max.x; x += metric.x)
	{
		for (float y = m_MapBound.Min.y; y < m_MapBound.Max.y; y += metric.y)
		{
			for (float z = m_MapBound.Min.z; z < m_MapBound.Max.z; z += metric.z)
			{
				CArea* iNewArea = new CArea();
				CBounds& bounds = iNewArea->m_Bound;
				bounds.Min.x = x;
				bounds.Min.y = y;
				bounds.Min.z = z;

				bounds.Max = bounds.Min;

				bounds.Max.x += metric.x;
				bounds.Max.y += metric.y;
				bounds.Max.z += metric.z;

				iNewArea->m_AreaID = areaId;

				if (false)
				{
					printf("Area #%u   Min { %.2f, %.2f, %.2f }   Max { %.2f, %.2f, %.2f }\n",
						areaId,
						bounds.Min.x, bounds.Min.y, bounds.Min.z,
						bounds.Max.x, bounds.Max.y, bounds.Max.z
					);
				}
				areaId++;

				m_Areas.push_back(iNewArea);
			}
		}
	}

	printf("-----  AREA COUNT:  %u  -----\n", areaId);
}




CArea* CMapBuilder::AddDrawable(SkySharedPtr<Drawable> drawable, TXD& embedTex, float scale, Vector3c offset, const char* filename)
{
	scale = MAP_SCALE;
	offset = MAP_OFFSET;

	Drawable& iD = *drawable.get();
	uint32 originalHash = iD.m_Hash;

	char name[128];
	if (iD.m_Hash == 0) {
		sprintf_s(name, "model_%u", m_ModelsProcessed.load());
		iD.m_Hash = JoaatStr(name);
	}
	else {
		uint8* pHash = (uint8*)&iD.m_Hash;
		sprintf_s(name, "model_0x%02hhx%02hhx%02hhx%02hhx", pHash[0], pHash[1], pHash[2], pHash[3]);
		iD.m_Hash = JoaatStr(name);
	}
	m_ModelsProcessed++;


	// if (!strstr(name, "model_0x06846cd4"))
	// 	return 0;

	if (strstr(name, "model_0x18cbc257"))
		return 0;

	if (strstr(name, "model_0x5f9e3404") ||
		strstr(name, "model_0x8fd23977") ||
		strstr(name, "model_0x18cbc257") ||
		strstr(name, "model_0x06846cd4"))
	{
		// return 0;
	}

	if (strstr(name, "model_0x5f9e3404")) {
		return 0;
	}

	char logfile[128];
	sprintf_s(logfile, "%s___%s.ydr", name, filename);

	// if (strstr(logfile, "model_0x78861ee5___RSC-133__coc_fence02x.xvd"))
	// 	Sleep(1000);

	// if (!strstr(logfile, "model_0x14454ecb______RSC-133__0x42C4850E.cpu.ydr"))
	// 	return 0;


	// printf("drawable: %s\n", logfile);

	iD.m_Name = logfile;


	using namespace v::rage;
	TinyAllocator alloc;

	gtaDrawablePG newDrawable = {};
	bool IsAProp = iD.m_IsAtPositionZero;

	Vector3c position = { 0.f, 0.f, 0.f };

	SkySharedPtr<TerrainInfos> terrainInfo = 0;
	DetailLevel detailLevel;
	bool isValidDrawable = BuildGtaDrawableX(
		alloc,
		newDrawable,
		embedTex,
		iD,
		position,
		detailLevel,
		true,
		terrainInfo,
		scale,
		offset,
		logfile
	);

	if (!isValidDrawable)
		return 0;

	// if (detailLevel != DetailLevel::DRAWABLE_LOD)
	// 	return 0;

	if (true && detailLevel != DetailLevel::DRAWABLE_HD)
	{
		SkyPrint->printColor(12, "INVALID DRAWABLE: %s\n", name);
		return 0;
	}



	// IGNORE SLODs TILES
	if (true)
	{
		float xSize = (fabsf(newDrawable.m_BBoxMin.x - newDrawable.m_BBoxMax.x));
		float ySize = (fabsf(newDrawable.m_BBoxMin.y - newDrawable.m_BBoxMax.y));
		if ((xSize >= (128.f * scale) && xSize <= (132.f * scale)) &&
			(ySize >= (128.f * scale) && ySize <= (132.f * scale)))
			return 0;
	}


	// // BUILD FINAL PATH
	// uint8* iHash = (uint8*)&iD.m_Hash;
	// char filenameOut[128];
	// sprintf_s(filenameOut, "%u-%02hhX%02hhX%02hhX%02hhX.ydr", g_FilesProcessed.load(), iHash[0], iHash[1], iHash[2], iHash[3]);
	// SkyString finalPath = BuildPath(outPath, IsAProp ? PATH_PROPS : PATH_GENERAL_DRAWABLE);
	// finalPath += filenameOut;

	// BUILD RSC7  YDR
	SkyVector<char> drawableBuilt;
	int size = newDrawable.BuildRSC7(strstr(name, "model_0x06846cd4") != 0, rage::RSC_TYPES::eRSC_DRAWABLE, drawableBuilt);
	if (size <= 0) {
		SkyPrint->printColor(12, "RSC7 build error! (%s)\n", name);
		return 0;
	}


	bool bypass = false;
	m_QualityHashmap.LockStore();
	{
		auto matchQual = m_QualityHashmap.find(iD.m_Hash);
		if (matchQual != m_QualityHashmap.end())
		{
			if (size > matchQual->second)
				matchQual->second = size;
			else
				bypass = true;
		}
		else
			m_QualityHashmap[iD.m_Hash] = size;
	}
	m_QualityHashmap.UnlockStore();

	if (bypass)
		return 0;


	//////////// m_WorkFileMutex.Lock();
	//////////// uint64 offset = m_WritePos;
	//////////// memcpy(m_MappedWorkFile + m_WritePos, drawableBuilt.data(), size);
	//////////// m_WritePos += size;
	//////////// m_WorkFileMutex.Unlock();
	uint8* pData = new uint8[size];
	memcpy(pData, drawableBuilt.data(), size);

	SkyPrint->printColor(10, "[rage::rmcDrawable] %s built!\n", logfile);

	const uint32 AreasWhiteList[] = { 0,
		// 1296, 1297, 1298,
		// 1376, 1377, 1378,
		// 1336, 1337, 1338
	};


	ArchetypeDef archetype;
	archetype.m_DrawableName = name;
	archetype.m_DrawableHash = iD.m_Hash;
	archetype.m_OriginalHash = originalHash;
	archetype.m_bsRadius = newDrawable.m_SphereRadius;
	archetype.m_AABB.Min = newDrawable.m_BBoxMin;
	archetype.m_AABB.Max = newDrawable.m_BBoxMax;


	bool vegetable = false;

	if (IsAProp)
	{
		archetype.m_bsCenter = newDrawable.m_BoundCenter;
		archetype.m_LodDist = PROPS_LOD_DISTANCE;
		m_Props->AddDrawable(archetype.m_DrawableHash, name, pData, size);
		m_Props->AddArchetypeDef(archetype);
		return m_Props;
	}
	else
	{
		CArea* toret = 0;
		int areaId = 0;
		m_Areas.LockStore();
		for (CArea* i : m_Areas)
		{
			if (i->CoordsIsInArea(position))
			{
				bool bypass = true;
				if (ARRAYSIZE(AreasWhiteList) > 1)
				{
					bypass = false;
					for (int y = 1; y < ARRAYSIZE(AreasWhiteList); y++) {
						if (AreasWhiteList[y] == areaId) {
							bypass = true;
							break;
						}
					}
				}

				if (bypass)
				{
					EntityDef newdef = {};
					newdef.m_ModelName = name;
					newdef.m_Hash = archetype.m_DrawableHash;
					newdef.m_Pos = position;
					newdef.m_Rot = { 0.f, 0.f, 0.f, 1.f };
					newdef.m_LODdist = LOD_DISTANCE;
					newdef.m_GUID = rand();
					newdef.m_Flags = 0;// 32; // 68681741

					// printf("\narea_%u   --- MODEL : %s\n", areaId, logfile);

					i->AddDrawable(archetype.m_DrawableHash, name, pData, size);
					i->AddEntityDef(newdef, archetype);
					i->AddArchetypeDef(archetype);

					// if (terrainInfo)
					// 	m_TerrainInfos.push_back(terrainInfo);

					toret = i;
					vegetable = true;
				}

				break;
			}

			areaId++;
		}
		m_Areas.UnlockStore();

		if (vegetable && toret)
			GenerateVegetation(toret, newDrawable, position);

		return toret;
	}

	return 0;
}



CArea* CMapBuilder::AddPlacement(
	const ArchetypeDef& arch,
	const SkyString& drawableName,
	uint32 hash,
	Vector3c position,
	Quaternion rotation)
{
	CArea* toret = 0;
	m_Areas.LockStore();
	for (CArea* i : m_Areas)
	{
		if (i->CoordsIsInArea(position))
		{
			EntityDef newdef = {};
			newdef.m_ModelName = drawableName;
			newdef.m_Hash = hash;
			newdef.m_Pos = position;
			newdef.m_Rot = rotation;
			newdef.m_LODdist = PROPS_LOD_DISTANCE;
			newdef.m_GUID = rand();
			newdef.m_Flags = 1;

			i->AddEntityDef(newdef, arch);

			toret = i;
			break;
		}
	}
	m_Areas.UnlockStore();
	return toret;
}




static CMTStore<SkyString> g_areas;
static CMTStore<SkyString> g_ytypPersistent;
static SkyString g_Outdirectory = "";

struct AreaBuilt {
	SkyString areaName;
	uint8* rpfBuffer;
	uint64 rpfSize;
};
static CMTStore<AreaBuilt> g_AreasFinish = {};


class CAreaBuilder
{
public:
	std::vector<CArea*> areas = {};

public:
	static DWORD ThreadAdapter(void* _this)
	{
		((CAreaBuilder*)_this)->BuildArea();
		return 0;
	}

	virtual void BuildArea() = 0;
};






static HANDLE g_DumpFile = 0;
static void Dump(const char* string)
{
	if (g_DumpFile == 0)
		g_DumpFile = CreateFileA("C:\\Users\\Unknown8192\\Desktop\\DUMP_MATRICES_ENTITY.txt", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	DWORD nullle;
	WriteFile(g_DumpFile, string, strlen(string), &nullle, 0);
}
static void DumpFlush() {
	if (g_DumpFile)
		FlushFileBuffers(g_DumpFile);
}


void CMapBuilder::BuildMap(const SkyString& outdirectory)
{
	// Vider le cache de travail des textures
	atSingleton<CTextureLinker>::Get()->DoWork();


	/*
		EFFECTUE LES RECHERCHES D'INSTANCES DE PLACEMENTS DES PROPS
		ET INJECTE LES INSTANCES DANS LEUR AREA
	*/
	if (true)
	{
		SkyPrint->print("\n\n\nConversion des placements...\n\n");

		SkyVector<Vector3c> antidoubleProp = {};

		auto& mapdatastore = atSingleton<CMapDataStoreMgr>::GetRef();
		for (const auto& i : m_Props->m_Archetypes)
		{
			// récupère tous les placements RDR du prop donné
			auto placements = mapdatastore.GetEntityPlacements(i.m_OriginalHash);
			for (auto& it : placements)
			{
				bool add = true;
				
				for (const auto& v : antidoubleProp) {
					if (v == it.m_Pos) {
						add = false;
						break;
					}
				}

				if (add)
				{
					antidoubleProp.push_back(it.m_Pos);

					it.m_Pos *= MAP_SCALE;
					it.m_Pos += MAP_OFFSET;
					// it.m_Pos.z -= ((i.m_AABB.Max.z - i.m_AABB.Min.z) * 0.5f);
					// it.m_Pos.x -= ((i.m_AABB.Max.x - i.m_AABB.Min.x) * 0.5f);
					// it.m_Pos.y -= ((i.m_AABB.Max.y - i.m_AABB.Min.y) * 0.5f);

					// if (ignoreRot)
					// 	it.m_Rot = { 0.f, 0.f, 0.f, 1.f };

					/*
					D3DXMATRIX transposed = {};
					D3DXMatrixTranspose(&transposed, &it.m_Matrix);
					char bufInfo[16384];
					sprintf_s(bufInfo,
						"\n\n--------------------------------------------------------\n"
						"Map File RDR:     %s (match index in file: %u)\n"
						"Prop name:        %s\n"
						"Prop RDR hash:    %u (0x%08" PRIX32 ")\n"
						"Prop GTA hash:    %u (0x%08" PRIX32 ")\n"
						"Translation used: { %.3f, %.3f, %.3f }\n"
						"Quaternion  used: { %.3f, %.3f, %.3f, %.3f }\n"
						"Matrix 4x4:\n"
						"\t------------------------------------\n"
						"\t|  %.3f  %.3f  %.3f  %.3f  |\n"
						"\t|  %.3f  %.3f  %.3f  %.3f  |\n"
						"\t|  %.3f  %.3f  %.3f  %.3f  |\n"
						"\t|  %.3f  %.3f  %.3f  %.3f  |\n"
						"\t------------------------------------\n"
						"Matrix 4x4 transposed:\n"
						"\t------------------------------------\n"
						"\t|  %.3f  %.3f  %.3f  %.3f  |\n"
						"\t|  %.3f  %.3f  %.3f  %.3f  |\n"
						"\t|  %.3f  %.3f  %.3f  %.3f  |\n"
						"\t|  %.3f  %.3f  %.3f  %.3f  |\n"
						"\t------------------------------------\n\n",
						it.m_Filename.c_str(), it.m_IndexInFile,
						i.m_DrawableName.c_str(),
						i.m_OriginalHash, i.m_OriginalHash,
						i.m_DrawableHash, i.m_DrawableHash,
						it.m_Pos.x, it.m_Pos.y, it.m_Pos.z,
						it.m_Rot.x, it.m_Rot.y, it.m_Rot.z, it.m_Rot.w,

						it.m_Matrix.m[0][0], it.m_Matrix.m[0][1], it.m_Matrix.m[0][2], it.m_Matrix.m[0][3],
						it.m_Matrix.m[1][0], it.m_Matrix.m[1][1], it.m_Matrix.m[1][2], it.m_Matrix.m[1][3],
						it.m_Matrix.m[2][0], it.m_Matrix.m[2][1], it.m_Matrix.m[2][2], it.m_Matrix.m[2][3],
						it.m_Matrix.m[3][0], it.m_Matrix.m[3][1], it.m_Matrix.m[3][2], it.m_Matrix.m[3][3],

						transposed.m[0][0], transposed.m[0][1], transposed.m[0][2], transposed.m[0][3],
						transposed.m[1][0], transposed.m[1][1], transposed.m[1][2], transposed.m[1][3],
						transposed.m[2][0], transposed.m[2][1], transposed.m[2][2], transposed.m[2][3],
						transposed.m[3][0], transposed.m[3][1], transposed.m[3][2], transposed.m[3][3]
					);

					Dump(bufInfo);
					*/

					AddPlacement(i, i.m_DrawableName, i.m_DrawableHash, it.m_Pos,
						{ it.m_Rot.x, it.m_Rot.y, it.m_Rot.z, it.m_Rot.w });
				}
			}

			// DumpFlush();

			if (placements.size())
				SkyPrint->print("\tdrawable '%s' (%p:%p) :  %u placements\n",
					i.m_DrawableName.c_str(), i.m_OriginalHash, i.m_DrawableHash, placements.size());
		}
		SkyPrint->print("\n");
	}




	/*
		EFFECTUE LES RECHERCHES D'INSTANCES DE GRASS DE RDR
		ET INJECTE LES INSTANCES DANS LEUR AREA
	*/
	{
		SkyPrint->print("\n\n\nConversion des grass batch...\n\n");

		const char* grassNamesList[] =
		{
			"gap_grass04",
			"gri_grass03",
			"cho_grass01",
			"rio_grass01",
			"grass",
			"grt_grass04",
			"grass1",
			"hen_grass03",
			"pun_grass03",
			"hen_grass04",
			"die_grass04",
			"die_grass03",
			"gap_grass01",
			"rio_grass02",
			"tal_grass01",
			"tie_grass01",
			"grt_grass03",
			"tal_grass03",
			"rio_grass03",
			"gap_grass02",
			"pun_grass02",
			"grt_grass02",
			"rio_grass04",
			"gri_grass01",
			"per_grass01",
			"tie_grass02",
			"gri_grass04",
			"tal_grass04",
			"per_grass02",
			"sno_grass01",
			"cho_grass03",
			"per_grass03",
			"pun_grass01",
			"pun_grass04",
			"grt_grass01",
			"gri_grass02",
			"cho_grass02",
			"per_grass04",
			"tal_grass02",
			"cho_grass04",
			"die_grass01",
			"hen_grass02",
			"sno_grass02",
			"gap_grass03",
			"hen_grass01",
			"die_grass02",
		};

		const uint32 grassHashesList[] =
		{
			JoaatHash("gap_grass04"),
			JoaatHash("gri_grass03"),
			JoaatHash("cho_grass01"),
			JoaatHash("rio_grass01"),
			JoaatHash("grass"),
			JoaatHash("grt_grass04"),
			JoaatHash("grass1"),
			JoaatHash("hen_grass03"),
			JoaatHash("pun_grass03"),
			JoaatHash("hen_grass04"),
			JoaatHash("die_grass04"),
			JoaatHash("die_grass03"),
			JoaatHash("gap_grass01"),
			JoaatHash("rio_grass02"),
			JoaatHash("tal_grass01"),
			JoaatHash("tie_grass01"),
			JoaatHash("grt_grass03"),
			JoaatHash("tal_grass03"),
			JoaatHash("rio_grass03"),
			JoaatHash("gap_grass02"),
			JoaatHash("pun_grass02"),
			JoaatHash("grt_grass02"),
			JoaatHash("rio_grass04"),
			JoaatHash("gri_grass01"),
			JoaatHash("per_grass01"),
			JoaatHash("tie_grass02"),
			JoaatHash("gri_grass04"),
			JoaatHash("tal_grass04"),
			JoaatHash("per_grass02"),
			JoaatHash("sno_grass01"),
			JoaatHash("cho_grass03"),
			JoaatHash("per_grass03"),
			JoaatHash("pun_grass01"),
			JoaatHash("pun_grass04"),
			JoaatHash("grt_grass01"),
			JoaatHash("gri_grass02"),
			JoaatHash("cho_grass02"),
			JoaatHash("per_grass04"),
			JoaatHash("tal_grass02"),
			JoaatHash("cho_grass04"),
			JoaatHash("die_grass01"),
			JoaatHash("hen_grass02"),
			JoaatHash("sno_grass02"),
			JoaatHash("gap_grass03"),
			JoaatHash("hen_grass01"),
			JoaatHash("die_grass02"),
		};

		struct Proc {
			const char* name;
			float density; // [0,1]
			int type; // 0 = desert
		};
		Proc procs[] = {
			{ "proc_desert_sage_01", 1.f, 0 },
			{ "proc_drygrasses01", 1.f, 0 },
			{ "proc_drygrasses01b", 1.f, 0 },
			{ "proc_dry_plants_01", 0.2f, 0 },
			{ "proc_stones_02", 0.1f, 0 },
			{ "proc_grassdandelion01", 0.5f, 0 },
			{ "proc_scrub_bush01", 0.2f, 0 },
		};

		uint32 grassCountTotal = 0;


		auto& mapdatastore = atSingleton<CMapDataStoreMgr>::GetRef();
		for (int y = 0; y < ARRAYSIZE(grassHashesList); y++)
		{
			int grassMatches = 0;
			uint32 hashBE = InvEnd(grassHashesList[y]);
			for (const auto& i : mapdatastore.m_GrassMapFiles)
			{
				auto* pData = (uint8*)i.m_CPU.get();
				int iSize = i.m_szCPU;
				for (int it = 0; it < iSize; it += 4)
				{
					uint8* pIT = pData + it;
					if (*(uint32*)(pIT) == hashBE)
					{
						uint8* pGrassDat = pIT - (sizeof(rdr::grassBatch) - sizeof(hashBE));
						if (pGrassDat >= pData)
						{
							rdr::grassBatch iGBatch = *(rdr::grassBatch*)pGrassDat;
							iGBatch.ToLittleEndian();
							// permute les composantes YZ et XY
							iGBatch.m_AABBMax.FlipYZ(); iGBatch.m_AABBMax.FlipXY();
							iGBatch.m_AABBMin.FlipYZ(); iGBatch.m_AABBMin.FlipXY();

							// verifie si il s'agit d'un pointeur d'espace mémoire CPU
							if ((iGBatch.m_BatchDefPtr.m_Offset & 0xF0) == 0x50)
							{
								uint8* pGPU = (uint8*)i.m_GPU.get();

								rdr::grassBatchDef BatchDef = *(iGBatch.m_BatchDefPtr.Get(pData, pGPU));
								BatchDef.ToLittleEndian();

								auto* grassBuffer = (rdr::rage__GrassBatch*)BatchDef.m_pBatchBuffer.Get(pData, pGPU);

								iGBatch.m_AABBMin *= MAP_SCALE;
								iGBatch.m_AABBMin += MAP_OFFSET;

								iGBatch.m_AABBMax *= MAP_SCALE;
								iGBatch.m_AABBMax += MAP_OFFSET;

								GrassBatchDef grassBatch = {};
								grassBatch.m_AABB.Min = iGBatch.m_AABBMin;
								grassBatch.m_AABB.Max = iGBatch.m_AABBMax;
								grassBatch.m_ModelName = procs[1].name;
								grassBatch.m_LODdist = GRASS_LOD_DIST;
								grassBatch.m_Scale = { 1.2f, 1.2f, 1.f };

								for (int g = 0; g < BatchDef.m_GrassCount; g++)
								{
									auto& iGrass = grassBuffer[g];

									// XY FLIP !
									uint16 XGrass = (uint16)(((float)iGrass.m_Y / 255.f) * 65535.f);
									uint16 YGrass = (uint16)(((float)iGrass.m_X / 255.f) * 65535.f);
									uint16 ZGrass = (uint16)(((float)iGrass.m_Z / 255.f) * 65535.f);

									grassBatch.m_GrassList.push_back({
										XGrass, YGrass, ZGrass,
										{ 161, 153, 124 }, // color
										126, 128, // normal xy
										iGrass.m_Scale, // scale
										255 // AO
									});

									grassMatches++;
									grassCountTotal++;
								}

								Vector3c batchSize = grassBatch.m_AABB.Max - grassBatch.m_AABB.Min;
								Vector3c batchCentroid = grassBatch.m_AABB.Min + (batchSize * 0.5f);

								for (CArea* i : m_Areas) {
									if (i->CoordsIsInArea(batchCentroid)) {
										i->AddGrassBatch(grassBatch);
										break;
									}
								}
							}
							else
								SkyPrint->printColor(12, "[rdr::rage__grass] error grass pointer match in %s at %u\n", i.m_Filename.c_str(), it);
						}
						else
							SkyPrint->printColor(12, "[rdr::rage__grass] error grass structure in %s at %u\n", i.m_Filename.c_str(), it);
					}
				}
			}

			SkyPrint->printColor(10, "[rdr::rage__grass] %s : %d grasses planted\n", grassNamesList[y], grassMatches);
		}

		SkyPrint->printColor(10, "\n\n---------------------\n[rdr::rage__grass] NUMBER OF GRASSES PLANTED : %u\n---------------------\n\n\n\n", grassCountTotal);
	}



	/*
		EFFECTUE LES RECHERCHES D'INSTANCES DE GRASS DE RDR
		ET INJECTE LES INSTANCES DANS LEUR AREA
	*/
	if (false)
	{
		SkyPrint->print("\n\n\nConversion des tree batch...\n\n");

		SkyVector<CBounds> antidoubleTrees = {};

		uint32 placementsCount = 0;
		auto& mapdatastore = atSingleton<CMapDataStoreMgr>::GetRef();
		for (const auto& i : mapdatastore.m_SpdMapFiles)
		{
			SkyPrint->printColor(10, "[rdr::rage__tree] %s\n", std::get<2>(i).c_str());

			uint8* ppgBase = (uint8*)std::get<0>(i).get();
			int iSize = std::get<1>(i);

			auto* pTreeBase = (rdr::rage::fiTreeHeadDB*)ppgBase;
			auto pTreeDB = (rdr::rage::fiTreeDB*)(ppgBase + 0x60);

			pTreeDB->m_Unk0 = InvEnd(pTreeDB->m_Unk0);
			pTreeDB->m_Unk2 = InvEnd(pTreeDB->m_Unk2);
			pTreeDB->m_Unk3 = InvEnd(pTreeDB->m_Unk3);

			CBounds TreeAABB = {
				LittleEndian(pTreeDB->m_AABBMin),
				LittleEndian(pTreeDB->m_AABBMax)
			};


			bool add = true;
			for (auto& v : antidoubleTrees) {
				if (v == TreeAABB) {
					add = false;
					break;
				}
			}

			if (add)
			{
				antidoubleTrees.push_back(TreeAABB);

				SkyVector<CArea*> cacheResolve = {};

				Vector3c BoundsSize = {
					TreeAABB.Max.x - TreeAABB.Min.x,
					TreeAABB.Max.y - TreeAABB.Min.y,
					TreeAABB.Max.z - TreeAABB.Min.z
				};


				// batchs buissons
				pgCollect<fiTreeBatch>& batchList = pTreeDB->m_BatchsList;
				for (int x = 0; x < batchList.GetCount(); x++)
				{
					auto iBatch = batchList.GetAt(x, ppgBase, (void*)nullptr);
					Vector3c CentroidBatch = LittleEndian(iBatch->m_Pos);
					iBatch->m_Unk0 = InvEnd(iBatch->m_Unk0);

					Vector3c CentroidCalculate = {
						TreeAABB.Min.x + (BoundsSize.x * 0.5f),
						TreeAABB.Min.y + (BoundsSize.y * 0.5f),
						TreeAABB.Min.z + (BoundsSize.z * 0.5f),
					};

					// positionnements matriciel
					pgCollect<rage__SpdEntityDef>& placements = iBatch->m_PlacementsList;
					for (int x = 0; x < placements.GetCount(); x++)
					{
						auto entityDef = placements.GetAt(x, ppgBase, (void*)nullptr);
						const D3DXMATRIX& defMatrix = entityDef->m_Matrix;

						D3DXMATRIX MatrixLE = {
							InvEnd(defMatrix.m[0][0]), InvEnd(defMatrix.m[0][1]), InvEnd(defMatrix.m[0][2]), InvEnd(defMatrix.m[0][3]),
							InvEnd(defMatrix.m[1][0]), InvEnd(defMatrix.m[1][1]), InvEnd(defMatrix.m[1][2]), InvEnd(defMatrix.m[1][3]),
							InvEnd(defMatrix.m[2][0]), InvEnd(defMatrix.m[2][1]), InvEnd(defMatrix.m[2][2]), InvEnd(defMatrix.m[2][3]),
							InvEnd(defMatrix.m[3][0]), InvEnd(defMatrix.m[3][1]), InvEnd(defMatrix.m[3][2]), InvEnd(defMatrix.m[3][3])
						};

						D3DXVECTOR3 outScale = {};
						D3DXQUATERNION outRot = {};
						D3DXVECTOR3 outTranslation = {};
						D3DXMatrixDecompose(&outScale, &outRot, &outTranslation, &MatrixLE);

						Vector3c absPos = { outTranslation.z, outTranslation.x, outTranslation.y };

						absPos *= MAP_SCALE;
						absPos += MAP_OFFSET;


						CArea* match = 0;
						for (const auto r : cacheResolve) {
							if (r->CoordsIsInArea(absPos)) {
								match = r;
								break;
							}
						}
						if (!match) {
							for (CArea* r : m_Areas) {
								if (r->CoordsIsInArea(absPos))
								{
									cacheResolve.push_back(r);
									match = r;
									break;
								}
							}
						}

						if (match)
						{
							static const SkyString prop = "prop_tree_birch_01";
							static const uint32 hashProp = JoaatHash("prop_tree_birch_01");
							EntityDef entity = {
								prop,
								hashProp,
								absPos,
								{ 0.f, 0.f, 0.f, 1.f },
								{ 1.f, 1.f, 1.f },
								(uint32)rand(),
								400.f,
								0
							};
							ArchetypeDef arch;
							arch.m_AABB = {
								{ -5.480225f, -5.089279f, -0.073127f },
								{ 4.627686f, 5.183853f, 15.98712f },
							};
							match->AddYTYP("v_trees", false);
							match->AddEntityDef(entity, arch, false);

							placementsCount++;
						}
					}



					// buissons ??? (batchs deter)
					pgCollect<rage__TreeInst>& treelist = iBatch->m_List;
					for (int y = 0; y < treelist.GetCount(); y++)
					{
						const rage__TreeInst& iTree = *treelist.GetAt(y, ppgBase, (void*)nullptr);
						Vector3c absPos = {
							TreeAABB.Min.z + (((((float)InvEnd(iTree.Z)) / 65535.f) * 2.f) * BoundsSize.z),
							TreeAABB.Min.x + (((((float)InvEnd(iTree.X)) / 65535.f) * 2.f) * BoundsSize.x),
							TreeAABB.Min.y + (((((float)InvEnd(iTree.Y)) / 65535.f) * 2.f) * BoundsSize.y),
						};

						absPos *= MAP_SCALE;
						absPos += MAP_OFFSET;

						CArea* match = 0;

						for (const auto r : cacheResolve) {
							if (r->CoordsIsInArea(absPos)) {
								match = r;
								break;
							}
						}
						if (!match) {
							for (CArea* r : m_Areas) {
								if (r->CoordsIsInArea(absPos))
								{
									cacheResolve.push_back(r);
									match = r;
									break;
								}
							}
						}

						if (match)
						{
							static const SkyString prop = "prop_bush_med_03_cr";
							static const uint32 hashProp = JoaatHash("prop_bush_med_03_cr");
							EntityDef entity = {
								prop,
								hashProp,
								absPos,
								{ 0.f, 0.f, 0.f, 1.f },
								{ 1.f, 1.f, 1.f },
								(uint32)rand(),
								400.f,
								0
							};
							ArchetypeDef arch;
							arch.m_AABB = {
								{ -1.494415f, -1.183617f, -0.105317f },
								{ 1.34964f, 1.478676f, 4.036383f },
							};
							match->AddYTYP("v_bush", false);
							match->AddEntityDef(entity, arch, false);


							// match->AddGrassBatch();


							placementsCount++;
						}
					}
				}
			}
		}

		SkyPrint->printColor(10, "[rdr::rage__tree] %d placements\n", placementsCount);
	}




	SkyVector<SkyString> ytypPersistent;

	g_Outdirectory = outdirectory;

	HANDLE ThreadsList[BUILD_MAP_THREAD_COUNT];

	SkyVector<CArea*> AreasToBuild = {};
	for (auto i : m_Areas)
	{
		if (i->m_Entities.size())
			AreasToBuild.push_back(i);
	}


	int AreaCount = AreasToBuild.size();
	uint32 AreasPerThread = AreaCount / BUILD_MAP_THREAD_COUNT;
	uint32 AreasRest = AreaCount % BUILD_MAP_THREAD_COUNT;

	for (int i = 0, iF = 0; i < BUILD_MAP_THREAD_COUNT; i++)
	{
		uint32 iAreasToProcessCount = AreasPerThread;
		if (i + 1 == BUILD_MAP_THREAD_COUNT)
			iAreasToProcessCount += AreasRest;

		CAreaBuilder* iAreaBuilder = new CAreaBuilder();
		uint32 limitCount = iF + iAreasToProcessCount;
		for (; iF < limitCount; iF++)
			iAreaBuilder->areas.push_back(AreasToBuild[iF]);

		ThreadsList[i] = CreateThread(0, 10 * 1024 * 1024, CAreaBuilder::ThreadAdapter, (void*)iAreaBuilder, 0, 0);
	}

	for (;;)
	{
		DWORD res = WaitForMultipleObjects(BUILD_MAP_THREAD_COUNT, ThreadsList, TRUE, 100);
		if ((res >= WAIT_OBJECT_0) && (res < (WAIT_OBJECT_0 + BUILD_MAP_THREAD_COUNT)))
			break;
	}





	SkyVector<rage::CPackageOpQueue> DlcRpfs = {};
	const uint32 MAX_RPF_SIZE = 1200000000;



	// PROPS RPFs
	const int PropsPerRpf = 400;
	int totalPropsCount = m_Props->m_Files.size();
	int PropsRpfCount = totalPropsCount / PropsPerRpf;
	PropsRpfCount += (((totalPropsCount % PropsPerRpf) == 0) ? 0 : 1);

	int it = 0;
	for (int i = 0; i < PropsRpfCount; i++)
	{
		char pathProps[32];
		sprintf_s(pathProps, "props_%u", i);
		SkyString propsPath = pathProps;
		propsPath += ".rpf";

		SkyPrint->print("Building RPF : '%s.rpf'\n", pathProps);


		SkyVector<uint8*> toRelease = {}; // release when the rpf has built

		rage::CPackageOpQueue iQueue = {};

		int maxItRange = it + PropsPerRpf;
		for (; it < maxItRange && it < totalPropsCount; it++)
		{
			auto& iF = m_Props->m_Files[it];
			SkyString filePath = "";
			filePath += std::get<0>(iF).c_str();
			filePath += ".ydr";

			uint8* pData = std::get<1>(iF);
			iQueue.AddFile(filePath.c_str(), pData, std::get<2>(iF));
			toRelease.push_back(pData);
		}



		rage7::CPackageBuilder7 rpf = {};
		rpf.BuildMap(iQueue.GetQueue());
		uint64 thisRpfSize = rpf.GetSize();

		uint8* rpfBuffer = new uint8[thisRpfSize + 2048];

		rpf.WriteAt(rpfBuffer);


		for (const auto z : toRelease)
			delete[] z;


		bool createNewRpf = true;
		for (auto& z : DlcRpfs) {
			if (z.GetApproxBytesCount() + thisRpfSize < MAX_RPF_SIZE) {
				z.AddFile(propsPath.c_str(), rpfBuffer, thisRpfSize);
				createNewRpf = false;
				break;
			}
		}
		if (createNewRpf) {
			rage::CPackageOpQueue newQueue = {};
			newQueue.AddFile(propsPath.c_str(), rpfBuffer, thisRpfSize);
			DlcRpfs.push_back(newQueue);
		}
	}




	for (const auto& i : g_AreasFinish)
	{
		SkyString nameInRpf = i.areaName;
		nameInRpf += ".rpf";

		bool createNewRpf = true;
		for (auto& z : DlcRpfs) {
			if (z.GetApproxBytesCount() + i.rpfSize < MAX_RPF_SIZE) {
				z.AddFile(nameInRpf.c_str(), i.rpfBuffer, i.rpfSize);
				createNewRpf = false;
				break;
			}
		}
		if (createNewRpf) {
			rage::CPackageOpQueue newQueue = {};
			newQueue.AddFile(nameInRpf.c_str(), i.rpfBuffer, i.rpfSize);
			DlcRpfs.push_back(newQueue);
		}
	}



	// CONSTRUIT LES SUBPACKS
	int subpackId = 1;
	for (auto& i : DlcRpfs)
	{
		wchar_t dlcRpfName[32];
		swprintf_s(dlcRpfName, L"dlc%u.rpf", subpackId++);
		SkyString16 outdlc = OUT_DIRECTORY;
		outdlc += dlcRpfName;


		rage7::CPackageBuilder7 rpf = {};
		rpf.BuildMap(i.GetQueue());

		CMapFile outRpf = {};
		outRpf.CreateMap(outdlc.c_str(), true, rpf.GetSize());

		rpf.WriteAt(outRpf.Get());
	}



	// CONSTRUIT LE dlc.rpf
	{
		// construit le setup2.xml
		std::stringstream setup;
		setup << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
		setup << "<SSetupData>";
		{
			setup << "<deviceName>venturas</deviceName>";
			setup << "<datFile>content.xml</datFile>";
			setup << "<timeStamp>02/05/2022 16:25:25</timeStamp>";
			setup << "<nameHash>LasVenturas</nameHash>";

			setup << "<contentChangeSets />";
			setup << "<contentChangeSetGroups>";
			{
				// // GROUP_STARTUP
				// setup << "<Item>";
				// {
				// 	setup << "<NameHash>GROUP_STARTUP</NameHash>";
				// 	setup << "<ContentChangeSets>";
				// 	{
				// 		setup << "<Item>CCS_lasventuras_NG_INIT</Item>";
				// 	}
				// 	setup << "</ContentChangeSets>";
				// }
				// setup << "</Item>";


				// GROUP_MAP
				setup << "<Item>";
				{
					setup << "<NameHash>GROUP_UPDATE_STREAMING</NameHash>";
					setup << "<ContentChangeSets>";
					{
						setup << "<Item>CCS_lasventuras_NG_STREAMING</Item>";
						setup << "<Item>CCS_lasventuras_NG_STREAMING_MAP</Item>";
					}
					setup << "</ContentChangeSets>";
				}
				setup << "</Item>";
			}
			setup << "</contentChangeSetGroups>";

			setup << "<startupScript />";
			setup << "<scriptCallstackSize value=\"0\" />";
			setup << "<type>EXTRACONTENT_LEVEL_PACK</type>";
			setup << "<order value=\"2\" />";
			setup << "<minorOrder value=\"0\" />";
			setup << "<isLevelPack value=\"true\" />";
			setup << "<dependencyPackHash />";
			setup << "<requiredVersion />";
			setup << "<subPackCount value=\"" << (subpackId - 1) << "\" />";
		}
		setup << "</SSetupData>";



		// construit le content.xml
		std::stringstream content;
		content << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
		content << "<CDataFileMgr__ContentsOfDataFileXml>";
		{
			content << "<disabledFiles />";
			content << "<includedXmlFiles />";

			content << "<dataFiles>";
			{
				for (int i = 0; i < PropsRpfCount; i++)
				{
					content << "<Item>";
					{
						content << "<filename>";
						{
							char pathProps[32];
							sprintf_s(pathProps, "props_%u.rpf", i);
							content << "venturas:/";
							content << pathProps;
						}
						content << "</filename>";
						content << "<fileType>RPF_FILE</fileType>";
						content << "<overlay value=\"false\" />";
						content << "<disabled value=\"true\" />";
						content << "<persistent value=\"true\" />";
					}
					content << "</Item>";
				}


				for (const auto& i : g_AreasFinish)
				{
					content << "<Item>";
					{
						content << "<filename>";
						{
							content << "venturas:/";
							content << i.areaName.c_str();
							content << ".rpf";
						}
						content << "</filename>";
						content << "<fileType>RPF_FILE</fileType>";
						content << "<overlay value=\"false\" />";
						content << "<disabled value=\"true\" />";
						content << "<persistent value=\"true\" />";
					}
					content << "</Item>";
				}


				for (const auto& i : ytypPersistent)
				{
					content << "<Item>";
					{
						content << "<filename>";
						{
							content << "venturas:/";
							content << i.c_str();
							content << ".rpf/";
							content << i.c_str();
							content << ".ityp";
						}
						content << "</filename>";
						content << "<fileType>DLC_ITYP_REQUEST</fileType>";
						content << "<overlay value=\"false\" />";
						content << "<disabled value=\"true\" />";
						content << "<persistent value=\"false\" />";
						content << "<contents>CONTENTS_PROPS</contents>";
					}
					content << "</Item>";
				}
			}
			content << "</dataFiles>";

			content << "<contentChangeSets>";
			{
				content << "<Item>";
				{
					content << "<changeSetName>CCS_lasventuras_NG_STREAMING_MAP</changeSetName>";
					content << "<filesToEnable>";
					{
						for (int i = 0; i < PropsRpfCount; i++)
						{
							content << "<Item>";
							{
								char pathProps[32];
								sprintf_s(pathProps, "props_%u.rpf", i);
								content << "venturas:/";
								content << pathProps;
							}
							content << "</Item>";
						}

						for (const auto& i : g_AreasFinish)
						{
							content << "<Item>";
							{
								content << "venturas:/";
								content << i.areaName.c_str();
								content << ".rpf";
							}
							content << "</Item>";
						}

						for (const auto& i : ytypPersistent)
						{
							content << "<Item>";
							{
								content << "venturas:/";
								content << i.c_str();
								content << ".rpf/";
								content << i.c_str();
								content << ".ityp";
							}
							content << "</Item>";
						}
					}
					content << "</filesToEnable>";
					content << "<executionConditions>";
					{
						content << "<activeChangesetConditions>";
						content << "</activeChangesetConditions>";
						content << "<genericConditions>$level=MO_JIM_L11</genericConditions>";
					}
					content << "</executionConditions>";
				}
				content << "</Item>";
			}
			content << "</contentChangeSets>";

			content << "<patchFiles />";
		}
		content << "</CDataFileMgr__ContentsOfDataFileXml>";




		SkyString16 outdlc = OUT_DIRECTORY;
		outdlc += L"dlc.rpf";

		rage::CPackageOpQueue queue = {};

		auto strContent = content.str();
		queue.AddFile("content.xml", (uint8*)strContent.c_str(), strContent.size());

		auto strSetup = setup.str();
		queue.AddFile("setup2.xml", (uint8*)strSetup.c_str(), strSetup.size());

		rage7::CPackageBuilder7 rpf = {};
		rpf.BuildMap(queue.GetQueue());

		CMapFile outRpf = {};
		outRpf.CreateMap(outdlc.c_str(), true, rpf.GetSize());

		rpf.WriteAt(outRpf.Get());
	}
}

