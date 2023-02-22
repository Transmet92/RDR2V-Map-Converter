#pragma once
#include "rage_fwrap.h"
#include "DDS.h"
#include "SkyCommons/SkyTuple.h"
#include "SkyCommons/SkyCommons.h"
#include <Windows.h>
#include "CTexMatcher.h"
#include <algorithm>


using namespace DirectX;

#pragma pack(push, 1)
struct DDSHead
{
	uint32 m_magic = '\x44\x44\x53\x20';
	DDS_HEADER m_DDS;
};
#pragma pack(pop)

enum {
	DDSD_CAPS = 0x1,
	DDSD_HEIGHT = 0x2,
	DDSD_WIDTH = 0x4,
	DDSD_PITCH = 0x8,
	DDSD_PIXELFORMAT = 0x1000,
	DDSD_MIPMAPCOUNT = 0x20000,
	DDSD_LINEARSIZE = 0x80000,
	DDSD_DEPTH = 0x800000
};

enum TEX_FORMATS_ENUM : uint16
{
	eGPUTEXTUREFORMAT_1_REVERSE,
	eGPUTEXTUREFORMAT_1,
	eGPUTEXTUREFORMAT_8,
	eGPUTEXTUREFORMAT_1_5_5_5,
	eGPUTEXTUREFORMAT_5_6_5,
	eGPUTEXTUREFORMAT_6_5_5,
	eGPUTEXTUREFORMAT_8_8_8_8,
	eGPUTEXTUREFORMAT_2_10_10_10,
	eGPUTEXTUREFORMAT_8_A,
	eGPUTEXTUREFORMAT_8_B,
	eGPUTEXTUREFORMAT_8_8,
	eGPUTEXTUREFORMAT_Cr_Y1_Cb_Y0_REP,
	eGPUTEXTUREFORMAT_Y1_Cr_Y0_Cb_REP,
	eGPUTEXTUREFORMAT_16_16_EDRAM,
	eGPUTEXTUREFORMAT_8_8_8_8_A,
	eGPUTEXTUREFORMAT_4_4_4_4,
	eGPUTEXTUREFORMAT_10_11_11,
	eGPUTEXTUREFORMAT_11_11_10,
	eGPUTEXTUREFORMAT_DXT1,
	eGPUTEXTUREFORMAT_DXT2_3,
	eGPUTEXTUREFORMAT_DXT4_5,
	eGPUTEXTUREFORMAT_16_16_16_16_EDRAM,
	eGPUTEXTUREFORMAT_24_8,
	eGPUTEXTUREFORMAT_24_8_FLOAT,
	eGPUTEXTUREFORMAT_16,
	eGPUTEXTUREFORMAT_16_16,
	eGPUTEXTUREFORMAT_16_16_16_16,
	eGPUTEXTUREFORMAT_16_EXPAND,
	eGPUTEXTUREFORMAT_16_16_EXPAND,
	eGPUTEXTUREFORMAT_16_16_16_16_EXPAND,
	eGPUTEXTUREFORMAT_16_FLOAT,
	eGPUTEXTUREFORMAT_16_16_FLOAT,
	eGPUTEXTUREFORMAT_16_16_16_16_FLOAT,
	eGPUTEXTUREFORMAT_32,
	eGPUTEXTUREFORMAT_32_32,
	eGPUTEXTUREFORMAT_32_32_32_32,
	eGPUTEXTUREFORMAT_32_FLOAT,
	eGPUTEXTUREFORMAT_32_32_FLOAT,
	eGPUTEXTUREFORMAT_32_32_32_32_FLOAT,
	eGPUTEXTUREFORMAT_32_AS_8,
	eGPUTEXTUREFORMAT_32_AS_8_8,
	eGPUTEXTUREFORMAT_16_MPEG,
	eGPUTEXTUREFORMAT_16_16_MPEG,
	eGPUTEXTUREFORMAT_8_INTERLACED,
	eGPUTEXTUREFORMAT_32_AS_8_INTERLACED,
	eGPUTEXTUREFORMAT_32_AS_8_8_INTERLACED,
	eGPUTEXTUREFORMAT_16_INTERLACED,
	eGPUTEXTUREFORMAT_16_MPEG_INTERLACED,
	eGPUTEXTUREFORMAT_16_16_MPEG_INTERLACED,
	eGPUTEXTUREFORMAT_DXN,
	eGPUTEXTUREFORMAT_8_8_8_8_AS_16_16_16_16,
	eGPUTEXTUREFORMAT_DXT1_AS_16_16_16_16,
	eGPUTEXTUREFORMAT_DXT2_3_AS_16_16_16_16,
	eGPUTEXTUREFORMAT_DXT4_5_AS_16_16_16_16,
	eGPUTEXTUREFORMAT_2_10_10_10_AS_16_16_16_16,
	eGPUTEXTUREFORMAT_10_11_11_AS_16_16_16_16,
	eGPUTEXTUREFORMAT_11_11_10_AS_16_16_16_16,
	eGPUTEXTUREFORMAT_32_32_32_FLOAT,
	eGPUTEXTUREFORMAT_DXT3A,
	eGPUTEXTUREFORMAT_DXT5A,
	eGPUTEXTUREFORMAT_CTX1,
	eGPUTEXTUREFORMAT_DXT3A_AS_1_1_1_1,
	eGPUTEXTUREFORMAT_8_8_8_8_GAMMA_EDRAM,
	eGPUTEXTUREFORMAT_2_10_10_10_FLOAT_EDRAM,
};

enum PixelFormat {
	ePX_DXT1,
	ePX_DXT3,
	ePX_DXT5,
	ePX_A8R8G8B8,
	ePX_L8
};

enum {
	DDPF_ALPHAPIXELS = 0x1,
	DDPF_ALPHA = 0x2,
	DDPF_FOURCC = 0x4,
	DDPF_RGB = 0x40,
	DDPF_YUV = 0x200,
	DDPF_LUMINANCE = 0x20000
};

enum {
	DDSCAPS_COMPLEX = 0x8,
	DDSCAPS_MIPMAP = 0x400000,
	DDSCAPS_TEXTURE = 0x1000
};

enum {
	DDSCAPS2_CUBEMAP = 0x200,
	DDSCAPS2_CUBEMAP_POSITIVEX = 0x400,
	DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800,
	DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000,
	DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000,
	DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000,
	DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
	DDSCAPS2_VOLUME = 0x200000
};




namespace rdr
{
	namespace rage
	{
		enum UsageTex : uint16 {
			eSO = 2,
			eDIFFUSE = 6,
			eNORMAL = 5,
		};

		struct Texture
		{
			uint32 m_Hash;

			std::string m_Texname;
			SkySharedPtr<char[]> m_Buffer = 0;
			uint32 m_BufferSize = 0;
			uint32 m_Stride;

			TEX_FORMATS_ENUM m_Format;
			PixelFormat m_PxFormat;

			uint16 m_Width = 0;
			uint16 m_Height = 0;
			uint16 m_Mips;

			UsageTex m_Usage;

			Texture() = default;
		};


		struct TXD
		{
			SkyVector<Texture> m_Texs;

			
			bool GetTextureByMatcher(Texture& texOut, const CTexMatcher& matcher, uint16 width = 0, uint16 height = 0)
			{
				for (auto& i : m_Texs)
				{
					if (i.m_Hash == matcher.m_TexHash || i.m_Texname == matcher.m_Texname)
					{
						if (width && height) {
							if (i.m_Width != width && i.m_Height != height)
								continue;
						}

						texOut = i;
						return true;
					}
				}

				return false;
			}
		};




		static const char* TEX_FORMAT_STR[] =
		{
			"GPUTEXTUREFORMAT_1_REVERSE",
			"GPUTEXTUREFORMAT_1",
			"GPUTEXTUREFORMAT_8",
			"GPUTEXTUREFORMAT_1_5_5_5",
			"GPUTEXTUREFORMAT_5_6_5",
			"GPUTEXTUREFORMAT_6_5_5",
			"GPUTEXTUREFORMAT_8_8_8_8",
			"GPUTEXTUREFORMAT_2_10_10_10",
			"GPUTEXTUREFORMAT_8_A",
			"GPUTEXTUREFORMAT_8_B",
			"GPUTEXTUREFORMAT_8_8",
			"GPUTEXTUREFORMAT_Cr_Y1_Cb_Y0_REP",
			"GPUTEXTUREFORMAT_Y1_Cr_Y0_Cb_REP",
			"GPUTEXTUREFORMAT_16_16_EDRAM",
			"GPUTEXTUREFORMAT_8_8_8_8_A",
			"GPUTEXTUREFORMAT_4_4_4_4",
			"GPUTEXTUREFORMAT_10_11_11",
			"GPUTEXTUREFORMAT_11_11_10",
			"GPUTEXTUREFORMAT_DXT1",
			"GPUTEXTUREFORMAT_DXT2_3",
			"GPUTEXTUREFORMAT_DXT4_5",
			"GPUTEXTUREFORMAT_16_16_16_16_EDRAM",
			"GPUTEXTUREFORMAT_24_8",
			"GPUTEXTUREFORMAT_24_8_FLOAT",
			"GPUTEXTUREFORMAT_16",
			"GPUTEXTUREFORMAT_16_16",
			"GPUTEXTUREFORMAT_16_16_16_16",
			"GPUTEXTUREFORMAT_16_EXPAND",
			"GPUTEXTUREFORMAT_16_16_EXPAND",
			"GPUTEXTUREFORMAT_16_16_16_16_EXPAND",
			"GPUTEXTUREFORMAT_16_FLOAT",
			"GPUTEXTUREFORMAT_16_16_FLOAT",
			"GPUTEXTUREFORMAT_16_16_16_16_FLOAT",
			"GPUTEXTUREFORMAT_32",
			"GPUTEXTUREFORMAT_32_32",
			"GPUTEXTUREFORMAT_32_32_32_32",
			"GPUTEXTUREFORMAT_32_FLOAT",
			"GPUTEXTUREFORMAT_32_32_FLOAT",
			"GPUTEXTUREFORMAT_32_32_32_32_FLOAT",
			"GPUTEXTUREFORMAT_32_AS_8",
			"GPUTEXTUREFORMAT_32_AS_8_8",
			"GPUTEXTUREFORMAT_16_MPEG",
			"GPUTEXTUREFORMAT_16_16_MPEG",
			"GPUTEXTUREFORMAT_8_INTERLACED",
			"GPUTEXTUREFORMAT_32_AS_8_INTERLACED",
			"GPUTEXTUREFORMAT_32_AS_8_8_INTERLACED",
			"GPUTEXTUREFORMAT_16_INTERLACED",
			"GPUTEXTUREFORMAT_16_MPEG_INTERLACED",
			"GPUTEXTUREFORMAT_16_16_MPEG_INTERLACED",
			"GPUTEXTUREFORMAT_DXN",
			"GPUTEXTUREFORMAT_8_8_8_8_AS_16_16_16_16",
			"GPUTEXTUREFORMAT_DXT1_AS_16_16_16_16",
			"GPUTEXTUREFORMAT_DXT2_3_AS_16_16_16_16",
			"GPUTEXTUREFORMAT_DXT4_5_AS_16_16_16_16",
			"GPUTEXTUREFORMAT_2_10_10_10_AS_16_16_16_16",
			"GPUTEXTUREFORMAT_10_11_11_AS_16_16_16_16",
			"GPUTEXTUREFORMAT_11_11_10_AS_16_16_16_16",
			"GPUTEXTUREFORMAT_32_32_32_FLOAT",
			"GPUTEXTUREFORMAT_DXT3A",
			"GPUTEXTUREFORMAT_DXT5A",
			"GPUTEXTUREFORMAT_CTX1",
			"GPUTEXTUREFORMAT_DXT3A_AS_1_1_1_1",
			"GPUTEXTUREFORMAT_8_8_8_8_GAMMA_EDRAM",
			"GPUTEXTUREFORMAT_2_10_10_10_FLOAT_EDRAM",
		};




	#pragma pack(push, 1)
		struct grcTextureFormat
		{
			uint32 m_Unk0;
			uint32 m_Unk1;
			uint32 m_Unk2;
			uint32 m_Unk3;
			uint32 m_Unk4;
			uint32 m_Unk5;
			uint32 m_Unk6;
			uint32 m_Unk7;
			uint32 m_Format; // value between the bits 0 and 6
			uint32 m_Unk9;
			uint32 m_Unk10;
			uint32 m_MipLevelMax;
			ptr32<void> m_MipMapOffset; // GPU offset
		};


		// VTable: 640A5E00 | 4C337300
		struct grcTexture
		{
			uint32 m_VTable;
			uint32 m_Unk0;
			uint32 m_Unk1; // eg: 1
			uint32 m_Unk2;
			uint32 m_Unk3;
			uint32 m_TextureSize; // or 2x uint16
			ptr32<char> m_pTexName; // eg:  blabla.dds
			ptr32<grcTextureFormat> m_pTexInfo;
			uint16 m_WidthCAND0;
			uint16 m_HeightCAND0;
			uint16 m_Unk5;
			uint16 m_Unk6;
			// UsageTex m_UsageTex;

			float m_Unk7; // 1.0
			float m_Unk8; // 1.0
			float m_Unk9; // 1.0

			uint32 m_Unk10;
			uint32 m_Unk11;
			uint32 m_Unk12;
		};

		// VTable: ACDE5900 | 0C486D00
		struct grcTextureDictionnary
		{
			uint32 m_VTable;
			ptr32<uint32> m_Unk0;
			uint16 m_Unk1;
			uint16 m_Unk2;
			uint32 m_Unk3; // eg: 1

			// hashmap struct below
			pgCollect<uint32> m_TexturesHashsList; // collection of hash textures
			pgCollect<ptr32<grcTexture>> m_TexturesList;
		};

	#pragma pack(pop)




		static const char* GetFormatStr(uint32 formatId)
		{
			if (formatId >= ARRAYSIZE(TEX_FORMAT_STR))
				return "UNKNOWN";
			else
				return TEX_FORMAT_STR[formatId];
		}


		static DWORD _andnot(DWORD a1, DWORD a2) {
			return a1 & (~a2);
		}

		static DWORD RdrX360TexData(int x, int y, int w, DWORD texelPitch)
		{
			DWORD alignedWidth = _andnot((w + 31), 31);
			DWORD logBpp = (texelPitch >> 2) + ((texelPitch >> 1) >> (texelPitch >> 2));
			DWORD Macro = ((x >> 5) + (y >> 5) * (alignedWidth >> 5)) << (logBpp + 7);
			DWORD Micro = (((x & 7) + ((y & 6) << 2)) << logBpp);
			DWORD Offset = Macro + ((_andnot(Micro, 15)) << 1) + (Micro & 15) + ((y & 8) << (3 + logBpp)) + ((y & 1) << 4);
			return (((_andnot(Offset, 511) << 3)) + ((Offset & 448) << 2) + (Offset & 63) + ((y & 16) << 7) + (((((y & 8) >> 2) + (x >> 3)) & 3) << 6)) >> logBpp;
		}


		// bypass the "memory:$blabla:"
		static const char* JustGetTexName(const char* texname)
		{
			char* first = (char*)strstr(texname, ":");
			if (first)
			{
				first++;
				first = (char*)strstr(first, ":");
				if (first)
				{
					first++;
					return first;
				}
			}
			
			return texname;
		}


		struct TexDef {
			std::string m_Tex;
			uint32 m_TexHash;
		};

		static SkyVector<TexDef> EnumTexHashsOfTXD(char* sys, int sysSize, char* gfx, int gfxSize)
		{
			SkyVector<TexDef> toret;
			uint32 magic = *(uint32*)sys;

			// SIMPLE TEX
			if (magic == '\xB4\xF7\x5D\x00' ||
				magic == '\x74\xF6\x5D\x00' ||
				magic == '\x74\xF9\x5D\x00' ||
				magic == '\x64\x0A\x5E\x00')
			{
				rage::grcTexture* iTex = (rage::grcTexture*)sys;
				toret.push_back({ iTex->m_pTexName.Get(sys, gfx), 0 });
			}

			// DICTIONNARY
			else
			{
				grcTextureDictionnary* texDict = (grcTextureDictionnary*)sys;
				for (uint16 i = 0; i < texDict->m_TexturesList.GetCount(); i++)
				{
					rage::grcTexture* iTex = texDict->m_TexturesList.GetAt(i, sys, gfx)->Get(sys, gfx);
					uint32 iHash = InvEnd(*texDict->m_TexturesHashsList.GetAt(i, sys, gfx));
					toret.push_back({ iTex->m_pTexName.Get(sys, gfx), iHash });
				}
			}

			return toret;
		}



		static bool TranslateTexture(Texture& out, rage::grcTexture* tex, uint32 hash, char* sys, int sysSize, char* gfx, int gfxSize)
		{
			rage::grcTexture* iTex = tex;
			rage::grcTextureFormat* pTexFormat = iTex->m_pTexInfo.Get(sys, gfx);

			out.m_Texname = std::string(JustGetTexName(iTex->m_pTexName.Get(sys, gfx)));

			// printf("\n\n\n\n\n%s\n\nTexBase:\n\n", out.m_Texname.c_str());
			// printHex(*iTex, 1);
			// printf("\nTexFormat:\n\n");
			// printHex(*pTexFormat, 1);
			// 
			// 
			// 
			// if (strstr(out.m_Texname.c_str(), "_n.dds"))
			// 	Sleep(1);
			// 
			// if (strstr(out.m_Texname.c_str(), "seaofcoronando_s07_flow_dd.dds"))
			// 	Sleep(1);

			uint32 iMipMapOff = 0;
			uint32 iFormat = 0;
			uint32 iMipLevelsMax = 0;

			uint32 iTexSize = InvEnd(iTex->m_TextureSize);
			uint16 iWidth = InvEnd(iTex->m_WidthCAND0);
			uint16 iHeight = InvEnd(iTex->m_HeightCAND0);
			if (pTexFormat)
			{
				iFormat = InvEnd(pTexFormat->m_Format);
				iFormat <<= 26;
				iFormat >>= 26;

				// 00000000000000000000000001010100
				uint32 val0 = (pTexFormat->m_MipLevelMax & 0xC0000000) >> 6;
				uint32 val1 = (pTexFormat->m_MipLevelMax & 0x00030000) << 10;
				iMipLevelsMax = InvEnd(val0 | val1) + 1;

				iMipMapOff = InvEnd(pTexFormat->m_MipMapOffset.m_Offset);
			}

			uint32 texPtr = InvEnd(pTexFormat->m_Format & 0xFFFFFF00);
			texPtr >>= 7;
			texPtr <<= 7;

			PixelFormat pxFormat = ePX_DXT1;
			uint16 endian = 0;
			uint16 TSize = 0;
			uint16 width = iWidth;
			uint16 height = iHeight;
			bool swizzled = false;
			uint32 computedTexSize = 0;
			if (iFormat == eGPUTEXTUREFORMAT_DXT1 ||
				iFormat == eGPUTEXTUREFORMAT_DXT2_3 ||
				iFormat == eGPUTEXTUREFORMAT_DXT4_5 ||
				iFormat == eGPUTEXTUREFORMAT_DXT5A ||
				iFormat == eGPUTEXTUREFORMAT_8 ||
				iFormat == eGPUTEXTUREFORMAT_8_8_8_8 ||
				iFormat == eGPUTEXTUREFORMAT_DXN)
			{
				if (width % 128 != 0) width += 128 - (width % 128);
				if (height % 128 != 0) height += 128 - (height % 128);

				swizzled = true;


				if (iFormat == eGPUTEXTUREFORMAT_DXT1)
				{
					computedTexSize = (width * height) / 2;
					width = iWidth / 4;
					height = iHeight / 4;
					TSize = 8;
					endian = 1;
					pxFormat = ePX_DXT1;
				}
				else if (iFormat == eGPUTEXTUREFORMAT_DXT2_3)
				{
					computedTexSize = width * height;
					width = iWidth / 4;
					height = iHeight / 4;
					TSize = 16;
					endian = 1;
					pxFormat = ePX_DXT3;
				}
				else if (iFormat == eGPUTEXTUREFORMAT_DXT4_5)
				{
					computedTexSize = width * height;
					width = iWidth / 4;
					height = iHeight / 4;
					TSize = 16;
					endian = 1;
					pxFormat = ePX_DXT5;
				}
				else if (iFormat == eGPUTEXTUREFORMAT_DXT5A)
				{
					computedTexSize = width * height;
					width = iWidth / 4;
					height = iHeight / 4;
					TSize = 8;
					endian = 1;
					pxFormat = ePX_DXT5;
				}
				else if (iFormat == eGPUTEXTUREFORMAT_8)
				{
					computedTexSize = width * height;
					width = iWidth;
					height = iHeight;
					TSize = 1;
					endian = 1;
					pxFormat = ePX_L8;
				}
				else if (iFormat == eGPUTEXTUREFORMAT_8_8_8_8)
				{
					computedTexSize = width * height * 4;
					width = iWidth;
					height = iHeight;
					TSize = 4;
					endian = 2;
					pxFormat = ePX_A8R8G8B8;
				}
				else if (iFormat == eGPUTEXTUREFORMAT_DXN)
				{
					computedTexSize = width * height * 4;
					width = iWidth;
					height = iHeight;
					TSize = 4;
					endian = 2;
				}
			}


			iTexSize = computedTexSize;
			out.m_Stride = iTexSize / iHeight; // test iHeight if dont work !



	#ifdef VERBOSE_TEX
			printf("\t%ux%u  size_cand: %u   mips: %u   format: %u   GPU_tex_ptr: ", iWidth, iHeight, iTexSize, iMipLevelsMax, iFormat);
			printHex(InvEnd(texPtr), 0);

			printf("   GPU_mip_ptr: ");
			printHex(InvEnd(iMipMapOff), 0);

			printf("   %s\n", iTex->m_pTexName.Get(sys));
	#endif

			// continue;
			// printf("   gpu_ptr_cand1: ");
			// printHex(ptrCand0, 1);


			// printf("\t[%ux%u  size: %u  mip levels: %u  format: %u (%s)] ",
			// 	iWidth, iHeight, iTexSize, iMipLevelsMax, iFormat, GetFormatStr(iFormat));
			// printf("0x");
			// printHex(iHash, 0);
			// printf(" : %s\n", iTex->m_pTexName.Get(sys));


			// TXMap texMap;
			// texMap.m_Hash = iHash;

			//// Texture newTex = Texture(
			//// 	std::string(iTex->m_pTexName.Get(sys)),
			//// 	hash,
			//// 	new char[iTexSize], // std::shared_ptr<char[]>(new char[iTexSize]),
			//// 	iTexSize,
			//// 	(TEX_FORMATS_ENUM)iFormat,
			//// 	pxFormat,
			//// 	iWidth,
			//// 	iHeight,
			//// 	iMipLevelsMax
			//// );

			out.m_Hash = hash;
			out.m_Buffer = SkySharedPtr<char[]>(new char[iTexSize]);
			out.m_BufferSize = iTexSize;
			out.m_Format = (TEX_FORMATS_ENUM)iFormat;
			out.m_PxFormat = pxFormat;
			out.m_Width = iWidth;
			out.m_Height = iHeight;
			out.m_Mips = iMipLevelsMax;
			// out.m_Usage = InvEnd(iTex->m_UsageTex);


			int toCpySize = iTexSize;
			// int overflowProb = (int)(texPtr + toCpySize);
			// if (overflowProb > (int)gfxSize)
			// 	toCpySize = gfxSize - texPtr;
			char* pOutTex = out.m_Buffer.get();

			if (swizzled)
			{
				char* pTex = (char*)(gfx + texPtr);
				char* pOut = pOutTex;

				for (uint16 x = 0; x < width; x++) {
					for (uint16 y = 0; y < height; y++) {
						int iOff = RdrX360TexData(x, y, width, TSize);
						memcpy(pOut + ((x + y * width) * TSize), pTex + (iOff * TSize), TSize);
					}
				}
			}
			else
				memcpy((char*)out.m_Buffer.get(), gfx + texPtr, toCpySize);


			// GPUENDIAN_8IN16
			if (endian == 1)
			{
				uint16* texUint16 = (uint16*)pOutTex;
				for (uint32 i = 0; i < computedTexSize / sizeof(uint16); i++) {
					texUint16[i] = InvEnd(texUint16[i]);
				}
			}

			// GPUENDIAN_8IN32
			else if (endian == 2)
			{
				uint32* texUint32 = (uint32*)pOutTex;
				for (uint32 i = 0; i < computedTexSize / sizeof(uint32); i++) {
					texUint32[i] = InvEnd(texUint32[i]);
				}
			}

			return true;
		}


		static TXD ReadTextureDict(void* pTexDict, char* sys, int sysSize, char* gfx, int gfxSize)
		{
			TXD txd;
			grcTextureDictionnary* texDict = (grcTextureDictionnary*)pTexDict;
			for (uint16 i = 0; i < texDict->m_TexturesList.GetCount(); i++)
			{
				rage::grcTexture* iTex = texDict->m_TexturesList.GetAt(i, sys, gfx)->Get(sys, gfx);
				uint32 iHash = InvEnd(*texDict->m_TexturesHashsList.GetAt(i, sys, gfx));

				Texture newTex = Texture();
				if (TranslateTexture(newTex, iTex, iHash, sys, sysSize, gfx, gfxSize))
				{
					if (newTex.m_Buffer)
						txd.m_Texs.push_back(std::move(newTex));
				}
			}
			return txd;
		}



		static TXD LoadTextureDictionnary(const char* txdName, char* sys, int sysSize, char* gfx, int gfxSize)
		{
			TXD txd;
			uint32 magic = *(uint32*)sys;

			// SIMPLE TEX
			if (magic == '\xB4\xF7\x5D\x00' ||
				magic == '\x74\xF6\x5D\x00' ||
				magic == '\x74\xF9\x5D\x00' ||
				magic == '\x64\x0A\x5E\x00')
			{
				rage::grcTexture* iTex = (rage::grcTexture*)sys;

				Texture newTex = Texture();
				if (TranslateTexture(newTex, iTex, 0, sys, sysSize, gfx, gfxSize))
				{
					if (newTex.m_Buffer)
					{
						// printf("%s\n", newTex.m_Texname.c_str());
						txd.m_Texs.push_back(std::move(newTex));
					}
				}
				// printf("\n\n");
			}

			// DICTIONNARY
			else
			{
				grcTextureDictionnary* texDict = (grcTextureDictionnary*)sys;
				// printf("%s\n", txdName);
				for (uint16 i = 0; i < texDict->m_TexturesList.GetCount(); i++)
				{
					rage::grcTexture* iTex = texDict->m_TexturesList.GetAt(i, sys, gfx)->Get(sys, gfx);
					uint32 iHash = InvEnd(*texDict->m_TexturesHashsList.GetAt(i, sys, gfx));

					Texture newTex = Texture();
					if (TranslateTexture(newTex, iTex, iHash, sys, sysSize, gfx, gfxSize))
					{
						if (newTex.m_Buffer)
							txd.m_Texs.push_back(std::move(newTex));
					}
				}
				// printf("\n\n");
			}

			return txd;
		}
	}
}