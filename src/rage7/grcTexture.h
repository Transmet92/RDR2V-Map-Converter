#pragma once
#include "rage_fwrap.h"
#include "DDS.h"
#include "pgBase.h"

using namespace rage;

namespace v
{
	namespace rage
	{
		enum TexUsage : uint8
		{
			eDEFAULT = 1,
			eTERRAIN = 2,
			eDIFFUSE = 20,
			eDETAIL = 21,
			eNORMAL = 22,
			eSPECULAR = 23,
			eEMISSIVE = 24,
		};

		enum TextureFormat : uint32
		{
			eD3DFMT_A8R8G8B8 = 21,
			eD3DFMT_A1R5G5B5 = 25,
			eD3DFMT_A8 = 28,
			eD3DFMT_A8B8G8R8 = 32,
			eD3DFMT_L8 = 50,

			eD3DFMT_DXT1 = 0x31545844,
			eD3DFMT_DXT3 = 0x33545844,
			eD3DFMT_DXT5 = 0x35545844,
			eD3DFMT_ATI1 = 0x31495441,
			eD3DFMT_ATI2 = 0x32495441,
			eD3DFMT_BC7 = 0x20374342,
		};

#pragma pack(push, 1)
		class grcTextureBase
		{
		public:
			// uint64 m_VT = 0x14D5FFFFFF;
			uint32 m_Unk0 = 0;
			uint32 m_Unk1 = 0;
			uint32 m_Unk2 = 0;
			uint32 m_Unk3 = 0;
			uint32 m_Unk4 = 0;
			uint32 m_Unk5 = 0;
			uint32 m_Unk6 = 0;
			uint32 m_Unk7 = 0;
			const char* m_pName = 0;
			uint16 m_Unk8 = 1; // 1
			uint16 m_Unk9 = 128; // 2
			uint32 m_Unk10 = 0;
			uint32 m_Unk11 = 0;
			uint32 m_Unk12 = 0;
			uint32 m_UsageData = 0;
			uint32 m_Unk13 = 0;
			uint32 m_ExtraFlags = 0;
			uint32 m_Unk14 = 0;

		public:
			grcTextureBase() {
				SetUsage(TexUsage::eDEFAULT);
			}

			void SetUsage(uint32 val) {
				m_UsageData = (m_UsageData & 0xFFFFFFE0) + (((uint32)val) & 0x1F);
			}

			void SetUsageFlags(uint32 val) {
				m_UsageData = (m_UsageData & 0x1F) + (((uint32)val) << 5);
			}

			void SetAsExternal() {
				m_Unk8 = 1;
				m_Unk9 = 2;
			}

			void SetAsInternal() {
				m_Unk8 = 1;
				m_Unk9 = 128;
			}

			uint32 GetUsage() const { return m_UsageData & 0x1F; };
			uint32 GetUsageFlags() const { return m_UsageData >> 5; }


			void MapBase(pgPtrList* pList, pgMap& map)
			{
				*pList << &m_pName;

				if (m_pName)
					map.PushString(m_pName);
			}

			virtual void Map(pgMap& map)
			{
				auto pList = map.Push(this);
				if (pList)
					MapBase(pList, map);
			}
		};

		class grcTexture : public grcTextureBase
		{
		public:
			uint16 m_Width = 0;
			uint16 m_Height = 0;
			uint16 m_Depth = 0;
			uint16 m_Stride = 0; // représente la quantité de données sur une seule ligne
			uint32 m_Format = 0; // fourcc (DXT5, DXT1 etc...)
			uint8 m_Unk0 = 0;
			uint8 m_Levels = 0;
			uint16 m_Unk1 = 0;
			uint32 m_Unk2 = 0;
			uint32 m_Unk3 = 0;
			uint32 m_Unk4 = 0;
			uint32 m_Unk5 = 0;
			void* m_pData = 0;
			uint32 m_Unk6 = 0;
			uint32 m_Unk7 = 0;
			uint32 m_Unk8 = 0;
			uint32 m_Unk9 = 0;
			uint32 m_Unk10 = 0;
			uint32 m_Unk11 = 0;

		public:
			virtual void Map(pgMap& map) override
			{
				auto ptrList = map.Push(this);
				if (ptrList)
				{
					*ptrList << &m_pName;
					*ptrList << &m_pData;

					if (m_pName)
						map.PushString(m_pName);

					if (m_pData)
					{
						int fullLength = 0;
						int length = m_Stride * m_Height;
						for (int i = 0; i < m_Levels; i++) {
							fullLength += length;
							length /= 4;
						}

						map.PushBlock((uint64)m_pData, fullLength, true);
					}
				}
			}
		};



		class grcTextureDictionnaryBase
		{
		public:
			uint64 m_Unk0 = 0;
			uint32 m_Unk1 = 1;
			uint32 m_Unk2 = 0;
			pgCollection<uint32, false> m_TexturesHashs;
			pgCollectionPtr<grcTexture> m_Textures;

		public:
			void BaseMap(pgPtrList* ptrList, pgMap& map)
			{
				if (ptrList)
				{
					m_TexturesHashs.Map(map, ptrList);
					m_Textures.Map(map, ptrList);
				}
			}
		};

		// Use grcTextureDictionnaryPG  for root grcTextureDictionnary (include pgInfoPage)
		class grcTextureDictionnary : public grcTextureDictionnaryBase, public pgBase<false>
		{
		public:
			void Map(pgMap& map)
			{
				auto ptrList = map.Push(this);
				BaseMap(ptrList, map);
			}
		};

		class grcTextureDictionnaryPG : public grcTextureDictionnaryBase, public pgBase<true>
		{
		public:
			void Map(pgMap& map)
			{
				auto ptrList = map.Push(this);
				PushPageInfo(ptrList);
				BaseMap(ptrList, map);
			}
		};
#pragma pack(pop)
	}
}
