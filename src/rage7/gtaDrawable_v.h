#pragma once
#include "rage_fwrap.h"
#include "pgBase.h"
#include "grcTexture.h"
#include "crSkeleton.h"
#include "phBounds.h"
#include <functional>


namespace v
{
	namespace rage
	{
		using namespace ::rage;

		enum VertexType : uint32
		{
			eVT_Default = 89, //PNCT
			eVT_PNCCTX = 16505,
			eVT_PNCCTT = 249,
			eVT_PNCCTTTX = 17145,
			eVT_PT = 65,
		};

		struct {
			const char* m_Name;
			uint32 m_Value;
		} g_SEMANTIC_FLAGS_STR[] =
		{
			{ "eVT_Default" , 89 }, //PNCT
			{ "eVT_PNCCTX", 16505 },
			{ "eVT_PNCCTT", 249 },
			{ "eVT_PNCCTTTX", 17145 },
			{ "eVT_PT", 65 },
		};


#pragma pack(push, 1)
		struct RGBA {
			uint8 r, g, b, a;

			operator uint32() const { return *((uint32*)&r); }
			void operator=(uint32 val) {
				*((uint32*)&r) = val;
			}
		};

		struct VertexDefault
		{
			Vector3c Position;
			Vector3c Normal;
			RGBA Color;
			Vector2c Texcoord;
		};

		struct VertexPNCCTX
		{
			Vector3c Position;
			Vector3c Normal;
			RGBA Color;
			RGBA Color2;
			Vector2c Texcoord;
			Vector4c Tangent;
		};

		struct VertexPT
		{
			Vector3c Position;
			Vector2c Texcoord;
		};

		struct VertexPNCCTT
		{
			Vector3c Position;
			Vector3c Normal;
			RGBA Color;
			RGBA Color2;
			Vector2c Texcoord;
			Vector2c Texcoord2;
		};

		struct VertexPNCCTTTX
		{
			Vector3c Position;
			Vector3c Normal;
			RGBA Color;
			RGBA Color2;
			Vector2c Texcoord;
			Vector2c Texcoord2;
			Vector2c Texcoord3;
			Vector4c Tangent;
		};



		class VertexInfo
		{
		public:
			uint32 m_Flags;
			uint16 m_Stride;
			uint8 m_Unk0 = 0;
			uint8 m_Count = 0;
			uint64 m_VertexType = 0x7755555555996996;
		};


		class CLightAttr {
		public:
			uint64 m_VT = 0x14E9A5E2D1F;

		public:
			void Map(pgMap& map) {
			}
		};


		class grcVertexBuffer
		{
		public:
			uint64 m_VT = 0x1489953208;
			uint16 m_Stride;
			uint16 m_Unk1 = 0;
			uint32 m_Unk2 = 0;

			void* m_pVertexDataBlob;
			uint32 m_VertexCount;
			uint32 m_Unk3 = 0;
			void* m_pVertexDataBlob1 = 0;

			uint64 m_Unk4 = 0;

			VertexInfo* m_pVertexInfo;
			void* m_UnkData = 0;

			uint64 m_Unk5 = 0;
			uint64 m_Unk6 = 0;
			uint64 m_Unk7 = 0;
			uint64 m_Unk8 = 0;
			uint64 m_Unk9 = 0;
			uint64 m_Unk10 = 0;
			uint64 m_Unk11 = 0;
			uint64 m_Unk12 = 0;

			grcVertexBuffer() {};

		public:
			void Map(pgMap& map)
			{
				auto ptrList = map.Push(this);
				if (ptrList)
				{
					*ptrList << &m_pVertexDataBlob;
					*ptrList << &m_pVertexDataBlob1;
					*ptrList << &m_pVertexInfo;
					*ptrList << &m_UnkData;

					map.PushBlock((uint64)m_pVertexDataBlob, m_Stride * m_VertexCount, false);

					if (m_pVertexInfo)
						map.PushBlock((uint64)m_pVertexInfo, sizeof(VertexInfo));
				}
			}
		};

		class grcIndexBuffer
		{
		public:
			uint64 m_VT = 0x1462006668;
			uint32 m_IndicesCount;
			uint32 m_Unk1 = 0;
			uint16* m_pIndexDataBlob;

			uint64 m_Unk2 = 0;
			uint64 m_Unk3 = 0;
			uint64 m_Unk4 = 0;
			uint64 m_Unk5 = 0;
			uint64 m_Unk6 = 0;
			uint64 m_Unk7 = 0;
			uint64 m_Unk8 = 0;
			uint64 m_Unk9 = 0;
			uint64 m_Unk10 = 0;

		public:
			void Map(pgMap& map)
			{
				auto ptrList = map.Push(this);
				if (ptrList)
				{
					*ptrList << &m_pIndexDataBlob;
					map.PushBlock((uint64)m_pIndexDataBlob, m_IndicesCount * sizeof(uint16), false);
				}
			}
		};

		class grmGeometry // QB
		{
		public:
			uint64 m_VT = 0x14998723321;
			void* m_Unk0 = 0;
			uint64 m_Unk1 = 0;
			grcVertexBuffer* m_pVertexBuff;
			uint64 m_Unk2 = 0;
			uint64 m_Unk3 = 0;
			uint64 m_Unk4 = 0;
			grcIndexBuffer* m_pIndexBuff;

			uint32 m_Unk5 = 0;
			uint32 m_Unk6 = 0;
			uint32 m_Unk7 = 0;
			uint32 m_Unk8 = 0;
			uint32 m_Unk9 = 0;
			uint32 m_Unk10 = 0;

			uint32 m_IndicesCount = 0;
			uint32 m_TrianglesCount = 0;
			uint16 m_VertexCount;
			uint16 m_RasterizerState = 3; // 3 = Normal Render; 2 = Wireframe Render; etc...
			uint32 m_Unk13 = 0;
			uint16* m_BonesMapping = 0;
			uint16 m_Stride;
			uint16 m_BonesCount = 0;
			uint32 m_Unk15 = 0;
			void* m_pVertexDataBlob = 0;

			uint64 m_Unk16 = 0;
			uint64 m_Unk17 = 0;
			uint64 m_Unk18 = 0;
			// uint64 m_Unk19;

		public:
			void Map(pgMap& map)
			{
				auto ptrList = map.Push(this);
				if (ptrList)
				{
					*ptrList << &m_pVertexBuff;
					*ptrList << &m_pIndexBuff;
					*ptrList << &m_BonesMapping;
					*ptrList << &m_pVertexDataBlob;

					m_pVertexBuff->Map(map);
					m_pIndexBuff->Map(map);

					if (m_BonesMapping)
						map.PushBlock((uint64)m_BonesMapping, sizeof(uint16) * m_BonesCount);
				}
			}
		};


		class grmModel
		{
		public:
			uint64 m_VT = 0x149A7FD36;
			pgCollectionPtr<grmGeometry> m_GeoCollection;
			GeoBound* m_GeoBounds = 0;
			uint16* m_pShaderMapping = 0;
			uint32 m_SkeletonBind = 0;
			uint16 m_RenderMaskFlags = 255;
			uint16 m_ShaderMappingCount = 0;

		public:
			void Map(pgMap& map)
			{
				auto ptrList = map.Push(this);
				if (ptrList)
				{
					*ptrList << &m_GeoBounds;
					*ptrList << &m_pShaderMapping;

					if (m_GeoBounds)
					{
						uint16 geoCount = m_GeoCollection.GetCount();
						uint16 geoBoundsCount = ((geoCount > 1) ? geoCount + 1 : 1);
						map.PushBlock((uint64)m_GeoBounds, sizeof(GeoBound) * geoBoundsCount, false);
					}

					if (m_pShaderMapping)
						map.PushBlock((uint64)m_pShaderMapping, sizeof(uint16) * m_ShaderMappingCount, false);

					m_GeoCollection.Map(map, ptrList);
				}
			}
		};

		struct grmShaderParam
		{
			uint8 m_DataType = 0;
			uint8 m_RegisterIndex = 0;
			uint16 m_Unk3 = 0;
			uint32 m_Unk1 = 0;
			void* m_pData = 0;
		};

		class grmShaderParamsFactory
		{
		public:
			// tuple: paramHash, pData, dataSize, isOnExternalBlock(dont store the data just below the grmShaderParams blocks), registerIndex
			TinyAllocator* m_RootAllocator;
			SkyVector<SkyTuple<uint32, void*, uint32, bool, uint8>> m_Params;
			uint32 m_TexArgsCount = 0;
			uint32 m_ArgsCount = 0;
			
		public:
			grmShaderParamsFactory(TinyAllocator* allocator) : m_RootAllocator(allocator) {};


			template<typename T>
			void PushParameter(uint32 paramName, T* data, uint32 dataCount, uint8 registerIndex)
			{
				m_Params.push_back(std::make_tuple(paramName, (void*)data, sizeof(T) * dataCount, false, registerIndex));
				m_ArgsCount++;
			}

			void PushVector4(uint32 paramName, Vector4c data, uint16 registerIndex)
			{
				auto dataIn = m_RootAllocator->Alloc<Vector4c>();
				*dataIn = data;
				PushParameter(paramName, dataIn, 1, registerIndex);
			}

			void PushTexture(uint32 paramName, grcTextureBase* texBase, uint8 registerIndex)
			{
				m_Params.push_back(std::make_tuple(paramName, (void*)texBase, sizeof(grcTextureBase), true, registerIndex));
				m_ArgsCount++;
				m_TexArgsCount++;
			}

			uint32 GetTexArgsCount() const { return m_TexArgsCount; };
			uint32 GetArgsCount() const { return m_ArgsCount; };

			// construit un paramètre de shader et intègre les données dans le bloc grmShader courant
			v::rage::grmShaderParam* BuildParametersBlock(uint16& parametersDataSize, uint16& parametersSize)
			{
				uint32 paramsListSize = (m_Params.size() * sizeof(v::rage::grmShaderParam));
				uint32 hashsListSize = (m_Params.size() * sizeof(uint32));
				parametersSize = paramsListSize;

				uint32 blockSize = paramsListSize + hashsListSize;
				for (const auto& i : m_Params) {
					if (!std::get<3>(i))
						blockSize += std::get<2>(i);
				}

				// alignement de 128bits nécessaire des blocs
				if (blockSize % 16 != 0)
					blockSize += (16 - (blockSize % 16));

				parametersDataSize = 32 + blockSize;

				v::rage::grmShaderParam* buf = (v::rage::grmShaderParam*)m_RootAllocator->Alloc(parametersDataSize);
				char* dataBuf = ((char*)buf) + paramsListSize;

				uint32 it = 0;
				for (const auto& i : m_Params) {
					buf[it] = v::rage::grmShaderParam();
					buf[it].m_RegisterIndex = std::get<4>(i); // registre GPU (index de banque)

					bool isTex = std::get<3>(i);
					if (isTex) {
						buf[it].m_DataType = 0;
						buf[it].m_pData = std::get<1>(i); // rage::grcTextureBase*
					}
					else {
						// gère les Vector4 seuls ou les matrices 4x4
						uint8 vectorsCount = (std::get<2>(i) / sizeof(Vector4c));
						buf[it].m_DataType = vectorsCount;
						uint32 dataSize = sizeof(Vector4c) * vectorsCount;
						parametersSize += dataSize;

						buf[it].m_pData = dataBuf;

						void* iData = std::get<1>(i);
						memcpy(dataBuf, iData, dataSize);

						dataBuf += dataSize;
					}

					it++;
				}

				for (const auto& i : m_Params) {
					*((uint32*)dataBuf) = std::get<0>(i);
					dataBuf += sizeof(uint32);
				}

				return buf;
			}
		};



		class grmShader
		{
		public:
			grmShaderParam* m_pShaderParameters = 0;

			uint32 m_ShaderHash = 0; // emissive, spec

			uint32 m_Unk1 = 0;

			uint8 m_ParamsCount = 0;
			uint8 m_DrawBucket = 0;
			uint16 m_Unk10 = 32768;

			uint16 m_ParameterSize = 0;
			uint16 m_ParameterDataSize = 0;

			uint32 m_FilenameHash = 0; // emissive.sps, spec.sps
			uint32 m_Unk5 = 0;
			uint32 m_RenderBucketMask = 65281; // 65344
			uint16 m_Unk7 = 0;
			uint8 m_Unk8 = 0;
			uint8 m_TexParamsCount = 0;
			uint64 m_Unk9 = 0;

		public:
			void Map(pgMap& map)
			{
				auto ptrList = map.Push(this);
				*ptrList << &m_pShaderParameters;

				if (m_pShaderParameters)
				{
					auto pList = map.PushBlock((uint64)m_pShaderParameters, m_ParameterDataSize - 32);
					for (int i = 0; i < m_ParamsCount; i++) {
						*pList << &m_pShaderParameters[i].m_pData;
					}

					for (int i = 0; i < m_ParamsCount; i++) {
						if (m_pShaderParameters[i].m_DataType == 0 && m_pShaderParameters[i].m_pData) {
							((grcTextureBase*)m_pShaderParameters[i].m_pData)->Map(map);
						}
					}
				}
			}

			// 6, 1 etc...
			void SetBucket(uint8 drawBucket)
			{
				m_DrawBucket = drawBucket;
				m_RenderBucketMask = ((1 << drawBucket) | 0xFF00);
			}

			void ForeachTextures(std::function<void(v::rage::grcTextureBase*)> fn)
			{
				for (int i = 0; i < m_ParamsCount; i++)
				{
					if (m_pShaderParameters[i].m_DataType == 0)
						fn((v::rage::grcTextureBase*)m_pShaderParameters[i].m_pData);
				}
			}
		};

		class grmShaderGroup
		{
		public:
			uint64 m_VT = 0x14FF6A2D5E3;
			grcTextureDictionnary* m_pTexDictionnary = 0;
			pgCollectionPtr<grmShader> m_ShadersCollection;
			pgCollectionPtr<int, false> m_Unk0;
			uint64 m_Unk1 = 0;
			uint64 m_Unk2 = 0;

			uint64 m_Unk3 = 0;
			uint64 m_Unk4 = 0;

		public:
			void Map(pgMap& map)
			{
				auto ptrList = map.Push(this);
				if (ptrList)
				{
					*ptrList << &m_pTexDictionnary;

					if (m_pTexDictionnary)
						m_pTexDictionnary->Map(map);

					m_ShadersCollection.Map(map, ptrList);
					m_Unk0.Map(map, ptrList);
				}
			}
		};



		class gtaDrawableBase
		{
		public:
			rage::grmShaderGroup* m_pShaderGroup = 0;
			rage::crSkeletonData* m_pSkeleton = 0;

			Vector3c m_BoundCenter;
			float m_SphereRadius;
			Vector4c m_BBoxMin;
			Vector4c m_BBoxMax;

			rage::pgCollectionPtr<grmModel>* m_ModelsHigh = 0;
			rage::pgCollectionPtr<grmModel>* m_ModelsMedium = 0;
			rage::pgCollectionPtr<grmModel>* m_ModelsLow = 0;
			rage::pgCollectionPtr<grmModel>* m_ModelsExtraLow = 0;

			float m_LodDistHigh = 0.f;
			float m_LodDistMed = 0.f;
			float m_LodDistLow = 0.f;
			float m_LodDistVeryLow = 0.f;

			int m_RenderFlagHigh = 0;
			int m_RenderFlagMed = 0;
			int m_RenderFlagLow = 0;
			int m_RenderFlagVeryLow = 0;

			uint64 m_Unk7 = 0;
			uint16 m_Unk0 = 0;
			uint16 m_Unk1 = 9998;
			uint32 m_Unk2 = 0;

			rage::pgCollectionPtr<grmModel>* m_AllModels = 0;

			char* m_Name = 0;
			rage::pgCollection<CLightAttr> m_LightsAttr;

			void* m_Unk3 = 0;
			rage::phBoundComposite* m_Bounds = 0;


		public:
			void BaseMap(pgPtrList* ptrList, pgMap& map)
			{
				if (ptrList)
				{
					*ptrList << &m_pShaderGroup;
					*ptrList << &m_ModelsHigh;
					*ptrList << &m_ModelsMedium;
					*ptrList << &m_ModelsLow;
					*ptrList << &m_ModelsExtraLow;
					*ptrList << &m_AllModels;
					*ptrList << &m_Name;

					if (m_pShaderGroup)
						m_pShaderGroup->Map(map);

					if (m_ModelsHigh)
						m_ModelsHigh->Map(map);

					if (m_ModelsMedium)
						m_ModelsMedium->Map(map);

					if (m_ModelsLow)
						m_ModelsLow->Map(map);

					if (m_ModelsExtraLow)
						m_ModelsExtraLow->Map(map);

					if (m_AllModels)
						m_AllModels->Map(map);

					if (m_Name)
						map.PushBlock((uint64)m_Name, strlen(m_Name) + 1);
				}
			}
		};



		class gtaDrawable : public gtaDrawableBase, public pgBase<false>
		{
		public:
			void Map(pgMap& map)
			{
				auto ptrList = map.Push(this);
				BaseMap(ptrList, map);
			}
		};

		class gtaDrawablePG : public gtaDrawableBase, public pgBase<true>
		{
		public:
			void Map(pgMap& map)
			{
				auto ptrList = map.Push(this);
				PushPageInfo(ptrList);
				BaseMap(ptrList, map);
			}
		};



		class gtaDrawableDictionnary : public ::rage::pgBase<true>
		{
		public:
			uint64 m_Unk0 = 0;
			uint64 m_Unk1 = 0;
			pgCollection<uint32, false> m_DrawablesHashs;
			pgCollectionPtr<gtaDrawable> m_Drawables;

		public:
			void Map(pgMap& map)
			{
				auto ptrList = map.Push(this);
				PushPageInfo(ptrList);

				m_DrawablesHashs.Map(map, ptrList);
				m_Drawables.Map(map, ptrList);
			}
		};

#pragma pack(pop)
	}
}
