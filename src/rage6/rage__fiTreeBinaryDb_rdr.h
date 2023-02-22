#pragma once
#include "rage_fwrap.h"
#include <D3DX10Math.h>

namespace rdr
{
	namespace rage
	{
		/*
		struct fiTreeDb
		{
			uint32 m_VT;
			ptr32<uint32> m_pRscPageInfo;

			// tous les store en dessous ont la meme taille
			pgCollect<uint32> m_Unk0; // 0xFFFFFFFF
			pgCollect<uint32> m_BranchsHash;
			pgCollect<char> m_StringSection;
		};

		struct TreeDatabaseHead2
		{
			Vector3c m_AABBMin;
			uint32 m_Unk0;
			Vector3c m_Unk1;
			uint32 m_Unk2;
			Vector3c m_AABBMax;

			pgCollect<TreeBatchInst> m_BatchsList;
		};


		struct TreeBatchInst {
			Vector3c m_BatchPos;
			float m_Unk0;
			pgCollect<TreeInst> m_Trees;
			uint32 m_Unk1;
			uint32 m_Unk2;
			pgCollect<uint16> m_Unk16List;
		};


		struct TreeInst {
			uint64 m_Unk0;
		};
		*/

#pragma pack(push, 1)
		struct fiTreeBatch;
		struct rage__TreeInst;
		struct rage__SpdEntityDef;

		struct fiTreeHeadDB
		{
			uint32 m_VT;
			ptr32<uint32> m_pRscPageInfo;

			// tous les store en dessous ont la meme taille
			pgCollect<uint32> m_Unk0; // 0xFFFFFFFF
			pgCollect<uint32> m_Hashs;
			pgCollect<char> m_Strings;
		};


		struct fiTreeDB
		{
			Vector3c m_AABBMin;
			float m_Unk0;
			Vector3c m_Unk1;
			float m_Unk2;
			Vector3c m_AABBMax;
			float m_Unk3;

			pgCollect<fiTreeBatch> m_BatchsList;
		};

		struct fiTreeBatch
		{
			Vector3c m_Pos;
			float m_Unk0;
			pgCollect<rage__TreeInst> m_List; // buissons ??
			pgCollect<rage__SpdEntityDef> m_PlacementsList; // arbres/props ??
			pgCollect<uint16> m_ListUnk;
			uint64 m_Unk2;
		};

		struct rage__SpdEntityDef
		{
			uint32 m_Unk0;
			uint32 m_Unk1;
			uint64 m_Unk2;
			D3DXMATRIX m_Matrix;
		};
		struct rage__TreeInst
		{
			uint16 X;
			uint16 Y;
			uint16 Z;
			uint16 Scale; // candidat
		};
#pragma pack(pop)

	}
}