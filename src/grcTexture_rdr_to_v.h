#pragma once
#include "grcTexture_rdr.h"
#include "grcTexture.h"

#include "IOFile.h"
#include "CThreadShared.h"

#include "DDSTextureLoader/DDSTextureLoader11.h"
#include "DirectXTex/DirectXTex.h"
#include "wincodec.h"
#include "nvtt/nvtt.h"

#include "SkyCommons/SkyLogs.h"


static bool WriteDDSInBuffer(const rdr::rage::Texture& in, char* buffer, int bufferSize)
{
	if (in.m_BufferSize + sizeof(DDSHead) > bufferSize)
		return false;

	auto form = in.m_Format;
	auto pxForm = in.m_PxFormat;
	bool hasMips = in.m_Mips > 1;

	DDSHead* pDDS = (DDSHead*)buffer;
	pDDS->m_magic = '\x44\x44\x53\x20';
	pDDS->m_DDS.size = sizeof(DDS_HEADER);
	pDDS->m_DDS.width = in.m_Width;
	pDDS->m_DDS.height = in.m_Height;
	pDDS->m_DDS.mipMapCount = in.m_Mips;
	pDDS->m_DDS.depth = 1;
	pDDS->m_DDS.flags =
		DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT |
		// (hasMips ? DDS_HEADER_FLAGS_MIPMAP : 0) |
		DDSD_LINEARSIZE;// ((pxForm == ePX_A8R8G8B8 || pxForm == ePX_L8) ? DDSD_LINEARSIZE : 0);

	// DDSD_LINEARSIZE | DDSD_PIXELFORMAT | DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT

	// DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_PITCH | (i.m_Tex.m_Mips ? DDS_HEADER_FLAGS_MIPMAP : 0);
	if (pxForm == ePX_DXT1 ||
		pxForm == ePX_DXT3 ||
		pxForm == ePX_DXT5)
	{
		pDDS->m_DDS.pitchOrLinearSize = 0;
	}
	else if (pxForm == ePX_A8R8G8B8)
		pDDS->m_DDS.pitchOrLinearSize = in.m_Width * in.m_Height * 4;
	else if (pxForm == ePX_L8)
		pDDS->m_DDS.pitchOrLinearSize = in.m_Width * in.m_Height * 1;



	auto& pxFormat = pDDS->m_DDS.ddspf;
	pxFormat.size = sizeof(DDS_PIXELFORMAT);
	pxFormat.flags = // DDPF_ALPHAPIXELS |
		((pxForm != ePX_A8R8G8B8 && pxForm != ePX_L8) ? DDPF_FOURCC : 0) |
		(pxForm == ePX_A8R8G8B8 ? DDPF_RGB : 0);


	if (pxForm == PixelFormat::ePX_DXT1)
		pxFormat.fourCC = InvEnd('DXT1');

	else if (pxForm == PixelFormat::ePX_DXT3)
		pxFormat.fourCC = InvEnd('DXT3');

	else if (pxForm == PixelFormat::ePX_DXT5)
		pxFormat.fourCC = InvEnd('DXT5');


	if (pxForm == PixelFormat::ePX_A8R8G8B8)
	{
		pxFormat.RGBBitCount = 32;
		pxFormat.RBitMask = 0x00FF0000;
		pxFormat.GBitMask = 0x0000FF00;
		pxFormat.BBitMask = 0x000000FF;
		pxFormat.ABitMask = 0x00FF0000;
	}
	else if (pxForm == PixelFormat::ePX_L8)
		pxFormat.RGBBitCount = 8;

	pDDS->m_DDS.caps =
		DDSCAPS_TEXTURE | 0;
	// ((hasMips || (form > 0 && form < 3)) ? DDSCAPS_COMPLEX : 0) |
	// (hasMips ? DDSCAPS_MIPMAP : 0);

	if (form == TEX_FORMATS_ENUM::eGPUTEXTUREFORMAT_1_5_5_5)
		pDDS->m_DDS.caps2 = DDSCAPS2_VOLUME;

	memcpy(buffer + sizeof(DDSHead), in.m_Buffer.get(), in.m_BufferSize);

	return true;
}



static ID3D11Device* g_Device = 0;
static ID3D11DeviceContext* g_DeviceCtx = 0;
static IWICImagingFactory* g_WicFactory = 0;

using namespace v::rage;

namespace rage
{
	static CMTStore<SkyString> g_TmpFiles;

	static void ThreadTransitFileGC()
	{
		for (;;)
		{
			for (const auto& i : g_TmpFiles)
				DeleteFileA(i.c_str());

			Sleep(100);
		}
	}


	struct TexOuputInterface : public nvtt::OutputHandler
	{
		uint32 m_Pos = 0;

		virtual ~TexOuputInterface() {}
		virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) override { }

		virtual bool writeData(const void* data, int size) override
		{
			memcpy(g_LocalThreadBuffer + m_Pos, data, size);
			m_Pos += size;
			return true;
		}

		virtual void endImage() override { }
	};

	struct TexErrorHandler : public nvtt::ErrorHandler
	{
		virtual ~TexErrorHandler() {}
		virtual void error(nvtt::Error e) override {}
	};



	static struct NVTTContext
	{
		nvtt::Context* m_Cxt = 0;

		void Init() {
			if (!m_Cxt)
				m_Cxt = new nvtt::Context(false);
		}
		nvtt::Context& GetContext() const { return *m_Cxt; };

	} thread_local g_NvttCtx;



	// TinyAllocator est scoped dans le bloc de déclaration
	static bool ConvertTextureRdrToV(
		TinyAllocator& alloc,
		const rdr::rage::Texture& in,
		v::rage::grcTexture* out)
	{
		out->m_Width = in.m_Width;
		out->m_Height = in.m_Height;
		out->m_Depth = 1;


		char* realTexName = (char*)in.m_Texname.c_str();
		if (strstr(realTexName, "memory:") == realTexName) {
			for (int matchesCount = 0; realTexName != 0x00; realTexName++) {
				if (*realTexName == ':') {
					if (matchesCount == 1) {
						realTexName++;
						break;
					}
					matchesCount++;
				}
			}
		}

		int nameLength = strlen(realTexName);
		if (nameLength)
		{
			char* outBuf = (char*)alloc.Alloc(nameLength + 1);
			strcpy(outBuf, realTexName);
			out->m_pName = outBuf;
		}


		nvtt::Format NvttFormat = nvtt::Format::Format_DXT5;

		if (in.m_PxFormat == PixelFormat::ePX_DXT1) {
			out->m_Format = InvEnd('DXT1');
			NvttFormat = nvtt::Format::Format_DXT1;
		}
		else if (in.m_PxFormat == PixelFormat::ePX_DXT3) {
			out->m_Format = InvEnd('DXT3');
			NvttFormat = nvtt::Format::Format_DXT3;
		}
		else if (in.m_PxFormat == PixelFormat::ePX_DXT5) {
			out->m_Format = InvEnd('DXT5');
			NvttFormat = nvtt::Format::Format_DXT5;
		}


		// out->SetUsage(texUsage);



		// ici on peux traiter la texture (upscale processing par IA, generation de mipmaps etc...)
		{
			out->m_Stride = in.m_Stride;
			out->m_Levels = 1; // 1 niveau de mipmap pour test

			void* textureBuffer = 0;
			
			bool isBumpSampler = strstr(realTexName, "_n.dds") || strstr(realTexName, "fbtile_");
			bool needGenerateMips = (in.m_Width >= 128 && in.m_Height >= 128);
			
			int mipmapsCount = (in.m_Width / 128) + 1;
			if (!needGenerateMips)
			{
				mipmapsCount = 1;
			}

			// GENERER LES MIPMAPS DE LA TEXTURE
			if (true && (needGenerateMips || isBumpSampler))
			{
				char* finalTexPtr = 0;
				uint32 finalTexSize = 0;

				char cacheTexPath[_MAX_PATH];
				sprintf_s(cacheTexPath, "C:\\RDR1_to_V\\TMP\\cache\\%ux%u__%s",
					in.m_Width, in.m_Height, realTexName);


				// verifie si la texture n'a pas déjà été traité dans le cache
				SkySharedPtr<char[]> listBuff;
				uint32 fileSize = 0;
				if (LoadFile(cacheTexPath, fileSize, listBuff))
				{
					finalTexPtr = listBuff.get();
					finalTexSize = fileSize;
				}

				else
				{
					/*
						Traite la texture dans NVTT
						- Ajoute des niveaux mips
						- Effectue la transformation des normals Dec3N big to little endian
						- autres opérations d'ajustement colorimétrique
					*/
					WriteDDSInBuffer(in, (char*)g_LocalThreadBuffer, LOCAL_THREAD_BUFFER_SIZE);

					char pathTmpDds[256];
					sprintf_s(pathTmpDds, "C:\\RDR1_to_V\\TMP\\%u.dds", GetCurrentThreadId());

					HANDLE tmpDDS = CreateFileA(pathTmpDds, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, 0);
					WriteFileS(tmpDDS, g_LocalThreadBuffer, in.m_BufferSize + sizeof(DDSHead));
					SetEndOfFile(tmpDDS);
					CloseHandle(tmpDDS);



					bool isCM1 = strstr(realTexName, "cm1_") == realTexName;
					bool isCM2 = strstr(realTexName, "cm2_") == realTexName;

					{
						TexOuputInterface outTextureMgr = {};

						nvtt::Surface texIn;
						texIn.load(pathTmpDds);

						nvtt::CompressionOptions compressionOptions;
						compressionOptions.setFormat(NvttFormat);

						nvtt::OutputOptions outputOptions;
						outputOptions.setOutputHandler(&outTextureMgr);

						g_NvttCtx.Init();
						nvtt::Context& cxt = g_NvttCtx.GetContext();

						if (false && (isCM1 || isCM2 || isBumpSampler))
							mipmapsCount = 0;



						if (!cxt.outputHeader(texIn, mipmapsCount, compressionOptions, outputOptions))
							SkyPrint->print("Echec de l'écriture de l'en-tete DDS\n");



						// IS A TerrainBlendMap TEXTURE
						if (isCM1)
						{
							bool hasAlpha = texIn.alphaMode() == nvtt::AlphaMode_Transparency;
							// texIn.setAlphaMode(nvtt::AlphaMode_Transparency);

							float* pChannels[5];
							pChannels[0] = texIn.channel(0);
							pChannels[1] = texIn.channel(1);
							pChannels[2] = texIn.channel(2);
							pChannels[3] = texIn.channel(3);

							uint32 width = texIn.width();
							uint32 height = texIn.height();

							for (uint32 x = 0; x < width; x++)
							{
								for (uint32 y = 0; y < height; y++)
								{
									// Vector4c pixel = { *pChannels[0], *pChannels[1], *pChannels[2], *pChannels[3] };
									
									*pChannels[3] = 1.f;

									pChannels[0]++;
									pChannels[1]++;
									pChannels[2]++;
									pChannels[3]++;
								}
							}

							texIn.flipX();
						}

						if (true && isBumpSampler)
						{
							// ...

							/*
							* REARRANGE LES COMPOSANTES DES PIXELS
							*
									pxBuff[0] = 0xFF;
									pxBuff[1] = a;//b;
									pxBuff[2] = g;
									pxBuff[3] = 0xFF;
							*/

							float* pChannels[5];
							pChannels[0] = texIn.channel(0);
							pChannels[1] = texIn.channel(1);
							pChannels[2] = texIn.channel(2);
							pChannels[3] = texIn.channel(3);

							uint32 width = texIn.width();
							uint32 height = texIn.height();

							if (true && pChannels[3])
							{
								for (uint32 x = 0; x < width; x++)
								{
									for (uint32 y = 0; y < height; y++)
									{
										Vector4c pixel = { *(pChannels[0]), *(pChannels[1]), *(pChannels[2]), *(pChannels[3]) };


										*(pChannels[0]) = pixel.w;
										*(pChannels[1]) = pixel.y;
										*(pChannels[2]) = 1.f;
										*(pChannels[3]) = 1.f;


										pChannels[0]++;
										pChannels[1]++;
										pChannels[2]++;
										pChannels[3]++;
									}
								}
							}
						}


						// texIn.flipX();
						// texIn.flipY();

						for (int mip = 0; mip < mipmapsCount; mip++)
						{
							if (!cxt.compress(texIn, 0, mip, compressionOptions, outputOptions))
								SkyPrint->print("Echec de la compression et de l'écriture du niveau courant\n");

							if (mipmapsCount > 0)
							{
								texIn.toLinearFromSrgb();
								texIn.premultiplyAlpha();

								texIn.buildNextMipmap(nvtt::MipmapFilter_Box);

								texIn.demultiplyAlpha();
								texIn.toSrgb();
							}
						}

						finalTexPtr = g_LocalThreadBuffer;
						finalTexSize = outTextureMgr.m_Pos;

						HANDLE cacheDDS = CreateFileA(cacheTexPath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
						WriteFileS(cacheDDS, finalTexPtr, finalTexSize);
						SetEndOfFile(cacheDDS);
						CloseHandle(cacheDDS);
					}

					// tmpDDS = CreateFileA(pathTmpDds, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, 0);
					// WriteFileS(tmpDDS, g_LocalThreadBuffer, outTextureMgr.m_Pos);
					// SetEndOfFile(tmpDDS);
					// CloseHandle(tmpDDS);
				}


				DDSHead* pDDS = (DDSHead*)finalTexPtr;
				out->m_Levels = pDDS->m_DDS.mipMapCount;
				out->m_Stride = pDDS->m_DDS.pitchOrLinearSize / pDDS->m_DDS.height;

				uint32 ddsHeadSize = *(uint32*)(finalTexPtr + 4) + 4;
				textureBuffer = alloc.Alloc(finalTexSize - ddsHeadSize);
				// if (textureBuffer == 0)
				// 	Sleep(10000);

				memcpy((char*)(textureBuffer), finalTexPtr + ddsHeadSize, finalTexSize - ddsHeadSize);


				out->m_Format = *(uint32*)(finalTexPtr + 0x54);
			}


			// AUCUN MIPMAP A GENERER
			else
			{
				textureBuffer = alloc.Alloc(in.m_BufferSize);
				memcpy((char*)(textureBuffer), in.m_Buffer.get(), in.m_BufferSize);
			}
			

			out->m_pData = textureBuffer;
		}

		/*
		g_load_address = (fi_handle)writeBuff;
		FIBITMAP* texBmp = FreeImage_LoadFromHandle(FIF_DDS, &io, g_load_address);

		g_load_address = (fi_handle)writeBuff;
		FreeImage_SaveToHandle(FREE_IMAGE_FORMAT::FIF_BMP, texBmp, &io, g_load_address);
		FreeImage_Unload(texBmp);


		// Normal Pixel GTA V:
		// 0: généralement 0xFF
		// 1: valeur int8 qui varie généralement entre 105 et 127
		// 2: la meme que 2
		// 3: généralement 0xFF
		for (int iW = 0; iW < r.m_Width * r.m_Height * 4; iW += 4)
		{
			int8* pxBuff = (int8*)(writeBuff + iW + 0x36);

			int8 r = pxBuff[0];
			int8 g = pxBuff[1];
			int8 b = pxBuff[2];
			int8 a = pxBuff[3];

			pxBuff[0] = 0xFF;
			pxBuff[1] = a;//b;
			pxBuff[2] = g;
			pxBuff[3] = 0xFF;
		}

		g_load_address = (fi_handle)writeBuff;
		FIBITMAP* texNewBMP = FreeImage_LoadFromHandle(FIF_BMP, &io, g_load_address, );

		if (FreeImage_SaveU(FIF_DDS, texNewBMP, outFile.c_str()))
		{

		}

		FreeImage_Unload(texNewBMP);
		*/

		return true;
	}
}
