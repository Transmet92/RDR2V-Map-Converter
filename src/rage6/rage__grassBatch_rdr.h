#pragma once
#include "rage_fwrap.h"

namespace rdr
{
	struct grassBatchDef;
	struct grassBatchDefExt;
	struct UnkStruct2;
	struct grassBatchDefExt2;
	struct rage__GrassBatch;

	struct grassBatch
	{
		Vector4c m_Unk0; // possible ScaleRange equivalent for RDR (usually: 7F800001 x 4)
		Vector3c m_AABBMax; // AABB ou BB ?? pas sur
		float m_Pad1;
		Vector3c m_AABBMin;
		float m_Pad2;
		Vector4c m_Unk1; // probable scaleRange
		Vector3c m_AABBMin2; // pareil que m_AABBMin
		float m_SurfaceArea; // candidat pour l'aire de la surface XY du grassBatch
		uint32 m_Unk2;
		uint32 m_Unk3;
		ptr32<grassBatchDef> m_BatchDefPtr; // [CPU] pointeur vers la sous structure grassBatchDefExt
		uint16 m_Unk4;
		uint16 m_Pad0;
		ptr32<char> m_GrassName; // [CPU] pointeur vers le nom de grass. Eg: "cho_grass01", "gap_grass01"
		uint16 m_Unk5; // coords XY ??
		uint16 m_Unk6;
		uint32 m_GrassHash; // 0x69C10825 etc...

		void ToLittleEndian() {
			m_AABBMax.x = InvEnd(m_AABBMax.x);
			m_AABBMax.y = InvEnd(m_AABBMax.y);
			m_AABBMax.z = InvEnd(m_AABBMax.z);
			m_AABBMin.x = InvEnd(m_AABBMin.x);
			m_AABBMin.y = InvEnd(m_AABBMin.y);
			m_AABBMin.z = InvEnd(m_AABBMin.z);
			m_AABBMin2.x = InvEnd(m_AABBMin2.x);
			m_AABBMin2.y = InvEnd(m_AABBMin2.y);
			m_AABBMin2.z = InvEnd(m_AABBMin2.z);
			m_Unk0.x = InvEnd(m_Unk0.x);
			m_Unk0.y = InvEnd(m_Unk0.y);
			m_Unk0.z = InvEnd(m_Unk0.z);
			m_Unk0.w = InvEnd(m_Unk0.w);
			m_SurfaceArea = InvEnd(m_SurfaceArea);
		};
	};

	struct grassBatchDef
	{
		uint16 m_Unk0;
		uint16 m_Unk1;
		uint16 m_GrassCount; // x*4 = nombre d'octets du tampon grassbatch
		uint16 m_Unk5;
		ptr32<rage__GrassBatch> m_pBatchBuffer; // [GPU] pointeur vers le tampon grassbatch dans l'espace gpu
		uint32 m_Unk3;
		ptr32<rage__GrassBatch> m_pBatchBuffer2; // [GPU] pointeur vers le tampon grassbatch dans l'espace gpu
		uint32 m_Unk4;
		ptr32<UnkStruct2> m_pUnk0; // [CPU] pointeur 
		ptr32<grassBatchDefExt2> m_pBatchDefExt; // [CPU] extension de définition inconnu

		void ToLittleEndian() {
			m_GrassCount = InvEnd(m_GrassCount);
		};
	};

	// GPU Shader structure
	struct rage__GrassBatch {
		uint8 m_Scale;
		uint8 m_X;
		uint8 m_Z;
		uint8 m_Y;
	};

	struct grassBatchDefExt2
	{
		uint32 m_Unk0;
		uint32 m_Unk1;
		uint32 m_Unk2;
		uint32 m_Unk3;
		uint32 m_Unk4;
		uint16 m_Unk5;
		uint16 m_Unk6;
		ptr32<char*> m_pUnk; // [GPU] pointeur vers [m_pBatchBuffer] + 3 octets dans l'espace gpu
		uint32 m_Unk7;
	};

	// 00000010040000010000000000080000
	struct UnkStruct2 {

	};
}