#pragma once
#include "grcTexture_rdr.h"
#include "grcTexture.h"
#include "gtaDrawable_v.h"
#include "rmcDrawable_rdr.h"
#include "rmcDrawable_rdr_to_v.h"
#include <D3DX10math.h>
#include <unordered_map>
#include "ConversionsConstParameters.h"



// Représentation abstraite d'un CEntityDef
struct EntityDef
{
	SkyString m_ModelName;
	uint32 m_Hash = 0; // model hash
	Vector3c m_Pos;
	Quaternion m_Rot = { 0.f, 0.f, 0.f, 1.f };
	Vector3c m_Scale = { 1.f, 1.f, 1.f };
	uint32 m_GUID;
	float m_LODdist;
	uint32 m_Flags = 0;

	inline void SetModel(const SkyString& str) {
		m_ModelName = str;
		m_Hash = JoaatHash(str.c_str());
	}
};

struct ArchetypeDef
{
	uint32 m_Flags = 0;

	uint32 m_DrawableHash;
	SkyString m_DrawableName;
	uint32 m_OriginalHash = 0;

	uint32 m_TxdHash = 0;
	SkyString m_TxdName;

	float m_LodDist = LOD_DISTANCE;
	float m_HdTextureDist = 200.f;

	CBounds m_AABB = {};
	Vector3c m_bsCenter = { 0.f, 0.f, 0.f };
	float m_bsRadius = 1.f;
};


struct ColorRGB {
	uint8 rgb[3];

	uint8& operator[](int index) { return rgb[index]; };
	const uint8 operator[](int index) const { return rgb[index]; };
};

struct GrassDef {
	uint16 m_X;
	uint16 m_Y;
	uint16 m_Z;
	ColorRGB m_Color;

	uint8 m_NormalX;
	uint8 m_NormalY;

	uint8 m_Scale;
	uint8 m_AO;
};
struct GrassBatchDef
{
	SkyString m_ModelName; // proc name
	CBounds m_AABB; // equal to m_Bound of CArea parent
	SkyVector<GrassDef> m_GrassList;
	Vector3c m_Scale = { 0.6f, 1.f, 0.2f };
	float m_LODdist = GRASS_LOD_DIST;
};



/*
	Représente une zone de streaming, c'est une représentation
	abstraite d'un CMapData et d'un CMapTypes (ymap/ytyp)
*/
class CArea
{
public:
	int m_AreaID;

	CMTStore<ArchetypeDef> m_Archetypes;
	CMTStore<SkyTuple<EntityDef, ArchetypeDef>> m_Entities;
	CMTStore<GrassBatchDef> m_GrassBatches;
	CMTStore<SkyString> m_YTYPdeps;

	// CMTStore<SkyTuple<SkyString, gtaDrawablePG*, TinyAllocator*>> m_Drawables;

	CMTStore<SkyTuple<SkyString, uint8*, uint32, uint32>> m_Files; // tuple: name, offset, size, uniqueHash
	CBounds m_Bound;
	
	// Primary YMAP (drawables)
	CBounds m_EntitiesBounds;
	CBounds m_StreamBounds;

	// Batch YMAP (proc, grass)
	CBounds m_ProcEntitiesBounds;
	CBounds m_ProcStreamBounds;

public:
	void AddDrawable(uint32 uniqueHash, const SkyString& filename, uint8* pData, uint32 size);
	void AddEntityDef(const EntityDef& entDef, const ArchetypeDef& arch, bool lock = true);
	void AddArchetypeDef(const ArchetypeDef& arch);
	void AddGrassBatch(const GrassBatchDef& grass);

	void AddYTYP(const SkyString& ytyp, bool lock);
	void BuildArea(const std::string& rootDir);
	bool CoordsIsInArea(const Vector3c& coords);

	void CalculateBounds();
	void CalculateProcBounds();
};


/*
	Fournis un mécanisme de partitionnement
	de la map pour la conversion et orchestre la fusion
	de sortie des données pour construire le fichier de
	map final.
*/
class CMapBuilder
{
public:
	CMTStore<CArea*> m_Areas;
	CArea* m_Props = 0;
	CBounds m_MapBound; // map boundaries

	CSecure m_WorkFileMutex;
	HANDLE m_WorkHFile = 0;
	HANDLE m_WorkMapFile = 0;
	uint8* m_MappedWorkFile = 0;
	uint64 m_WritePos = 0;

	SkyAtomic<uint64> m_ModelsProcessed;

	// CMTMap<uint32, SkyString> m_BuiltHashmap;
	CMTMap<uint32, int> m_QualityHashmap; // hashOriginal, sizeRSC7

	CMTStore<SkySharedPtr<TerrainInfos>> m_TerrainInfos;


public:
	void Init(uint64 WorkFileSize); // 16 GiB

	// Ajoute un drawable dans le monde et l'intègre dans la bonne zone
	CArea* AddDrawable(SkySharedPtr<rdr::rage::Drawable> drawable, rdr::rage::TXD& embedTex, float scale, Vector3c offset, const char* filename = 0);

	// Ajoute une instance de placement et l'intégre dans la bonne zone
	CArea* AddPlacement(const ArchetypeDef& arch, const SkyString& drawableName, uint32 hash,
		Vector3c position, Quaternion rotation = { 0.f, 0.f, 0.f, 1.f });

	// // Etudie le terrain et intégre la végétation
	// void GenerateVegetation(uint32 seed);

	// Préconstruit le partitionnement de la map en volumes égaux
	// metric: représente une métrique afin de déterminer le volume souhaité d'une zone
	void PrebuildMapArea(Vector3c metric);

	void BuildMap(const SkyString& outdirectory);
};
