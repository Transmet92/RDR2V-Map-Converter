#pragma once
#include "rage_fwrap.h"

namespace rage
{
	// Material ?
	class phUnk1 {
	public:
		uint32 m_Unk0;
	};

	class phPoly {
		float m_Unk0;

		uint16 m_v1;
		uint16 m_v2;
		uint16 m_v3;

		uint16 m_e1;
		uint16 m_e2;
		uint16 m_e3;
	};

	struct phVector3 {
		int16 x;
		int16 y;
		int16 z;
	};

	enum phBoundsType : uint8 {
		BT_CAPSULE = 1,
		BT_POLYHEDRON_1 = 3,
		BT_POLYHEDRON_2 = 4,
		BT_POLYHEDRON_3 = 5,
		BT_RIBBON = 9,
		BT_POLYHEDRON_4 = 10,
		BT_SURFACE = 11,
		BT_UNK = 12, // BVH OR COMPOSITE
	};

	class phBound {
	public:
		uint32 m_VT; // inheritance vtable
		uint32 m_UnkA0;
		uint32 m_UnkA1;
		uint32 m_UnkA2;
		uint32 m_UnkA3;
		phBoundsType m_BoundType;
		uint8 m_UnkA4;
		uint16 m_UnkA5;
		float m_Radius;
		float m_WorldRadius;

		Vector4c m_AABBMax;
		Vector4c m_AABBMin;
		Vector4c m_Centre;
		Vector4c m_UnkA6; // 0, 0, 0
		Vector4c m_GravityCenter;

		Vector4c m_UnkA7;

		Vector3c m_UnkA8;
		uint32 m_UnkA9; // 2  (count ?)

	};

	// vtable: 14CB5700
	class phBoundPolyhedron : public phBound
	{
	public:
		uint32 m_VT;
		uint32 m_Unk0;
		uint32 m_Unk1;
		uint32 m_Unk2;
		uint32 m_Unk3;
		uint32 m_Unk4;
		float m_Unk5; // 26.52  (sphere radius ?)
		float m_Unk6; // 5154.977  (??)

		Vector4c m_UnkPos0; // AABB ?
		Vector4c m_UnkPos1;
		Vector4c m_UnkPos2;
		Vector4c m_Unk7; // 0, 0, 0, 1.n

		Vector4c m_UnkPos3;
		Vector4c m_Unk8; // scale ? (1, 1, 1, 1)

		uint32 m_Unk9; // 3BA3D70A
		uint32 m_Unk10; // 3BA3D70A
		uint32 m_Unk11; // 3BA3D70A
		uint32 m_Unk12; // 2

		uint32 m_Unk13;
		uint32 m_Unk14;
		uint32 m_Unk15;
		ptr32<phPoly> m_pUnk0; // BHV ? Material ?

		Vector4c m_VertexScale;
		Vector4c m_VertexOffset;

		// tampon de sommets physique BVH ?
		ptr32<phVector3> m_Vertices;
		uint32 m_Unk17;
		uint16 m_Unk18;
		uint16 m_VerticesCount;
		uint32 m_Unk19; // FFFFFFFF
		uint32 m_Unk20;
		uint32 m_Unk21;
		uint16 m_Unk22;
		uint16 m_VerticesCount2; // identique au 1 (pas sûr)
		uint32 m_PolyCount; // nombre de polygones
	};

	class phUnk4 {
	public:
		uint32 m_VT; // 149A5700
	};

	class phUnk5 {
	public:
		uint32 m_VT; // 08675600
		uint32 m_Unk0;
		uint32 m_Unk1;
		uint32 m_Unk2; // 1
	};

	class phUnk2 {
	public:
		ptr32<phBoundPolyhedron> m_Unk0;
		ptr32<phUnk4> m_Unk1;
		ptr32<phUnk5> m_Unk2;
		uint32 m_Padding0;
	};


	class phBoundUnk0
	{
		ptr32<phBound> m_pUnk0;
		ptr32<phBound> m_pUnk1;
		ptr32<phBound> m_pUnk2;
	};


	// phBoundsDictionnary ?????
	class phUnk0
	{
	public:
		uint32 m_VT; // F4665600
		uint32 m_Unk0;
		uint32 m_Unk1;

		uint32 m_UnkCount;

		pgCollect<phUnk1> m_Unk2;
		pgCollect<ptr32<phBound>> m_Unk3;
	};

	class phBoundsDictionnary
	{
	public:

	};
}
