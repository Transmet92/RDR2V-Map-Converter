#pragma once
#include "rage_fwrap.h"
#include "grcTexture_rdr.h"
#include "float16.h"
#include "rdrShadersList.h"
#include "grmShadersParamsList.h"
#include "gtaDrawable_v.h"
#include "D3D9.h"

typedef uint16_t float16;

struct Vector4_half {
	float16 x;
	float16 y;
	float16 z;
	float16 w;
};


/*
	vtables:

	6D00
	6E00

	- 2CC96D00 (rmcDrawable)
	- 50C06D00 (real candidat for grmModel)
	- 0CAE6D00 (real candidat for grmGeometry)
	- 64226E00 (real candidat for grmShaderGroup)
	- C47D6E00 (grcVertexBuffer)
	- AC7A6E00 (grcIndexBuffer)

	/////// - 409A6D00 (rmcDrawable)

	2820
*/
namespace rdr
{
	namespace rage
	{
		enum VertexType : uint64 {
			eVT_DEFAULT = 0xAA1111111199A996,
			eVT_OTHER01 = 0xAAEEEEEEEE99A996,
		};

		struct VertexBuffer
		{
			SkyVector<uint8> m_VertexBuff;
			uint32 m_VertexStride;

			uint32 m_VertexFlags;
			VertexType m_VertexType;
		};

		struct IndexBuffer
		{
			SkyVector<uint16> m_IndexBuff;
			uint32 m_IndexCount;
		};


		enum ShaderArgType {
			eSP_TEX_RESS,
			eSP_VECTOR4,
			eSP_UNK,
		};

		struct ShaderArg
		{
			ShaderArgType m_Type;
			std::string m_ParamName = "";
			
			std::string m_TexName = "";
			SkyVector<Vector4c> m_Value;
		};

		struct Shader
		{
			SkyVector<ShaderArg> m_Parameters;
			std::string m_ShaderName;

			uint32 m_TexsCount = 0;
			uint32 m_OtherParamsCount = 0;

			const ShaderArg* OwnParam(const SkyString& paramName) const {
				for (const auto& i : m_Parameters) {
					if (i.m_ParamName == paramName) {
						return &i;
					}
				}
				return 0;
			}
		};


		struct Geometry {
			VertexBuffer m_VertexBuffer;
			IndexBuffer m_IndexBuffer;
			uint32 m_VertexCount;
			uint32 m_TrianglesCount;
			Shader m_Shader;
		};


		struct Model {
			SkyVector<Geometry> m_Geometry;
			SkyVector<GeoBound> m_GeoBoundaries;
		};

		struct Drawable {
			Vector3c m_BoundCenter;
			float m_SphereRadius;
			Vector4c m_BoundsMin;
			Vector4c m_BoundsMax;
			uint32 m_Hash;
			bool m_IsAtPositionZero; // suppose que c'est un prop dans la très grande majorité des cas

			SkyVector<Model> m_Models[3]; // 0 = HIGH ?   1 = LOW ?
			SkyString m_Name;
		};


		struct Drawables {
			SkyVector<SkySharedPtr<Drawable>> m_Drawables;
			TXD m_EmbedTex;
		};


		/*
			Red Dead Redemption 1
				DRAWABLE
		*/
	#pragma pack(push, 1)

		// VTable: 64C37000  (CANDIDAT)
		struct phBound { };

		// VTable: 50606D00
		struct phBoundsCollection { };

		struct grcVertexBufferInfo {
			uint16 m_Unk0;
			uint16 m_Unk1;
			uint32 m_Unk2;
			uint32 m_Unk3;
			uint32 m_Unk4;
			uint32 m_Unk5;
			uint32 m_Unk6;
			uint16 m_Unk7;
			uint16 m_Unk8;
			uint16 m_Unk9;
			uint16 m_VertexBufferSize;
		};

		enum VertexFlags
		{
			XYZ, // +12

			eVF_XYZ = (1 << 0),  /* + 4*3 */

			eVF_UnkA = (1 << 1),  /* + 4 */
			eVF_UnkB = (1 << 2),  /* + 4 */
			eVF_UnkC = (1 << 3),  /* + 4 */

			eVF_Colour = (1 << 4),  /* + 4 */

			eVF_UnkD = (1 << 5),  /* + 4 */

			eVF_UV0 = (1 << 6),  /* + 4 */
			eVF_UV1 = (1 << 7),  /* + 4 */

			eVF_UnkX = (1 << 14), /* + 4 */
		};

		struct grcVertexInfo {
			uint32 m_VertexFlags; // use VertexFlags

			uint8 m_VertexStride;
			uint8 m_Unk0;
			uint8 m_Unk1;
			uint8 m_DataCount; // number of data types in the vertex

			VertexType m_VertexType; // usual : AA 11 11 11 11 99 A9 96
			// terrain : AA EE EE EE EE 99 A9 96
		};


		// VTable: C47D6E00 | 445D6E00 | 24B66C00
		struct grcVertexBuffer
		{
			uint32 m_VTable;

			uint16 m_VertexCount;
			uint16 m_Unk1;

			ptr32<uint8> m_VertexBuffer; // GPU

			uint16 m_Unk6; // +12
			uint16 m_StrideSize; // size of one vertex data

			ptr32<uint8> m_VertexBuffer1; // GPU (same that above)

			uint32 m_Unk2; // +18

			ptr32<grcVertexInfo> m_pVertexInfo; // CPU (interesting)
			ptr32<grcVertexBufferInfo> m_Unk4; // CPU
		};

		struct grcIndexBufferSub
		{
			uint16 m_Unk0;
			uint16 m_Unk1; // size of elem (ushort index = 2)
			uint32 m_Unk2;
			uint32 m_Unk3;
			uint32 m_Unk4;
			uint32 m_Unk5;
			uint16 m_Unk6; // max of index ? (0xFFFF)
			uint16 m_Unk7;
			ptr32<uint16> m_IndexBuffer; // GPU
			uint32 m_BufferSize; // in bytes
		};

		// VTable: AC7A6E00 | 4C8E6C00
		struct grcIndexBuffer
		{
			uint32 m_VTable;
			uint32 m_IndexCount;
			ptr32<uint16> m_IndexBufferData; // GPU
			ptr32<grcIndexBufferSub> m_IndexBuffer;
		};

		// VTable: 0CAE6D00 | 4C9E6D00 | 34A16C00
		struct grmGeometry
		{
			uint32 m_VTable;
			uint32 m_Unk0;
			uint32 m_Unk1;
			ptr32<grcVertexBuffer> m_VertexBuffer;
			uint32 m_Unk2;
			uint32 m_Unk3;
			uint32 m_Unk4;
			ptr32<grcIndexBuffer> m_IndexBuffer;
			uint32 m_Unk5;
			uint32 m_Unk6;
			uint32 m_Unk7;
			uint32 m_IndexesCount;
			uint32 m_TrianglesCount;
			uint16 m_VertexCount;
			uint16 m_Unk9;
			uint32 m_Unk10;
			uint16 m_VertexStrideCAND;
			uint16 m_Unk11;

			// Pas sûr
			ptr32<void*> m_pGPUVertexBuffer;
			uint32 m_Unk12;
			uint32 m_Unk13;
			uint32 m_Unk14; // valeur interessante
		};

		struct grmUnk0 {
			Vector4c m_Unk0;
		};
		struct grmUnk1 {
			uint16 m_Unk0[7];
		};

		// VTable: 50C06D00 | 70B06D00 | 70226C00 | 70226C00
		struct grmModel
		{
			uint32 m_VTable;
			pgCollect<ptr32<grmGeometry>> m_Geometries;
			ptr32<GeoBound> m_GeoBounds;
			ptr32<uint16> m_ShadersIndex; // shaders map index array
			uint32 m_Unk2;
			uint16 m_Unk3;
			uint8 m_Unk4;
			uint8 m_ShadersCount; // shaders map count

			uint32 m_Pad0; // unassigned
		};

		struct ShaderTexArg {
			uint32 m_Unk0;
			uint32 m_Unk1;
			uint32 m_Unk2; // usual: 1
			uint32 m_Unk3; // eg: 131072
			uint32 m_Unk4;
			uint32 m_Unk5;
			ptr32<char*> m_TextureName;
			uint32 m_Unk6;
		};

		struct ShaderUnk0Arg {
			float m_Data[36];
		};


		typedef Vector4c ShaderVecArg;

		struct ShaderParam {
			uint8 m_DataType; // 0: Texture, 1: Vector4, 9: 36 x float (unk)
			uint8 m_Unk0;
			uint16 m_Unk1; // eg: 256, 0
			ptr32<> m_pShaderParam;
		};

		struct ShaderInfo {
			ptr32<ShaderParam> m_ParamsList; // [CPU]  list of struct (look below for the count)
			uint32 m_ShaderHash;
			uint8 m_ParamsCount;
			uint8 m_Unk1;
			uint16 m_Unk2;
			uint16 m_ParameterSize;
			uint16 m_ParameterDataSize;

			uint32 m_Unk4;
			uint32 m_Unk5;
			
			uint16 m_Unk6;
			uint8 m_Unk10;
			uint8 m_TexsCount; // candidat

			uint32 m_Unk7; // hash of anything ?

			// uint32 m_Unk8;
			// uint16 m_Unk9;
		};
	

		// VTable: F4116C00 | 409A6D00 | 14626C00
		struct grmShaderGroup
		{
			uint32 m_VT; // 0x409A6D00
			uint32 m_Unk0;
			pgCollect<ptr32<ShaderInfo>> m_ShaderGroup; // [CPU]
		};

		// rmcDrawableDictionnary VT :  0C486D00



		/*
			Si m_ShaderGroup et m_modelsCollect  sont null il s'agit probablement
			d'un rage::fragDrawable
		*/
		// VTable: 2CC96D00 | 24A96D00 | 34A96D00 | 84137200 | 44B96D00 |
		// [fragDrawable: A4407000 | 0C927000 | 8CA27000 | 34B57000 | 04B67000
		//    | 84A47000 | 44917000 | 54B97000 | 5CB97000 | 84B97000 | ECB97000]
		struct rmcDrawable
		{
			uint32 m_VTable;
			uint32 m_Unk0; // another shader group ?  look at 82AD7E48
			ptr32<grmShaderGroup> m_ShaderGroup;
			ptr32<void> m_FragCandidat;
		 
			Vector3c m_BoundCenter;
			float m_SphereRadius;
			Vector4c m_BoundsMin;
			Vector4c m_BoundsMax;

			ptr32<pgCollect<ptr32<grmModel>>> m_modelsCollect;
			ptr32<pgCollect<ptr32<grmModel>>> m_modelsCollect1;
		};

	
		// VTables: E0147200 | 68EE7100 | 60187200 | 58EE7100 | 40FF7100 | 48127200 | C8017200 | 38187200 | 48FF7100
		//			10027200 | F8127200 | C0017200 | C8157200 | 38187200 | D0287200 | 88FF7100 | C0EE7100

		const uint32 RDRDrawableVTables[] = {
			0xE0147200, 0x68EE7100, 0x60187200, 0x58EE7100,
			0x40FF7100, 0x48127200, 0xC8017200, 0x38187200,
			0x48FF7100, 0x10027200, 0xF8127200, 0xC0017200,
			0xC8157200, 0x38187200, 0xD0287200, 0x88FF7100,
			0xC0EE7100
		};
		struct rdrDrawable {
			uint32 m_VT;
			uint32 m_Unk2;
			uint32 m_Unk3;
			uint32 m_Unk4; // usual : 1
			pgCollect<uint32> m_DrawablesHashs; // candidat
			pgCollect<ptr32<rmcDrawable>> m_Drawables;
			ptr32<grcTextureDictionnary> m_TexDict; // embed
		};

		// VTable: 74187200 (just before rdrDrawable +8 header)

		/*
		struct grmShaderGroup {
			uint32 m_VT; // 64226E00
			uint32 m_Unk0;
			uint32 m_Unk1;
			uint32 m_Unk2;
			uint32 m_Unk3;
			uint32 m_Unk4;
			ptr32<char*> m_texName;
		};
		*/

		enum VertexSemantic : uint32 {
			eVS_POS = (1 << 0),
			eVS_UNK0 = (1 << 1),
			eVS_UNK1 = (1 << 2),
			eVS_UNK2 = (1 << 3), // used (maybe 2x float16)
			eVS_COLOR1 = (1 << 4),
			eVS_COLOR2 = (1 << 5),
			eVS_UV1 = (1 << 6),
			eVS_UV2 = (1 << 7),
			eVS_UNK3 = (1 << 8), // used (2x float16)
		};

		struct VertexBase {
			float x;
			float y;
			float z;

			// uint32 m_ColorCand0;
			// uint32 m_ColorCand1; // usually 0xFFFFFFFF
		};


		struct UV {
			float16 u;
			float16 v;
		};


		// STRIDE 12 :  OSEF
		struct Vertex12
		{
		};


		// STRIDE 16
		struct Vertex16 : VertexBase
		{
			uint32 m_Unk0;
			// UV m_UV;
		};

		// STRIDE 20
		struct Vertex20 : VertexBase
		{
			uint32 m_Color0;
			UV m_UV;
		};

		// STRIDE 24
		struct Vertex24 : VertexBase
		{
			uint32 m_Color0CAND;
			UV m_UV;
			UV m_UV1;
		};

		// STRIDE 28
		struct Vertex28 : VertexBase
		{
			uint32 m_Unk0; // Color0 ??  high entropy values
			uint32 m_Color0;
			
			UV m_UV;

			// not sure (90% du temps c'est des valeurs float16, 10% du temps c'est NaN)
			float16 m_V2;
			float16 m_V3;
		};

		// STRIDE 32
		struct Vertex32 : VertexBase
		{
			UV m_Unk0; // NOT FLOAT16 VALUES (incohérentes)

			uint32 m_Color0;

			UV m_UV;
			UV m_UV1;

			// NOT FLOAT16 Below (valeurs incohérentes)
			float16 m_Unk55;
			float16 m_Unk56;
			// uint32 m_Unk2; // very probably two float16
		};

		// STRIDE 36
		struct Vertex36 : VertexBase
		{
			UV m_Unk0; // NOT FLOAT16 VALUES (incohérentes)

			uint32 m_Color0;

			UV m_Unk1;
			UV m_Unk2;

			// NOT FLOAT16 Below (valeurs incohérentes)
			float16 m_Unk55;
			float16 m_Unk56;

			float16 m_Unk57;
			float16 m_Unk58;
			// uint32 m_Unk2; // very probably two float16
		};

		// STRIDE 40
		struct Vertex40 : VertexBase
		{
			UV m_Unk0;

			uint32 m_Color0;

			UV m_Unk1;
			UV m_Unk2;
			UV m_Unk3;
			UV m_Unk4;

			UV m_Unk5;
		};

		// STRIDE 32
		struct VertexUnk1 {
			float x;
			float y;
			float z;
			uint32 m_Unk0;
			uint32 m_Unk1; // usual : 0xFFFFFFFF

			float16 m_Unk2; // float16 UV ?
			float16 m_Unk3;

			float16 m_Unk4;
			float16 m_Unk5;

			uint32 m_Unk6;
		};

		// STRIDE 36
		struct VertexUnk2 {
			float x;
			float y;
			float z;
			uint32 m_Unk0;
			int m_Unk1; // usually -1

			float16 m_Unk2; // float16
			float16 m_Unk3; // float16
			float16 m_Unk4; // float16
			float16 m_Unk5; // float16

			// high proba for UV
			float16 m_Unk6; // float16
			float16 m_Unk7; // float16

			uint32 m_Unk8;
		};

		// STRIDE = 40
		struct VertexUnk3 {
			float x;
			float y;
			float z;

			uint32 m_Unk0;
			int m_Unk1; // usually -1

			// UV, Vector3_16 of Normal coords below ??
			float16 m_HFloat0;
			float16 m_HFloat1;
			float16 m_HFloat2;
			float16 m_HFloat3;
			float16 m_HFloat4;

			float16 m_UV0;
			float16 m_UV1;

			uint16 m_Unk7;
			uint16 m_Unk8;
		};


		struct VertexUnk0 {
			uint32 m_Unk0;
			uint16 m_Unk1;
			uint16 m_Unk2;
			uint16 m_Unk3;
			uint16 m_Unk4;
			float t;
			float x;
			float y;
			float z;
			uint16 m_Unk5;
			uint16 m_Unk6;
		};
	#pragma pack(pop)



		static SkyVector<uint32> g_VT;

		const bool g_SHOW_SHADERS = false;
		const bool g_SHOW_VERTEX_DATA = false;
		const bool g_SHOW_INDEX_DATA = false;

		static SkyAtomic<uint64> g_IDDrawable = 0;

		static uint32 g_DrawablesVTables[32]{0};


		static int ParseDrawable(
			Drawables& drawablesOut,
			rmcDrawable* pDrawable,
			uint32 drawableHash,
			char* sys, int sysSize,
			char* gfx, int gfxSize)
		{
			rmcDrawable* iD = pDrawable;

			if (iD->m_ShaderGroup.m_Offset)
			{
				uint32 memFlag = (iD->m_ShaderGroup.m_Offset & 0xF0);
				if (memFlag != 0x50)
				{
					MessageBoxA(0, "Pointer to a not CPU mem", "", 0);
					return 0;
				}
			}
			else
				return 0;

			if (iD->m_modelsCollect.m_Offset == 0 && iD->m_modelsCollect1.m_Offset == 0)
			{
				return 0;
			}
			else
			{
				if (iD->m_modelsCollect.m_Offset)
				{
					if ((iD->m_modelsCollect.m_Offset & 0xF0) != 0x50)
					{
						MessageBoxA(0, "Pointer to a not CPU mem", "", 0);
						return 0;
					}
				}

				if (iD->m_modelsCollect1.m_Offset)
				{
					if ((iD->m_modelsCollect1.m_Offset & 0xF0) != 0x50)
					{
						MessageBoxA(0, "Pointer to a not CPU mem", "", 0);
						return 0;
					}
				}
			}


			Vector3c iCenter = iD->m_BoundCenter;
			Vector4c AABBMin = iD->m_BoundsMin;
			Vector4c AABBMax = iD->m_BoundsMax;


			SkySharedPtr<Drawable> pNewDrawable = SkySharedPtr<Drawable>(new Drawable());
			Drawable& newDrawable = *pNewDrawable.get();


			// BOUNDS MAX
			newDrawable.m_BoundsMax.x = InvEnd(AABBMax.x);
			newDrawable.m_BoundsMax.z = InvEnd(AABBMax.y);
			newDrawable.m_BoundsMax.y = InvEnd(AABBMax.z);
			newDrawable.m_BoundsMax.w = InvEnd(AABBMax.w);
			newDrawable.m_BoundsMax.FlipXY();

			// BOUNDS MIN
			newDrawable.m_BoundsMin.x = InvEnd(AABBMin.x); newDrawable.m_BoundsMin.z = InvEnd(AABBMin.y);
			newDrawable.m_BoundsMin.y = InvEnd(AABBMin.z); newDrawable.m_BoundsMin.w = InvEnd(AABBMin.w);
			newDrawable.m_BoundsMin.FlipXY();

			// BOUNDS CENTER
			newDrawable.m_BoundCenter.x = InvEnd(iCenter.x); newDrawable.m_BoundCenter.z = InvEnd(iCenter.y);
			newDrawable.m_BoundCenter.y = InvEnd(iCenter.z);
			newDrawable.m_BoundCenter.FlipXY();



			newDrawable.m_SphereRadius = InvEnd(iD->m_SphereRadius);

			newDrawable.m_Hash = drawableHash;


			newDrawable.m_IsAtPositionZero =
				DistToZeroIsOk(newDrawable.m_BoundCenter.x, 8.f) &&
				DistToZeroIsOk(newDrawable.m_BoundCenter.y, 8.f) &&
				DistToZeroIsOk(newDrawable.m_BoundCenter.z, 8.f);

			// printf("Center: %.4f, %.4f, %.4f\nBoxMin: %.4f, %.4f, %.4f\nBoxMax: %.4f, %.4f, %.4f\n\n",
			// 	newDrawable.m_BoundCenter.x, newDrawable.m_BoundCenter.y, newDrawable.m_BoundCenter.z,
			// 	newDrawable.m_BoundsMin.x, newDrawable.m_BoundsMin.y, newDrawable.m_BoundsMin.z,
			// 	newDrawable.m_BoundsMax.x, newDrawable.m_BoundsMax.y, newDrawable.m_BoundsMax.z
			// );


			// 1ST MODEL COLLECTION (HIGH ?)
			pgCollect<ptr32<grmModel>>* iModelsCollect = 0;

			for (int tt = 0; tt < 2; tt++)
			{
				if (tt == 0)
				{
					if (iD->m_modelsCollect.m_Offset)
						iModelsCollect = iD->m_modelsCollect.Get(sys, gfx);
					else continue;
				}
				else if (tt == 1)
				{
					if (iD->m_modelsCollect1.m_Offset)
						iModelsCollect = iD->m_modelsCollect1.Get(sys, gfx);
					else break;
				}

				if (iModelsCollect->m_Off.Get())
				{
					for (int y = 0; y < iModelsCollect->GetCount(); y++)
					{
						auto iModel = iModelsCollect->GetAt(y, sys, gfx)->Get(sys, gfx);

						Model newModel;

						grmShaderGroup* pShaderGroup = iD->m_ShaderGroup.Get(sys, gfx);
						uint16* iModShaderList = iModel->m_ShadersIndex.Get(sys, gfx);
						auto iGeometries = iModel->m_Geometries;

						// CONVERT AND COPY GEOBOUNDS OF MODEL
						uint32 geoBoundsCount = iGeometries.GetCount() == 1 ? 1 : iGeometries.GetCount();
						GeoBound* pGeoBounds = iModel->m_GeoBounds.Get(sys, gfx);
						for (int xr = 0; xr < geoBoundsCount; xr++)
						{
							pGeoBounds[xr].ConvertRDR2V();
							newModel.m_GeoBoundaries.push_back(pGeoBounds[xr]);
						}

						// REBUILD GEOMETRIES
						for (int u = 0; u < iGeometries.GetCount(); u++)
						{
							auto iGeo = iGeometries.GetAt(u, sys, gfx)->Get(sys, gfx);

							Geometry newGeometry;

							// REBUILD SHADER OF GEOMETRY
							uint16 geoShaderId = InvEnd(iModShaderList[u]);
							rage::ShaderInfo* pGeoShader = pShaderGroup->m_ShaderGroup.GetAt(geoShaderId, sys, gfx)->Get(sys, gfx);

							newGeometry.m_Shader.m_ShaderName = atSingleton<CShaderHashMap>::GetRef()[InvEnd(pGeoShader->m_ShaderHash)];

							auto pUShaderParams = pGeoShader->m_ParamsList.Get(sys, gfx);
							uint64 pHashParamsBlock = (uint64)pUShaderParams;
							if (pUShaderParams)
							{
								int paramsCount = InvEnd(pGeoShader->m_ParamsCount);
								pHashParamsBlock += sizeof(ShaderParam) * paramsCount;

								for (int r = 0; r < paramsCount; r++)
								{
									auto pRShaderParam = pUShaderParams[r].m_pShaderParam.Get(sys, gfx);
									{
										ShaderArg newShaderArg;
										if (pUShaderParams[r].m_DataType == 0)
										{
											newShaderArg.m_Type = eSP_TEX_RESS;

											newGeometry.m_Shader.m_TexsCount++;
											auto pTexArg = (ShaderTexArg*)pRShaderParam;

											char* texNamePtr = (char*)"NULL_TEX";
											if (pTexArg)
												texNamePtr = (char*)pTexArg->m_TextureName.Get(sys, gfx);

											newShaderArg.m_TexName = std::string(texNamePtr);

											std::transform(
												newShaderArg.m_TexName.begin(),
												newShaderArg.m_TexName.end(),
												newShaderArg.m_TexName.begin(),
												[](unsigned char c) { return std::tolower(c); }
											);
										}
										else
										{
											newGeometry.m_Shader.m_OtherParamsCount++;

											newShaderArg.m_Type = eSP_VECTOR4;
											if (pRShaderParam)
											{
												auto pVecArg = (Vector4c*)pRShaderParam;
												for (int rr = 0; rr < pUShaderParams[r].m_DataType; rr++)
												{
													Vector4c newVecRR;
													newVecRR.x = InvEnd(pVecArg->x);
													newVecRR.y = InvEnd(pVecArg->y);
													newVecRR.z = InvEnd(pVecArg->z);
													newVecRR.w = InvEnd(pVecArg->w);
													newShaderArg.m_Value.push_back(newVecRR);
												}
											}

											pHashParamsBlock += pUShaderParams[r].m_DataType * sizeof(Vector4c);

											////// printf("\t\t{ %.2f, %.2f, %.2f, %.2f }\n",
											////// 	newShaderArg.m_Value.x, newShaderArg.m_Value.y,
											////// 	newShaderArg.m_Value.z, newShaderArg.m_Value.w
											////// );
										}
										newGeometry.m_Shader.m_Parameters.push_back(newShaderArg);
									}
								}
							}

							// ignore strange padding (risk)
							for (; *((uint32*)pHashParamsBlock) == 0; pHashParamsBlock += 4) {}



							// SHOW SHADER INFO
							{
								if (g_SHOW_SHADERS)
									printf("\n\n\tSHADER USED : %s\n\tParameters :\n\n", atSingleton<CShaderHashMap>::GetRef()[InvEnd(pGeoShader->m_ShaderHash)]);

								int ri = 0;
								for (auto& r : newGeometry.m_Shader.m_Parameters)
								{
									// resolve hash param name
									uint32 hashParam = ((uint32*)pHashParamsBlock)[ri];
									char* nameParam = (char*)"NOT_RESOLVED";
									for (int ii = 0; ii < ARRAYSIZE(g_ShadersParamsList); ii++) {
										if (g_ShadersParamsList[ii].Hash == hashParam) {
											nameParam = (char*)g_ShadersParamsList[ii].Name;
											break;
										}
									}

									// if (strstr(nameParam, "OT_RESOLVE"))
									// 	Sleep(1);

									r.m_ParamName = nameParam;

									if (g_SHOW_SHADERS)
										printf("\t\t%s\n", nameParam);

									if (g_SHOW_SHADERS)
									{
										if (r.m_Type == ShaderArgType::eSP_TEX_RESS)
										{
											printf("\t\t\t{ %s }\n", r.m_TexName.c_str());
										}
										else
										{
											for (const auto& ii : r.m_Value)
												printf("\t\t\t{ %.2f, %.2f, %.2f, %.2f }\n", ii.x, ii.y, ii.z, ii.w);
										}
									}

									if (g_SHOW_SHADERS)
										printf("\n");

									ri++;
								}

								if (g_SHOW_SHADERS)
									printf("\n\n");
							}

							auto iVertexBuff = iGeo->m_VertexBuffer.Get(sys, gfx);
							newGeometry.m_VertexCount = InvEnd(iVertexBuff->m_VertexCount);
							newGeometry.m_TrianglesCount = InvEnd(iGeo->m_TrianglesCount);
							grcVertexInfo* vertInfo = iVertexBuff->m_pVertexInfo.Get(sys, gfx);


							auto iIndexBuffRoot = iGeo->m_IndexBuffer.Get(sys, gfx);
							auto iIndexBuff = iIndexBuffRoot->m_IndexBuffer.Get(sys, gfx);



							// INDEXES ENUMERATION
							uint32 indexCount = InvEnd(iIndexBuffRoot->m_IndexCount);
							newGeometry.m_IndexBuffer.m_IndexBuff.resize(indexCount);

							// SkyPrint->print("\nVB GPU: %p\nIndexes Count: %u\n%s\n", InvEnd(iIndexBuff->m_IndexBuffer.m_Offset), indexCount, (indexCount % 3 == 0) ? "IS A GOOD VB:" : "IS NOT A GOOD VB:");

							newGeometry.m_IndexBuffer.m_IndexCount = indexCount;
							uint16* indexBuf = (uint16*)iIndexBuff->m_IndexBuffer.Get(sys, gfx);
							for (int t = 0; t < indexCount; t++)
							{
								newGeometry.m_IndexBuffer.m_IndexBuff[t] = InvEnd(indexBuf[t]);

								// if (t < 12)
								// 	SkyPrint->print("%u  ", newGeometry.m_IndexBuffer.m_IndexBuff[t]);
							}
							// SkyPrint->print("\n\n\n");

							// if (indexCount % 3 != 0)
							// 	printf("ILLEGAL INDEX COUNT ([%u %u] MOD 3 != 0) [GPU: %u]\n", indexCount, InvEnd(iIndexBuff->m_BufferSize) / 2, iIndexBuffRoot->m_IndexBuffer.Get());
							// else
							// 	printf("LEGA INDEX COUNT %u [GPU: %u]\n", indexCount, iIndexBuffRoot->m_IndexBuffer.Get());

							// VERTEX ENUMERATION
							auto& outVertexBuf = newGeometry.m_VertexBuffer.m_VertexBuff;
							newGeometry.m_VertexBuffer.m_VertexStride = InvEnd(iVertexBuff->m_StrideSize);
							uint8* vertexBuf = (uint8*)iVertexBuff->m_VertexBuffer.Get(sys, gfx);
							uint32 vertexBufSize = newGeometry.m_VertexCount * newGeometry.m_VertexBuffer.m_VertexStride;
							outVertexBuf.insert(outVertexBuf.end(), vertexBuf, vertexBuf + vertexBufSize);

							newGeometry.m_VertexBuffer.m_VertexFlags = vertInfo->m_VertexFlags;
							newGeometry.m_VertexBuffer.m_VertexType = InvEnd(vertInfo->m_VertexType);


							// CHECK VERTEX POSITION BOUNDARIES FLOAT
							if (false)
							{
								if (newGeometry.m_VertexBuffer.m_VertexStride > 12)
								{
									uint8* pVert = vertexBuf;
									for (int tt = 0; tt < newGeometry.m_VertexCount; tt++)
									{
										Vector3c thisVec = *(Vector3c*)pVert;
										Vector3c clean = {};
										clean.x = InvEnd(thisVec.x);
										clean.y = InvEnd(thisVec.y);
										clean.z = InvEnd(thisVec.x);

										if (!((clean.x > -6500.f && clean.x < 6500.f) &&
											(clean.y > -6500.f && clean.y < 6500.f) &&
											(clean.z > -6500.f && clean.z < 6500.f)))
										{
											printf("ERROR VERTEX: %p { %.6f, %.6f, %.6f }\n", pVert, clean.x, clean.y, clean.z);
										}

										pVert += newGeometry.m_VertexBuffer.m_VertexStride;
									}
								}
							}



							// SHOW VERTEX TYPE INFORMATIONS
							if (false)
							{
								uint32 vertFlags = 0;
								grcVertexInfo* vertInfo = iVertexBuff->m_pVertexInfo.Get(sys, gfx);

								std::string iFilePath = "C:\\Users\\Unknown8192\\Desktop\\DUMP_UV\\";
								char bufP[64];
								sprintf_s(bufP, "0x%02hhX_%u_%u", newGeometry.m_VertexBuffer.m_VertexStride, newGeometry.m_VertexBuffer.m_VertexStride, g_IDDrawable.load());
								iFilePath += bufP;

								// HANDLE iFile = CreateFileA(iFilePath.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS,
								// 	FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0);
								HANDLE iFile = 0;

								printf("\n\n\nID: %u\nStride: %u\nVertexInfo: ", g_IDDrawable.load(), newGeometry.m_VertexBuffer.m_VertexStride);
								if (vertInfo)
								{
									printHex(*vertInfo);
									printf("VertexFlag: ");

									vertFlags = (vertInfo->m_VertexFlags);
									uint32 vertFlagsLittle = InvEnd(vertInfo->m_VertexFlags);
									for (int ua = 0; ua < 32; ua++)
										printf((vertFlagsLittle & (1 << ua)) ? "1" : "0");

									for (int eer = 0; eer < ARRAYSIZE(v::rage::g_SEMANTIC_FLAGS_STR); eer++) {
										if (v::rage::g_SEMANTIC_FLAGS_STR[eer].m_Value == vertFlagsLittle) {
											printf("\nVertexType: %s\n", v::rage::g_SEMANTIC_FLAGS_STR[eer].m_Name);
											break;
										}
									}
								}
								printf("\n\n");

								g_IDDrawable++;


								for (int t = 0; t < newGeometry.m_VertexCount; t++)
								{
									uint8* piV = vertexBuf + (t * newGeometry.m_VertexBuffer.m_VertexStride);

									DWORD writtt = 0;
									WriteFile(iFile, piV, newGeometry.m_VertexBuffer.m_VertexStride, &writtt, 0);

									if (t < 16)
									{
										const uint32 vertexDataSize[] = {
											2, 4, 6, 8, 4, 8, 12, 16,
											4, 4, 4, 0, 0, 0, 4, 8
										};
										for (uint32 tt = 0; tt < 16; tt++)
										{
											uint32 iMaskFlag = InvEnd(1 << tt);
											if ((iMaskFlag & vertFlags) == iMaskFlag)
											{
												uint64 tti = InvEnd(vertInfo->m_VertexType);
												// HIDWORD(tti) = 0xFBB7A43F;

												uint64 tti0 = tti >> (4 * tt);
												uint64 pTT = ((4 * tti0) & 0x3C);

												int iDataSize = *(int*)((char*)vertexDataSize + pTT);

												// printf("%u ", iDataSize);

												uint32 indexInDataSize = (uint32)pTT / 4;
												printf("%u { ", indexInDataSize);

												if (indexInDataSize == 6)
												{
													Vector3c* ipPos = (Vector3c*)piV;
													printf("%.2f, %.2f, %.2f ", InvEnd(ipPos->x), InvEnd(ipPos->y), InvEnd(ipPos->z));
												}
												else if (indexInDataSize == 14)
												{
													uint16* ipUV = (uint16*)piV;

													uint16 uv[2];
													uv[0] = (ipUV[0]);
													uv[1] = (ipUV[1]);

													// const uint16 offsetBit = 0;
													// float divider = (float)(IntPower(2, 16 - offsetBit) - 1);

													uv[0] = InvEnd(uv[0]);
													uv[1] = InvEnd(uv[1]);

													// uv[0] <<= offsetBit;
													// uv[0] >>= offsetBit;
													// 
													// uv[1] <<= offsetBit;
													// uv[1] >>= offsetBit;

													printf("%.6f, %.6f   ", ((float)uv[0]) / 65535.f, ((float)uv[1]) / 65535.f);

													printf("%u, %u   ", uv[0], uv[1]);

													printHexBlob(uv, sizeof(uv), 0);

													printf("  ");
													printBits(&uv[0], sizeof(uv[0]));

													printf(" ");
													printBits(&uv[1], sizeof(uv[1]));

													printf(" ");

													// for (int i = 0; i < 16; i++)
													// 	printf((InvEnd(ipUV[0]) & (1 << i)) == (1 << i) ? "1" : "0");
													// 
													// printf(", ");
													// 
													// for (int i = 0; i < 16; i++)
													// 	printf((InvEnd(ipUV[1]) & (1 << i)) == (1 << i) ? "1" : "0");

													// printf(" ");
													// printf("%.2f, %.2f ", Float16To32(InvEnd(ipUV->u)), Float16To32(InvEnd(ipUV->v)));
												}
												else if (indexInDataSize == 1000)
												{
													UV* ipUV = (UV*)piV;
													printf("%.2f, %.2f ", Float16To32(InvEnd(ipUV->u)), Float16To32(InvEnd(ipUV->v)));
												}
												else if (indexInDataSize == 100)
												{
													UV* ipUV = (UV*)piV;
													printf("%.2f, %.2f ", Float16To32(InvEnd(ipUV->u)), Float16To32(InvEnd(ipUV->v)));
												}
												else
													printHexBlob(piV, iDataSize, 0);

												printf("}    ");
												piV += iDataSize;
											}
										}
										printf("\n");

										continue;



										if (vertFlags & InvEnd(eVS_POS))
										{
											Vector3c* ipPos = (Vector3c*)piV;
											printf("POS { %.2f, %.2f, %.2f }   ", InvEnd(ipPos->x), InvEnd(ipPos->y), InvEnd(ipPos->z));
											piV += sizeof(Vector3c);
										}

										if (vertFlags & InvEnd(eVS_UNK0))
										{
											piV += 4;
										}

										if (vertFlags & InvEnd(eVS_UNK1))
										{
											piV += 4;
										}

										if (vertFlags & InvEnd(eVS_UNK2))
										{
											piV += 4;
										}

										if (vertFlags & InvEnd(eVS_COLOR1))
										{
											printf("COLOR1 { ");
											printHex(*(uint32*)piV, 0);
											printf("}   ");
											piV += 4;
										}

										if (vertFlags & InvEnd(eVS_COLOR2))
										{
											printf("COLOR2 { ");
											printHex(*(uint32*)piV, 0);
											printf("}   ");
											piV += 8;
										}

										if (vertFlags & InvEnd(eVS_UV1))
										{
											UV* ipUV = (UV*)piV;
											printf("UV1 { %.2f, %.2f } ", Float16To32(InvEnd(ipUV->u)), Float16To32(InvEnd(ipUV->v)));
											printf("{ ");
											printHex(ipUV->u, 0);
											printf(", ");
											printHex(ipUV->v, 0);
											printf("}    ");
											piV += sizeof(UV);

											// DWORD writtt = 0;
											// WriteFile(iFile, ipUV, sizeof(UV), &writtt, 0);
										}

										if (vertFlags & InvEnd(eVS_UV2))
										{
											UV* ipUV = (UV*)piV;
											printf("UV2 { %.2f, %.2f } ", Float16To32(InvEnd(ipUV->u)), Float16To32(InvEnd(ipUV->v)));
											printf("{ ");
											printHex(ipUV->u, 0);
											printf(", ");
											printHex(ipUV->v, 0);
											printf("}    ");
											piV += sizeof(UV);

											// DWORD writtt = 0;
											// WriteFile(iFile, ipUV, sizeof(UV), &writtt, 0);
										}

										if (vertFlags & InvEnd(eVS_UNK3))
										{
											UV* ipUV = (UV*)piV;
											printf("UNK_HYPOTYPE { %.2f, %.2f }   ", Float16To32(InvEnd(ipUV->u)), Float16To32(InvEnd(ipUV->v)));
											piV += sizeof(UV);
										}
										printf("\n");




										if (false)
										{
											auto pVector3 = (Vector3c*)piV;

											int written = printf("%.2f, %.2f, %.2f  ",
												InvEnd(pVector3->x), InvEnd(pVector3->y), InvEnd(pVector3->z));

											for (int tt = 0; tt < 32 - written; tt++)
												printf(" ");

											printHexBlob(piV + sizeof(Vector3c), newGeometry.m_VertexBuffer.m_VertexStride - sizeof(Vector3c));
										}
									}
								}
								printf("\n\n\n");

								FlushFileBuffers(iFile);
								CloseHandle(iFile);
							}



							// SHOW VERTEX DATA
							if (false)
							{
								uint16 stride = newGeometry.m_VertexBuffer.m_VertexStride;
								uint16 count = newGeometry.m_VertexCount;

								if (stride == 40)
								{
									int countToShow = count > 50 ? 50 : count;
									for (int vi = 0; vi < countToShow; vi++)
									{
										uint8* iVertexPtr = vertexBuf + (vi * stride);

										VertexBase* iVBase = ((VertexBase*)iVertexPtr);
										printf("%u: POS { %.2f, %.2f, %.2f }  ", stride, InvEnd(iVBase->x), InvEnd(iVBase->y), InvEnd(iVBase->z));



										// UV Missing
										if (stride == 16)
										{
											auto v16 = (Vertex16*)iVBase;
											// printf("Color0 { ");
											// printHex(v16->m_Unk0, 0);
											// printf(" }  ");
											// 
											// printf("Color1 { ");
											// printHex(v16->m_Unk1, 0);
											// printf(" }  ");

											printHex(*(uint32*)(((uint8*)v16) + 12), 0);
											// printf("UV0 { %.2f %.2f }  ", Float16To32(InvEnd(v16->m_UV.u)), Float16To32(InvEnd(v16->m_UV.v)));
										}


										else if (stride == 20)
										{
											auto v20 = (Vertex20*)iVBase;
											printHex(*(uint32*)(((uint8*)v20) + 12), 0);
											printf("  UV0 { %.2f %.2f }  ", Float16To32(InvEnd(v20->m_UV.u)), Float16To32(InvEnd(v20->m_UV.v)));
										}
										else if (stride == 24)
										{
											auto v24 = (Vertex24*)iVBase;
											printf("Color0 { ");
											printHex(*(uint32*)(((uint8*)v24) + 12), 0);
											printf(" }  ");

											printf("UV0 { %.2f %.2f }  ", Float16To32(InvEnd(v24->m_UV.u)), Float16To32(InvEnd(v24->m_UV.v)));
											printf("UV1 { %.2f %.2f }  ", Float16To32(InvEnd(v24->m_UV1.u)), Float16To32(InvEnd(v24->m_UV1.v)));
										}



										// UV missing
										else if (stride == 28)
										{
											auto v28 = (Vertex28*)iVBase;
											printf("Color0 { ");
											printHex(*(uint32*)(((uint8*)v28) + 12), 0);
											printf(" }  ");

											// printf("UV0 { %.2f %.2f }  ", Float16To32(InvEnd(v28->m_UV.u)), Float16To32(InvEnd(v28->m_UV.v)));

											printf("Color1 { ");
											printHex(*(uint32*)(((uint8*)v28) + 16), 0);
											printf(" }  ");

											// printf("UNK_VEC { %.2f, %.2f, %.2f, %.2f }  ",
											// 	Float16To32(InvEnd(v28->m_V0)),
											// 	Float16To32(InvEnd(v28->m_V1)),
											// 	Float16To32(InvEnd(v28->m_V2)),
											// 	Float16To32(InvEnd(v28->m_V3))
											// );
										}


										else if (stride == 32)
										{
											auto v32 = (Vertex32*)iVBase;

											// TO RESOLVE !
											// printf("UNK_VEC { %.2f, %.2f }  ",
											// 	Float16To32(InvEnd(v32->m_Unk0.u)),
											// 	Float16To32(InvEnd(v32->m_Unk0.v))
											// );

											printf("Color0 { ");
											printHex(*(uint32*)(((uint8*)v32) + 16), 0);
											printf(" }  ");

											printf("UV0 { %.2f %.2f }  ", Float16To32(InvEnd(v32->m_UV.u)), Float16To32(InvEnd(v32->m_UV.v)));
											printf("UV1 { %.2f %.2f }  ", Float16To32(InvEnd(v32->m_UV1.u)), Float16To32(InvEnd(v32->m_UV1.v)));

											// TO RESOLVE !
											// printf("UV2? { %.2f, %.2f }  ",
											// 	Float16To32(InvEnd(v32->m_Unk55)),
											// 	Float16To32(InvEnd(v32->m_Unk56))
											// );
										}


										else if (stride == 36)
										{
											auto v36 = (Vertex36*)iVBase;

											printf("UNK_VEC { %.2f, %.2f }  ",
												Float16To32(InvEnd(v36->m_Unk0.u)),
												Float16To32(InvEnd(v36->m_Unk0.v))
											);

											printf("Color0 { ");
											printHex(*(uint32*)(((uint8*)v36) + 16), 0);
											printf(" }  ");

											printf("UV0 { %.2f %.2f }  ", Float16To32(InvEnd(v36->m_Unk1.u)), Float16To32(InvEnd(v36->m_Unk1.v)));
											printf("UV1 { %.2f %.2f }  ", Float16To32(InvEnd(v36->m_Unk2.u)), Float16To32(InvEnd(v36->m_Unk2.v)));


											printf("UV2? { %.2f, %.2f }  ",
												Float16To32(InvEnd(v36->m_Unk55)),
												Float16To32(InvEnd(v36->m_Unk56))
											);
											printf("UV3? { %.2f, %.2f }  ",
												Float16To32(InvEnd(v36->m_Unk57)),
												Float16To32(InvEnd(v36->m_Unk58))
											);
										}


										else if (stride == 40)
										{
											auto v40 = (Vertex40*)iVBase;

											printf("UNK2 { %.2f, %.2f }  ",
												Float16To32(InvEnd(v40->m_Unk0.u)),
												Float16To32(InvEnd(v40->m_Unk0.v))
											);

											printf("Color0 { ");
											printHex(*(uint32*)(((uint8*)v40) + 16), 0);
											printf(" }  ");


											printf("UV0 { %.2f %.2f }  ", Float16To32(InvEnd(v40->m_Unk1.u)), Float16To32(InvEnd(v40->m_Unk1.v)));
											printf("UV1 { %.2f %.2f }  ", Float16To32(InvEnd(v40->m_Unk2.u)), Float16To32(InvEnd(v40->m_Unk2.v)));

											printf("UV2? { %.2f, %.2f }  ",
												Float16To32(InvEnd(v40->m_Unk3.u)),
												Float16To32(InvEnd(v40->m_Unk3.v))
											);
											printf("UV3? { %.2f, %.2f }  ",
												Float16To32(InvEnd(v40->m_Unk4.u)),
												Float16To32(InvEnd(v40->m_Unk4.v))
											);


											printf("UNK3 { %.2f, %.2f }  ",
												Float16To32(InvEnd(v40->m_Unk5.u)),
												Float16To32(InvEnd(v40->m_Unk5.v))
											);
										}



										printf("\n");
									}

									printf("\n\n\n\n\n");
								}

							}



							newModel.m_Geometry.push_back(newGeometry);
						}

						newDrawable.m_Models[tt].push_back(newModel);
					}
				}
			}

			drawablesOut.m_Drawables.push_back(pNewDrawable);

			return 0;
		}




		static int LoadDrawable(Drawables& drawablesOut, const char* filename,
			char* sys, int sysSize,
			char* gfx, int gfxSize,
			bool onlyTexDict = false)
		{
			atSingleton<CShaderHashMap>::GetRef().Init();


			if (g_DrawablesVTables[0] == 0)
			{
				for (int i = 0; i < ARRAYSIZE(RDRDrawableVTables); i++)
					g_DrawablesVTables[i] = InvEnd(RDRDrawableVTables[i]);
			}


			try
			{
				rdrDrawable* iDrawable = 0;
				for (int64 i = 0; i < sysSize - 4; i += 4)
				{
					uint32 magic_vt = *(uint32*)(sys + i);

					bool found = false;
					for (int i = 0; i < ARRAYSIZE(RDRDrawableVTables); i++) {
						if (magic_vt == g_DrawablesVTables[i]) {
							found = true;
							break;
						}
					}

					if (found)
					{
						if (*(uint8*)(sys + i + 4) == 0x00)
						{
							iDrawable = (rdrDrawable*)(sys + i);
							break;
						}
					}
				}

				// bool pass = true;
				// for (auto i : g_VT) {
				// 	if (i == magic_vt) {
				// 		pass = false;
				// 		break;
				// 	}
				// }
				// 
				// if (pass)
				// {
				// 	printHex(magic_vt, 0);
				// 	printf("  : %s\n", filename);
				// 	g_VT.push_back(magic_vt);
				// }

				if (iDrawable)
				{
					auto pRdrDraw = (rdrDrawable*)iDrawable;
					auto pDrawables = pRdrDraw->m_Drawables;


					bool hasTexDict = false;
					if (pRdrDraw->m_TexDict.m_Offset)
					{
						drawablesOut.m_EmbedTex = ReadTextureDict(pRdrDraw->m_TexDict.Get(sys, gfx), sys, sysSize, gfx, gfxSize);
						hasTexDict = true;
					}

					if (onlyTexDict)
						return hasTexDict ? 1 : 0;



					for (int i = 0; i < pDrawables.GetCount(); i++)
					{
						uint32 iHash = 0;

						if (i < pRdrDraw->m_DrawablesHashs.GetCount())
							iHash = *pRdrDraw->m_DrawablesHashs.GetAt(i, sys, gfx);

						rmcDrawable* pDrawable = pDrawables.GetAt(i, sys, gfx)->Get(sys, gfx);
						ParseDrawable(drawablesOut, pDrawable, iHash, sys, sysSize, gfx, gfxSize);
					}
				}
			}
			catch (std::exception& e) {
				printf("error try/catch for file!");
			}

			return drawablesOut.m_Drawables.size();
		}
	}
}