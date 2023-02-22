#pragma once
#include "rmcDrawable_rdr.h"
#include "grcTexture_rdr.h"
#include "grcTexture_rdr_to_v.h"
#include "grcTexture.h"
#include "gtaDrawable_v.h"
#include "DefaultTex.h"
#include "CTextureLinker.h"
#include "CThreadShared.h"
// #include "CTexMatcher.h"
#include "SkyCommons/SkyLogs.h"
#include <algorithm>
#include <cctype>

using namespace ::rage;
using namespace rage;

static v::rage::grcTexture* g_DefaultTexture = 0;
static v::rage::grcTexture* g_NullBlendMap = 0;

const uint8 AlphaTexBlob[] = {
		0x00, 0x00, 0x00, 0x00,
		0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
		0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
		0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
		0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
		0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
		0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
		0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
		0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA,
};

static void InitDefaultTex()
{
	if (!g_DefaultTexture)
	{
		g_DefaultTexture = new v::rage::grcTexture();

		g_DefaultTexture->m_Format = InvEnd('DXT1');
		g_DefaultTexture->m_Width = 64;
		g_DefaultTexture->m_Height = 64;
		g_DefaultTexture->m_Depth = 1;
		g_DefaultTexture->m_Levels = 5;
		g_DefaultTexture->m_pName = "damier_texture";
		g_DefaultTexture->SetUsage(TexUsage::eDEFAULT);

		g_DefaultTexture->m_Stride = 32;
		g_DefaultTexture->m_pData = (void*)g_DamierTexData;
	}

	if (!g_NullBlendMap)
	{
		g_NullBlendMap = new v::rage::grcTexture();

		g_NullBlendMap->m_Format = InvEnd('DXT1');
		g_NullBlendMap->m_Width = 16;
		g_NullBlendMap->m_Height = 16;
		g_NullBlendMap->m_Depth = 1;
		g_NullBlendMap->m_Levels = 1;
		g_NullBlendMap->m_pName = "full_alpha";
		g_NullBlendMap->SetUsage(TexUsage::eDEFAULT);
		g_NullBlendMap->m_Stride = 8;
		g_NullBlendMap->m_pData = (void*)AlphaTexBlob;
	}
}


class CVertexAccess
{
public:
	uint8* m_pData;
	uint32 m_Stride;

public:
	CVertexAccess(uint8* data, uint32 stride) : m_pData(data), m_Stride(stride) {}

	Vector3c GetPos(uint32 i)
	{
		Vector3c* pPos = (Vector3c*)(m_pData + (i * m_Stride));
		return Vector3c(pPos->x, pPos->y, pPos->z);
	}
};

// Retourne l'index de la texture dans le dictionnaire
static int FindTextureIndexByName(rdr::rage::TXD& texs, const SkyString& texname)
{
	for (auto i = 0; i < texs.m_Texs.size(); i++) {
		if (texs.m_Texs[i].m_Texname == texname)
			return i;
	}
	return -1;
}


static Vector3c UnpackDec3N(uint32 N)
{
	int64 nSafe = N;
	float x = float(((nSafe >> uint32_t(20)) & 0x3FF) - 512) / 511.0f;
	float y = float(((nSafe >> uint32_t(10)) & 0x3FF) - 512) / 511.0f;
	float z = float(((nSafe >> uint32_t(0)) & 0x3FF) - 512) / 511.0f;
	// float x = (float)(N >> 22) / (float)((1 << 10) - 1);
	// float y = ((float)((N >> 12) & ((1 << 10) - 1)) / (float)((1 << 10) - 1));
	// float z = ((float)((N >> 2) & ((1 << 10) - 1)) / (float)((1 << 10) - 1));

	return { z, y, x };
}

static v::rage::grcTexture* EmbedTexture(v::rage::grcTexture* pTex, SkyVector<std::tuple<CTexMatcher, v::rage::grcTexture*, uint32>>& TexToPushMap)
{
	bool isPresent = false;
	for (const auto& i : TexToPushMap) {
		if (std::get<0>(i).m_Texname == pTex->m_pName) {
			isPresent = true;
			break;
		}
	}

	if (!isPresent) {
		CTexMatcher defaultMatcher(pTex->m_pName, JoaatStrict(pTex->m_pName));
		TexToPushMap.push_back(std::make_tuple(defaultMatcher, pTex, defaultMatcher.m_TexHash));
	}

	return pTex;
}


static v::rage::grcTexture* GetAndEmbedTexture(
	TinyAllocator& alloc,
	SkyString texname,
	SkyMap<int, v::rage::grcTexture*>& TexToEmbedMap,
	SkyVector<std::tuple<CTexMatcher, v::rage::grcTexture*, uint32>>& TexToPushMap,
	rdr::rage::TXD& texs,
	v::rage::TexUsage texUsage,
	DetailLevel levelDetail,
	bool embed = true)
{
	v::rage::grcTexture* texBase = 0;

	if (texname != "null")
	{
		// RECHERCHE DANS LES TEXTURES EMBEDD
		int texId = FindTextureIndexByName(texs, texname);
		if (texId != -1)
		{
			auto texMatch = TexToEmbedMap.find(texId);
			if (texMatch != TexToEmbedMap.end())
				texBase = texMatch->second;
			else
			{
				// préalloue la texture, elle sera rempli à la fin de la construction du drawable
				v::rage::grcTexture* iEmbedTex = alloc.Alloc<v::rage::grcTexture>();
				texBase = iEmbedTex;

				if (embed)
					TexToEmbedMap[texId] = iEmbedTex;
			}

			goto RETURN_END;
		}

		else
		{
			const char* fullTexName = texname.c_str();
			int r = strlen(fullTexName) - 1;
			for (; r >= 0; r--) {
				if (fullTexName[r] == '.')
					break;
			}
			char cleanTexName[256];
			strncpy_s(cleanTexName, texname.c_str(), r);
			uint32 texHash = JoaatStrict(cleanTexName);


			// RECHERCHE DANS LES TEXTURE DEJA LOAD (textures de dictionnaires)
			CTexMatcher texMatcher(fullTexName, texHash);
			v::rage::grcTexture* pTexMatch = 0;

			for (const auto& ie : TexToPushMap) {
				if (std::get<0>(ie) == texMatcher) {
					pTexMatch = std::get<1>(ie);
					break;
				}
			}

			if (pTexMatch)
			{
				texBase = pTexMatch;
				goto RETURN_END;
			}


			// RECHERCHE DANS LES DICTIONNAIRES DE TEXTURES ET CHARGE LA TEXTURE
			else
			{
				uint16 widthOut = 0;
				uint16 heightOut = 0;

				auto pDict = atSingleton<CTextureLinker>::Get()->FindTextureByMatcher(texMatcher, levelDetail, &widthOut, &heightOut);
				if (pDict)
				{
					// if (strstr(texMatcher.m_Texname.c_str(), "die_01_dst_x.dds"))
					// 	Sleep(1);

					auto pTXD = pDict->GetOrOpenTexDict();
					if (pTXD)
					{
						Texture texGets;
						pTXD->GetTextureByMatcher(texGets, texMatcher, widthOut, heightOut);

						if (texGets.m_Width == 0 && texGets.m_Height == 0)
						{
							texGets.m_Width = widthOut;
							texGets.m_Height = heightOut;
							texGets.m_Texname = texMatcher.m_Texname;
						}

						{
							v::rage::grcTexture* iNewTex = alloc.Alloc<v::rage::grcTexture>();
							if (ConvertTextureRdrToV(alloc, texGets, iNewTex))
							{
								iNewTex->m_UsageData = v::rage::TexUsage::eDEFAULT;

								if (embed)
									TexToPushMap.push_back(std::make_tuple(texMatcher, iNewTex, texGets.m_Hash));

								texBase = iNewTex;
								goto RETURN_END;
							}
						}
					}


					SkyPrint->printColor(10, "texture '%s' is in '%s' texture dictionnary\n", fullTexName, pDict->m_Path.c_str());
				}
				else
					SkyPrint->printColor(12, "texture '%s' not resolved\n", fullTexName);
			}
		}
	}


RETURN_END:

	if (texBase)
		texBase->SetUsage(texUsage);

	if (!texBase)
	{
		texBase = g_DefaultTexture;

		bool isDefaultPresent = false;
		for (const auto& i : TexToPushMap) {
			if (std::get<0>(i).m_Texname == g_DefaultTexture->m_pName) {
				isDefaultPresent = true;
				break;
			}
		}

		if (!isDefaultPresent)
		{
			CTexMatcher defaultMatcher(g_DefaultTexture->m_pName, JoaatStrict(g_DefaultTexture->m_pName));
			TexToPushMap.push_back(std::make_tuple(defaultMatcher, g_DefaultTexture, defaultMatcher.m_TexHash));
		}
	}


	return texBase;
}




// Utilisé dans l'algorithme de placement de végétation
struct TerrainVertex {
	Vector3c Position; // position absolu du vertex
	Vector3c Normal;
};
struct TerrainGeometry
{
	SkyString TexturesName[4];
	TerrainVertex* Vertices = 0;
	uint32 VerticesCount = 0;
	Vector2c SurfaceSize = { 0.f, 0.f };

	TerrainGeometry::~TerrainGeometry() {
		if (Vertices)
			delete[] Vertices;
	}
};
struct TerrainInfos
{
	SkyVector<TerrainGeometry> Geometries;
	Vector3c Position;
	CBounds AABB;
};


// Stocke le maximum d'informations relatif au sommet
struct MegaVertex
{
	Vector3c Pos;
	Vector3c Normal;
	uint32 Colors[4] = { 0, 0, 0, 0 };
	Vector2c UV[3] = { { 0.f, 0.f }, { 0.f, 0.f }, { 0.f, 0.f } };
	Vector4c Tangent;
};

/*
	- Construction des models, géometries, vertex, index buffer.
	- Embarcation des textures (todo: créer un système de dictionnaire de textures)
*/
template<class gtaDrawableX>
bool BuildGtaDrawableX(
	TinyAllocator& alloc,
	gtaDrawableX& vDrawable,
	rdr::rage::TXD& texs,
	rdr::rage::Drawable& iD,
	Vector3c& posOut,
	DetailLevel& detailsLevel,
	bool isYDR,
	SkySharedPtr<TerrainInfos>& terrainInfo,
	float scale,
	Vector3c offset,
	char* filenameLog = 0)
{
	detailsLevel = DetailLevel::DRAWABLE_HD;

	uint64 vertexCountDrawable = 0;

	HANDLE fileLogHandle = 0;

	InitDefaultTex();
	using namespace v::rage;
	using namespace rdr::rage;

	vDrawable.m_Name = alloc.CloneStr(iD.m_Name);

	vDrawable.m_LodDistHigh = 9999.f;
	vDrawable.m_LodDistMed = 9999.f;
	vDrawable.m_LodDistLow = 9999.f;
	vDrawable.m_LodDistVeryLow = 9999.f;

	bool translateToOrigin = false;
	Vector3c positionDrawable = { 0.f, 0.f, 0.f };

	bool IsAProp = iD.m_IsAtPositionZero;
	if (!IsAProp)
	{
		translateToOrigin = true;
		if (translateToOrigin)
		{
			vDrawable.m_BoundCenter = { 0.f, 0.f, 0.f };

			positionDrawable = iD.m_BoundCenter;
			posOut = positionDrawable;

			posOut.x *= scale;
			posOut.y *= scale;
			posOut.z *= scale;

			posOut += offset;
		}
	}
	else
	{
		vDrawable.m_BoundCenter = iD.m_BoundCenter * scale;
		posOut = { 0.f, 0.f, 0.f };
	}

	vDrawable.m_SphereRadius = 0.f;// 10.f; // iD.m_SphereRadius;

	auto shaderGroup = alloc.Alloc<v::rage::grmShaderGroup>();

	// stock les index des textures à embarqué dans le drawable
	SkyMap<int, v::rage::grcTexture*> TexToEmbedMap;
	SkyVector<std::tuple<CTexMatcher, v::rage::grcTexture*, uint32>> TexToPushMap;

	// SHADERS PART
	SkyVector<v::rage::grmShader*> ShadersList;
	// auto nullShader = alloc.Alloc<v::rage::grmShader>();
	// ShadersList.push_back(nullShader);

	SkyVector<v::rage::grmModel*> ModelsList;


	int modelIdCheck = 0;
	uint16 modelsCollectId[3]{ 10, 10, 10 };
	for (int detailLvl = 0; detailLvl < 3; detailLvl++) {
		if (iD.m_Models[detailLvl].size()) {
			modelsCollectId[modelIdCheck++] = detailLvl;
		}
	}

	GeoBound drawableBounds = {};
	drawableBounds.m_AABBMax = { -9999999.f, -9999999.f, -9999999.f, -9999999.f };
	drawableBounds.m_AABBMin = { 9999999.f, 9999999.f, 9999999.f, 9999999.f };

	// MODELS PART
	for (int detailLvl = 0; detailLvl < 3; detailLvl++)
	{
		pgCollectionPtr<v::rage::grmModel>* thisModelsCollect = 0;

		uint16 modelCollId = modelsCollectId[detailLvl];
		if (modelCollId == 10)
			continue;

		auto& thisModelCollectRdr = iD.m_Models[modelsCollectId[detailLvl]];
		if (thisModelCollectRdr.size()) {
			thisModelsCollect = alloc.Alloc<pgCollectionPtr<v::rage::grmModel>>();
			thisModelsCollect->Alloc(thisModelCollectRdr.size()); // (error: alloc 0)
		}
		
		if (thisModelsCollect)
		{
			if (detailLvl == 0) {
				vDrawable.m_ModelsHigh = thisModelsCollect;
				vDrawable.m_RenderFlagHigh = 65281;
			}
			else if (detailLvl == 1) {
				vDrawable.m_ModelsMedium = thisModelsCollect;
				vDrawable.m_RenderFlagMed = 65281;
			}
			else if (detailLvl == 2) {
				vDrawable.m_ModelsLow = thisModelsCollect;
				vDrawable.m_RenderFlagLow = 65281;
			}
		}
		else
			continue;


		int modelIndex = 0;
		for (auto& iM : thisModelCollectRdr) // iteration des modèles par niveau de détails
		{
			v::rage::grmModel* iMDest = alloc.Alloc<v::rage::grmModel>();
			(*thisModelsCollect)[modelIndex++] = iMDest;

			SkyVector<GeoBound> geometriesBounds;
			SkyVector<v::rage::grmGeometry*> geometriesCollect;
			int startShaderIndex = ShadersList.size();

			GeoBound modelBound = {};
			modelBound.m_AABBMax = { -9999999.f, -9999999.f, -9999999.f, -9999999.f };
			modelBound.m_AABBMin = { 9999999.f, 9999999.f, 9999999.f, 9999999.f };

			int validGeoCount = 0;
			for (auto& iG : iM.m_Geometry) // iteration des géometries
			{
				bool isValidGeometry = true;
				// auto terrainGeometry = TerrainGeometry();

				bool geometryIsATile = false;

				GeoBound geoBound = {};
				geoBound.m_AABBMax = { -9999999.f, -9999999.f, -9999999.f, -9999999.f };
				geoBound.m_AABBMin = { 9999999.f, 9999999.f, 9999999.f, 9999999.f };

				float geoSphereRadius = 0.f;

				auto geoShader = alloc.Alloc<v::rage::grmShader>();


				/*
					0 :  default
					1 :  terrain (PNCCTX)
					2 :  terrain blend LOD (PNCCTT)
					3 :  terrain blend (PNCCTTTX)
				*/
				uint32 ShaderType = 0;
				uint32 diffuseCount = 0;
				uint32 normalsCount = 0;
				uint32 VertexStrideTarget = sizeof(VertexDefault);


				// Determine le niveau de détails du drawable
				if (true && detailsLevel != DetailLevel::DRAWABLE_SLOD)
				{
					if (false && strstr(iG.m_Shader.m_ShaderName.c_str(), "low_lod"))
						detailsLevel = DetailLevel::DRAWABLE_LOD;

					for (auto& i : iG.m_Shader.m_Parameters)
					{
						if (i.m_Type == rdr::rage::ShaderArgType::eSP_TEX_RESS)
						{
							bool isTile = strstr(i.m_TexName.c_str(), "fbtile_") || strstr(i.m_TexName.c_str(), "fdtile_");
							if (isTile || strstr(i.m_TexName.c_str(), "ultralowlod"))
							{
								if (isTile)
									geometryIsATile = true;

								detailsLevel = DetailLevel::DRAWABLE_LOD;
								// break;
							}
						}
					}
				}


				/* Reciblage heuristique des shaders RDR vers V  */
				if (true)
				{
					geoShader->SetBucket(0);

					grmShaderParamsFactory paramsFactory(&alloc);

					bool terrain = (iG.m_Shader.m_ShaderName == "rdr2_terrain_blend" || iG.m_Shader.m_ShaderName == "rdr2_terrain");
					const rdr::rage::ShaderArg* BlendMap1 = iG.m_Shader.OwnParam("TerrainBlendMap1");
					const rdr::rage::ShaderArg* BlendMap2 = iG.m_Shader.OwnParam("TerrainBlendMap2");
					const rdr::rage::ShaderArg* BlendMap = BlendMap1 ? BlendMap1 : BlendMap2;

					// v::rage::grcTexture* BlendMap1Tex = BlendMap1 ? GetAndEmbedTexture(alloc, BlendMap1->m_TexName, TexToEmbedMap, TexToPushMap, texs, TextureUsage::eTERRAIN, detailsLevel) : 0;
					// v::rage::grcTexture* BlendMap2Tex = BlendMap2 ? GetAndEmbedTexture(alloc, BlendMap2->m_TexName, TexToEmbedMap, TexToPushMap, texs, TextureUsage::eTERRAIN, detailsLevel) : 0;

					bool isWaterUni = (iG.m_Shader.m_ShaderName == "rdr2_river_water");
					bool isWaterJoin = (iG.m_Shader.m_ShaderName == "rdr2_river_water_joint"); // utilisé pour les jointures de différents types d'eaux

					// TERRAIN SHADER
					if (terrain)
					{
						// TERRAIN BLEND SHADER
						if (true && BlendMap)
						{
							geoShader->m_ShaderHash = JoaatHash("terrain_cb_w_4lyr_2tex_blend_pxm_spm");
							geoShader->m_FilenameHash = JoaatHash("terrain_cb_w_4lyr_2tex_blend_pxm_spm.sps");
							ShaderType = 3;

							auto diffuse1 = iG.m_Shader.OwnParam("TerrainDiffuseSampler1");
							paramsFactory.PushTexture(JoaatHash("TextureSampler_Layer0"), GetAndEmbedTexture(alloc, diffuse1->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDIFFUSE, detailsLevel), 2);
							
							auto normal1 = iG.m_Shader.OwnParam("TerrainNormalSampler1");
							paramsFactory.PushTexture(JoaatHash("BumpSampler_Layer0"), GetAndEmbedTexture(alloc, normal1->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eNORMAL, detailsLevel), 3);
							
							paramsFactory.PushTexture(JoaatHash("HeightMapSamplerLayer0"), GetAndEmbedTexture(alloc, "null", TexToEmbedMap, TexToPushMap, texs, TexUsage::eDEFAULT, detailsLevel), 4);
							
							auto diffuse2 = iG.m_Shader.OwnParam("TerrainDiffuseSampler2");
							paramsFactory.PushTexture(JoaatHash("TextureSampler_Layer1"), GetAndEmbedTexture(alloc, diffuse2->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDIFFUSE, detailsLevel), 5);
							
							auto normal2 = iG.m_Shader.OwnParam("TerrainNormalSampler2");
							paramsFactory.PushTexture(JoaatHash("BumpSampler_Layer1"), GetAndEmbedTexture(alloc, normal2->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eNORMAL, detailsLevel), 6);
							
							paramsFactory.PushTexture(JoaatHash("HeightMapSamplerLayer1"), GetAndEmbedTexture(alloc, "null", TexToEmbedMap, TexToPushMap, texs, TexUsage::eDEFAULT, detailsLevel), 7);
							
							auto diffuse3 = iG.m_Shader.OwnParam("TerrainDiffuseSampler3");
							paramsFactory.PushTexture(JoaatHash("TextureSampler_Layer2"), GetAndEmbedTexture(alloc, diffuse3->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDIFFUSE, detailsLevel), 8);
							
							auto normal3 = iG.m_Shader.OwnParam("TerrainNormalSampler3");
							paramsFactory.PushTexture(JoaatHash("BumpSampler_Layer2"), GetAndEmbedTexture(alloc, normal3->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eNORMAL, detailsLevel), 9);
							
							paramsFactory.PushTexture(JoaatHash("HeightMapSamplerLayer2"), GetAndEmbedTexture(alloc, "null", TexToEmbedMap, TexToPushMap, texs, TexUsage::eDEFAULT, detailsLevel), 10);
							


							auto diffuse4 = iG.m_Shader.OwnParam("TerrainDiffuseSampler4");
							paramsFactory.PushTexture(JoaatHash("TextureSampler_Layer3"), GetAndEmbedTexture(alloc, diffuse4->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDIFFUSE, detailsLevel), 11);
							
							auto normal4 = iG.m_Shader.OwnParam("TerrainNormalSampler4");
							paramsFactory.PushTexture(JoaatHash("BumpSampler_Layer3"), GetAndEmbedTexture(alloc, normal4->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eNORMAL, detailsLevel), 12);
							
							paramsFactory.PushTexture(JoaatHash("HeightMapSamplerLayer3"), GetAndEmbedTexture(alloc, "null", TexToEmbedMap, TexToPushMap, texs, TexUsage::eDEFAULT, detailsLevel), 13);



							paramsFactory.PushTexture(JoaatHash("LookupSampler"), EmbedTexture(g_NullBlendMap, TexToPushMap), 14);

							paramsFactory.PushVector4(JoaatHash("UseTessellation"), { 0.f, 0.f, 0.f, 0.f }, 186);
							paramsFactory.PushVector4(JoaatHash("MaterialWetnessMultiplier"), { 0.8f, 0.2f, 0.3f, 0.8f }, 185);
							paramsFactory.PushVector4(JoaatHash("BumpSelfShadowAmount"), { 0.3f, 0.f, 0.f, 0.f }, 184);
							paramsFactory.PushVector4(JoaatHash("HeightBias3"), { 0.015f, 0.f, 0.f, 0.f }, 183);
							paramsFactory.PushVector4(JoaatHash("HeightScale3"), { 0.03f, 0.f, 0.f, 0.f }, 182);
							paramsFactory.PushVector4(JoaatHash("HeightBias2"), { 0.015f, 0.f, 0.f, 0.f }, 181);
							paramsFactory.PushVector4(JoaatHash("HeightScale2"), { 0.03f, 0.f, 0.f, 0.f }, 180);
							paramsFactory.PushVector4(JoaatHash("HeightBias1"), { 0.015f, 0.f, 0.f, 0.f }, 179);
							paramsFactory.PushVector4(JoaatHash("HeightScale1"), { 0.03f, 0.f, 0.f, 0.f }, 178);
							paramsFactory.PushVector4(JoaatHash("HeightBias0"), { 0.015f, 0.f, 0.f, 0.f }, 169);
							paramsFactory.PushVector4(JoaatHash("HeightScale0"), { 0.03f, 0.f, 0.f, 0.f }, 168);
							paramsFactory.PushVector4(JoaatHash("ParallaxSelfShadowAmount"), { 0.95f, 0.f, 0.f, 0.f }, 167);
							paramsFactory.PushVector4(JoaatHash("Bumpiness"), { 3.f, 0.f, 0.f, 0.f }, 166);
							paramsFactory.PushVector4(JoaatHash("SpecIntensityAdjust"), { 1.f, 0.f, 0.f, 0.f }, 165);
							paramsFactory.PushVector4(JoaatHash("SpecularIntensityMultSpecMap"), { 1.f, 0.f, 0.f, 0.f }, 164);
							paramsFactory.PushVector4(JoaatHash("SpecularIntensityMult"), { 0.01f, 0.f, 0.f, 0.f }, 163);
							paramsFactory.PushVector4(JoaatHash("SpecFallOffAdjust"), { 1.f, 0.f, 0.f, 0.f }, 162);
							paramsFactory.PushVector4(JoaatHash("SpecularFallOffMultSpecMap"), { 48.f, 0.f, 0.f, 0.f }, 161);


#ifdef LOD_TERRAIN_BLEND
							iShader->m_ShaderHash = JoaatHash("terrain_cb_w_4lyr_2tex_blend_lod");
							iShader->m_FilenameHash = JoaatHash("terrain_cb_w_4lyr_2tex_blend_lod.sps");
							ShaderType = 2;
							uint32 DiffuseLayer = 0;
							uint32 NormalLayer = 0;

							for (auto& i : iG.m_Shader.m_Parameters)
							{
								if (i.m_Type == rdr::rage::ShaderArgType::eSP_TEX_RESS)
								{
									if (strstr(i.m_ParamName.c_str(), "TerrainDiffuseSampler"))
									{
										int samplerId = atoi(i.m_ParamName.c_str() + 21) - 1;
										if (samplerId <= 1)
											continue;

										terrainGeometry.TexturesName[DiffuseLayer] = i.m_TexName;

										char paramName[72];
										sprintf_s(paramName, "TextureSampler_Layer%u", DiffuseLayer++);

										v::rage::grcTexture* iTex = GetAndEmbedTexture(alloc, i.m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDEFAULT, detailsLevel);
										paramsFactory.PushTexture(JoaatHash(paramName), iTex ? iTex : g_DefaultTexture);
										diffuseCount++;
									}

									else if (false && strstr(i.m_ParamName.c_str(), "TerrainNormalSampler"))
									{
										int samplerId = atoi(i.m_ParamName.c_str() + 20) - 1;
										if (samplerId <= 1)
											continue;

										char paramName[72];
										sprintf_s(paramName, "BumpSampler_Layer%u", NormalLayer++);

										v::rage::grcTexture* iTex = GetAndEmbedTexture(alloc, i.m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eNORMAL, detailsLevel);
										paramsFactory.PushTexture(JoaatHash(paramName), iTex ? iTex : g_DefaultTexture);
										normalsCount++;
									}
								}
							}

							if (BlendMap1Tex)
								paramsFactory.PushTexture(JoaatHash("LookupSampler"), BlendMap1Tex);

							if (BlendMap2Tex && false) // toast after
								paramsFactory.PushTexture(JoaatHash("LookupSampler2"), BlendMap1Tex);

							paramsFactory.PushVector4(JoaatHash("MaterialWetnessMultiplier"), { 0.5f, 0.7f, 0.8f, 0.6f });
							paramsFactory.PushVector4(JoaatHash("BumpSelfShadowAmount"), { 0.3f, 0.f, 0.f, 0.f });
							// paramsFactory.PushVector4(JoaatHash("Bumpiness"), { 1.8f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("SpecularIntensityMult"), { 0.007f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("SpecularFallOffMult"), { 5.f, 0.f, 0.f, 0.f });
#endif

							/*
							paramsFactory.PushVector4(JoaatHash("UseTessellation"), { 0.f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("MaterialWetnessMultiplier"), { 0.3f, 0.3f, 0.3f, 0.4f });
							paramsFactory.PushVector4(JoaatHash("BumpSelfShadowAmount"), { 0.3f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("HeightBias3"), { 0.015f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("HeightScale3"), { 0.03f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("HeightBias2"), { 0.015f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("HeightScale2"), { 0.03f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("HeightBias1"), { 0.015f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("HeightScale1"), { 0.03f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("HeightBias0"), { 0.015f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("HeightScale0"), { 0.03f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("ParallaxSelfShadowAmount"), { 0.95f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("Bumpiness"), { 1.3f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("SpecIntensityAdjust"), { 1.f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("SpecularIntensityMultSpecMap"), { 1.f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("SpecularIntensityMult"), { 0.01f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("SpecFallOffAdjust"), { 1.f, 0.f, 0.f, 0.f });
							paramsFactory.PushVector4(JoaatHash("SpecularFallOffMultSpecMap"), { 48.f, 0.f, 0.f, 0.f });

							iShader->m_ShaderHash = JoaatHash("terrain_cb_w_4lyr_2tex_blend_pxm_spm");
							iShader->m_FilenameHash = JoaatHash("terrain_cb_w_4lyr_2tex_blend_pxm_spm.sps");
							*/
						}


						// TERRAIN SHADER
						else
						{
							geoShader->m_ShaderHash = JoaatHash("terrain_cb_w_4lyr");
							geoShader->m_FilenameHash = JoaatHash("terrain_cb_w_4lyr.sps");
							ShaderType = 1;

							// 0
							auto diffuse1 = iG.m_Shader.OwnParam("TerrainDiffuseSampler1");
							paramsFactory.PushTexture(JoaatHash("TextureSampler_Layer0"), GetAndEmbedTexture(alloc, diffuse1->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDIFFUSE, detailsLevel), 2);

							auto normal1 = iG.m_Shader.OwnParam("TerrainNormalSampler1");
							paramsFactory.PushTexture(JoaatHash("BumpSampler_Layer0"), GetAndEmbedTexture(alloc, normal1->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eNORMAL, detailsLevel), 3);


							// 1
							auto diffuse2 = iG.m_Shader.OwnParam("TerrainDiffuseSampler2");
							paramsFactory.PushTexture(JoaatHash("TextureSampler_Layer1"), GetAndEmbedTexture(alloc, diffuse2->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDIFFUSE, detailsLevel), 4);

							auto normal2 = iG.m_Shader.OwnParam("TerrainNormalSampler2");
							paramsFactory.PushTexture(JoaatHash("BumpSampler_Layer1"), GetAndEmbedTexture(alloc, normal2->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eNORMAL, detailsLevel), 5);


							// 2
							auto diffuse3 = iG.m_Shader.OwnParam("TerrainDiffuseSampler3");
							paramsFactory.PushTexture(JoaatHash("TextureSampler_Layer2"), GetAndEmbedTexture(alloc, diffuse3->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDIFFUSE, detailsLevel), 6);

							auto normal3 = iG.m_Shader.OwnParam("TerrainNormalSampler3");
							paramsFactory.PushTexture(JoaatHash("BumpSampler_Layer2"), GetAndEmbedTexture(alloc, normal3->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eNORMAL, detailsLevel), 7);

							
							// 3
							auto diffuse4 = iG.m_Shader.OwnParam("TerrainDiffuseSampler4");
							paramsFactory.PushTexture(JoaatHash("TextureSampler_Layer3"), GetAndEmbedTexture(alloc, diffuse4->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDIFFUSE, detailsLevel), 8);

							auto normal4 = iG.m_Shader.OwnParam("TerrainNormalSampler4");
							paramsFactory.PushTexture(JoaatHash("BumpSampler_Layer3"), GetAndEmbedTexture(alloc, normal4->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eNORMAL, detailsLevel), 9);


							auto diffuse5 = iG.m_Shader.OwnParam("TerrainDiffuseSampler5");
							GetAndEmbedTexture(alloc, diffuse5->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDIFFUSE, detailsLevel, false);

							auto normal5 = iG.m_Shader.OwnParam("TerrainNormalSampler5");
							GetAndEmbedTexture(alloc, normal5->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eNORMAL, detailsLevel, false);

							auto diffuse6 = iG.m_Shader.OwnParam("TerrainDiffuseSampler6");
							GetAndEmbedTexture(alloc, diffuse6->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDIFFUSE, detailsLevel, false);

							auto normal6 = iG.m_Shader.OwnParam("TerrainNormalSampler6");
							GetAndEmbedTexture(alloc, normal6->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eNORMAL, detailsLevel, false);
						}
					}


					// WATER SHADER
					else if (isWaterUni || isWaterJoin)
					{
						ShaderType = 4; // vertex PT

						geoShader->SetBucket(6);

						geoShader->m_ShaderHash = JoaatHash("water_fountain");
						geoShader->m_FilenameHash = JoaatHash("water_fountain.sps");

						paramsFactory.PushVector4(JoaatHash("FogColor"), { 0.227f, 0.49f, 0.58f, 0.13f }, 197);
						paramsFactory.PushVector4(JoaatHash("SpecularFalloff"), { 2000.f, 0.f, 0.f, 0.f }, 153);
						paramsFactory.PushVector4(JoaatHash("SpecularIntensity"), { 10.f, 0.f, 0.f, 0.f }, 152);
						paramsFactory.PushVector4(JoaatHash("RippleScale"), { 0.2f, 0.f, 0.f, 0.f }, 151);
						paramsFactory.PushVector4(JoaatHash("RippleBumpiness"), { 0.5f, 0.f, 0.f, 0.f }, 149); // 0.5
					}

// #define EMISSIVES
#ifdef EMISSIVES
					else if (iG.m_Shader.m_ShaderName == "rdr2_window_glow" ||
							iG.m_Shader.m_ShaderName == "rdr2_door_glow")
					{
						ShaderType = 0;

						iShader->m_ShaderHash = JoaatHash("emissive");
						iShader->m_FilenameHash = JoaatHash("emissive.sps");

						v::rage::grcTexture* iTex = GetAndEmbedTexture(alloc, "just default texture", TexToEmbedMap,
							TexToPushMap, texs, TexUsage::eDEFAULT, detailsLevel);

						paramsFactory.PushTexture(JoaatHash("DiffuseSampler"), iTex);
						paramsFactory.PushVector4(JoaatHash("MatMaterialColorScale"), { 1.f, 0.f, 0.f, 1.f });
						paramsFactory.PushVector4(JoaatHash("HardAlphaBlend"), { 1.f, 0.f, 0.f, 0.f });
						paramsFactory.PushVector4(JoaatHash("UseTessellation"), { 0.f, 0.f, 0.f, 0.f });
						paramsFactory.PushVector4(JoaatHash("EmissiveMultiplier"), { 1.5f, 0.f, 0.f, 0.f });
						paramsFactory.PushVector4(JoaatHash("GlobalAnimUV1"), { 0.f, 1.f, 0.f, 0.f });
						paramsFactory.PushVector4(JoaatHash("GlobalAnimUV0"), { 1.f, 0.f, 0.f, 0.f });
					}
#endif

					else
					{
						ShaderType = 0;

						const rdr::rage::ShaderArg* diffuseArg = iG.m_Shader.OwnParam("TextureSampler");
						if (!diffuseArg)
							diffuseArg = iG.m_Shader.OwnParam("TerrainDiffuseSampler");


						rdr::rage::ShaderArg defaultArg = {};

						if (!diffuseArg)
						{
							defaultArg.m_Type = rdr::rage::ShaderArgType::eSP_TEX_RESS;
							defaultArg.m_ParamName = "TextureSampler";
							defaultArg.m_TexName = "null";

							diffuseArg = &defaultArg;
						}

						if (diffuseArg)
						{
							if (false && strstr(diffuseArg->m_ParamName.c_str(), "ultralowlod"))
								return DetailLevel::DRAWABLE_SLOD;

							const rdr::rage::ShaderArg* bumpArg = iG.m_Shader.OwnParam("BumpSampler");

							// NORMAL SHADER
							if (bumpArg && bumpArg->m_TexName != "NULL_TEX")
							{
								geoShader->m_ShaderHash = JoaatHash("normal");
								geoShader->m_FilenameHash = JoaatHash("normal.sps");

								paramsFactory.PushTexture(JoaatHash("DiffuseSampler"), GetAndEmbedTexture(alloc, diffuseArg->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDIFFUSE, detailsLevel), 0);
								paramsFactory.PushTexture(JoaatHash("BumpSampler"), GetAndEmbedTexture(alloc, bumpArg->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eNORMAL, detailsLevel), 2);
								paramsFactory.PushVector4(JoaatHash("HardAlphaBlend"), { 1.f, 0.f, 0.f, 0.f }, 168);
								paramsFactory.PushVector4(JoaatHash("UseTessellation"), { 0.f, 0.f, 0.f, 0.f }, 167);
								paramsFactory.PushVector4(JoaatHash("WetnessMultiplier"), { 1.f, 0.f, 0.f, 0.f }, 166);
								paramsFactory.PushVector4(JoaatHash("Bumpiness"), { 1.f, 0.f, 0.f, 0.f }, 165);
								paramsFactory.PushVector4(JoaatHash("SpecularIntensityMult"), { 0.01f, 0.f, 0.f, 0.f }, 164);
								paramsFactory.PushVector4(JoaatHash("SpecularFallOffMult"), { 5.f, 0.f, 0.f, 0.f }, 163);
								paramsFactory.PushVector4(JoaatHash("SpecularFresnel"), { 0.97f, 0.f, 0.f, 0.f }, 162);
								paramsFactory.PushVector4(JoaatHash("GlobalAnimUV1"), { 0.f, 1.f, 0.f, 0.f }, 161);
								paramsFactory.PushVector4(JoaatHash("GlobalAnimUV0"), { 1.f, 0.f, 0.f, 0.f }, 160);
							}


							// DEFAULT SHADER (ONLY DIFFUSE)
							else
							{
								geoShader->m_ShaderHash = JoaatHash("default");
								geoShader->m_FilenameHash = JoaatHash("default.sps");

								paramsFactory.PushTexture(JoaatHash("DiffuseSampler"), GetAndEmbedTexture(alloc, diffuseArg->m_TexName, TexToEmbedMap, TexToPushMap, texs, TexUsage::eDIFFUSE, detailsLevel), 0);
								paramsFactory.PushVector4(JoaatHash("MatMaterialColorScale"), { 1.f, 0.f, 0.f, 1.f }, 165);
								paramsFactory.PushVector4(JoaatHash("HardAlphaBlend"), { 1.f, 0.f, 0.f, 0.f }, 164);
								paramsFactory.PushVector4(JoaatHash("UseTessellation"), { 0.f, 0.f, 0.f, 0.f }, 163);
								paramsFactory.PushVector4(JoaatHash("WetnessMultiplier"), { 1.f, 0.f, 0.f, 0.f }, 162);
								paramsFactory.PushVector4(JoaatHash("GlobalAnimUV1"), { 0.f, 1.f, 0.f, 0.f }, 161);
								paramsFactory.PushVector4(JoaatHash("GlobalAnimUV0"), { 1.f, 0.f, 0.f, 0.f }, 160);
							}
						}
						else
						{
							SkyPrint->printColor(12, "SHADER IN: %s\n", iG.m_Shader.m_ShaderName.c_str());
							return false;
						}
					}

					geoShader->m_ParamsCount = paramsFactory.GetArgsCount();
					geoShader->m_TexParamsCount = paramsFactory.GetTexArgsCount();
					geoShader->m_pShaderParameters = paramsFactory.BuildParametersBlock(geoShader->m_ParameterDataSize, geoShader->m_ParameterSize);


					if (ShaderType == 0)
						VertexStrideTarget = sizeof(VertexDefault);
					else if (ShaderType == 1)
						VertexStrideTarget = sizeof(VertexPNCCTX);
					else if (ShaderType == 2)
						VertexStrideTarget = sizeof(VertexPNCCTT);
					else if (ShaderType == 3)
						VertexStrideTarget = sizeof(VertexPNCCTTTX);
					else if (ShaderType == 4)
						VertexStrideTarget = sizeof(VertexPT);
				}


				// construction du tampon d'indices
				v::rage::grcIndexBuffer* iIDest = alloc.Alloc<v::rage::grcIndexBuffer>();
				{
					iIDest->m_IndicesCount = iG.m_IndexBuffer.m_IndexCount;

					iIDest->m_pIndexDataBlob = (uint16*)alloc.Alloc(sizeof(uint16) * iIDest->m_IndicesCount);

					for (int iit = 0; iit < iIDest->m_IndicesCount; iit++)
						iIDest->m_pIndexDataBlob[iit] = iG.m_IndexBuffer.m_IndexBuff[iit];

					if (iIDest->m_IndicesCount % 3 != 0 && iIDest->m_pIndexDataBlob[0] != 0)
						detailsLevel = DetailLevel::DRAWABLE_SLOD;
				}


				// Calcul de la déclaration des sommets
				const uint32 vertexDataSize[] = {
					2, 4, 6, 8, 4, 8, 12, 16,
					4, 4, 4, 0, 0, 0, 4, 8
				};
				uint8 PositionCount = 0;
				uint8 NormalCount = 0;
				uint8 TexcoordCount = 0;
				uint8 ColorCount = 0;
				uint8 TangentCount = 0;
				uint8 BinormalCount = 0;

				enum VertexTypeData : uint32 {
					eVTD_POSITION,
					eVTD_UV_FLOAT,
					eVTD_UV_INT,
					eVTD_NORMAL,
					eVTD_COLOR,
					eVTD_TANGENT,
					eVTD_BINORMAL
				};

				uint32 vertexSizeMap[24];
				uint32 VertexTypeMap[24];
				memset(vertexSizeMap, 0x00, sizeof(vertexSizeMap));
				memset(VertexTypeMap, 0xFF, sizeof(VertexTypeMap));

				// décomposition de la sémantique des sommets de RDR 1
				for (uint32 tt = 0, i = 0; tt < 18; tt++)
				{
					uint32 iMaskFlag = InvEnd(1 << tt);
					if ((iMaskFlag & iG.m_VertexBuffer.m_VertexFlags) == iMaskFlag)
					{
						uint64 tti = iG.m_VertexBuffer.m_VertexType;
						uint64 tti0 = tti >> (4 * tt);
						uint64 pTT = ((4 * tti0) & 0x3C);

						uint32 indexInDataSize = (uint32)pTT / 4;
						vertexSizeMap[tt] = vertexDataSize[indexInDataSize];

						if (indexInDataSize == 6) {
							PositionCount++;
							VertexTypeMap[tt] = eVTD_POSITION;
						}
						else if (indexInDataSize == 10)
						{
							if (NormalCount == 0) {
								NormalCount++;
								VertexTypeMap[tt] = eVTD_NORMAL;
							}
							else if (TangentCount == 0) {
								TangentCount++;
								VertexTypeMap[tt] = eVTD_TANGENT;
							}
							else if (BinormalCount == 0) {
								BinormalCount++;
								VertexTypeMap[tt] = eVTD_BINORMAL;
							}
						}
						else if (indexInDataSize == 1)
						{
							TexcoordCount++;
							VertexTypeMap[tt] = eVTD_UV_FLOAT;
						}
						else if (indexInDataSize == 9)
						{
							ColorCount++;
							VertexTypeMap[tt] = eVTD_COLOR;
						}
						else if (indexInDataSize == 14)
						{
							TexcoordCount++;
							VertexTypeMap[tt] = eVTD_UV_INT;
						}
					}
				}




				// Reciblage et reconstruction du tampon de sommets (+ recalcul des normals/binormal/tangente)
				v::rage::grcVertexBuffer* iVDest = alloc.Alloc<v::rage::grcVertexBuffer>();
				{
					uint16 vertexStrideInput = iG.m_VertexBuffer.m_VertexStride;
					iVDest->m_VertexCount = iG.m_VertexCount;

					rdr::rage::VertexType vertexTypeIn = iG.m_VertexBuffer.m_VertexType;
					auto vertexInfo = alloc.Alloc<v::rage::VertexInfo>();
					vertexInfo->m_Flags = v::rage::VertexType::eVT_Default;
					vertexInfo->m_Count = 4;

					if (ShaderType == 1) {
						vertexInfo->m_Flags = v::rage::VertexType::eVT_PNCCTX;
						vertexInfo->m_Count = 6;
					}
					else if (ShaderType == 2) {
						vertexInfo->m_Flags = v::rage::VertexType::eVT_PNCCTT;
						vertexInfo->m_Count = 6;
					}
					else if (ShaderType == 3) {
						vertexInfo->m_Flags = v::rage::VertexType::eVT_PNCCTTTX;
						vertexInfo->m_Count = 8;
					}
					else if (ShaderType == 4) {
						vertexInfo->m_Flags = v::rage::VertexType::eVT_PT;
						vertexInfo->m_Count = 2;
					}

					iVDest->m_pVertexInfo = vertexInfo;



					if (vertexStrideInput > 12)
					{
						uint8* pDataVertex = iG.m_VertexBuffer.m_VertexBuff.data();
						for (int i = 0; i < iG.m_VertexCount; i++)
						{
							Vector3c* pPos = (Vector3c*)(pDataVertex + (i * vertexStrideInput));
							auto pVec = Vector3c(InvEnd(pPos->x), InvEnd(pPos->z), InvEnd(pPos->y));
							pVec.FlipXY();

							if (translateToOrigin)
								pVec -= positionDrawable;

							pVec *= scale;
							*pPos = pVec;

							if (pVec.x < geoBound.m_AABBMin.x) geoBound.m_AABBMin.x = pVec.x;
							if (pVec.y < geoBound.m_AABBMin.y) geoBound.m_AABBMin.y = pVec.y;
							if (pVec.z < geoBound.m_AABBMin.z) geoBound.m_AABBMin.z = pVec.z;
							if (pVec.x > geoBound.m_AABBMax.x) geoBound.m_AABBMax.x = pVec.x;
							if (pVec.y > geoBound.m_AABBMax.y) geoBound.m_AABBMax.y = pVec.y;
							if (pVec.z > geoBound.m_AABBMax.z) geoBound.m_AABBMax.z = pVec.z;

							// calcule du sphere radius du drawable
							static const Vector3c centerVec = Vector3c(0.f, 0.f, 0.f);
							float DistVertexToCenter = pPos->VDIST(centerVec);
							if (DistVertexToCenter > geoSphereRadius)
								geoSphereRadius = DistVertexToCenter;
						}


						// Recalcule rapide la normal des sommets
						Vector3c* normals = (Vector3c*)g_LocalThreadBuffer; //new Vector3c[iG.m_VertexCount];
						for (int i = 0; i < iG.m_VertexCount; i++)
							normals[i] = { 0.f, 0.f, 0.f };

						if (iIDest->m_IndicesCount % 3 == 0) {
							uint16* pIndices = iIDest->m_pIndexDataBlob;
							auto pVertex = pDataVertex;
							CVertexAccess vertexAccess(pVertex, vertexStrideInput);
							for (uint32 i = 0; i < iIDest->m_IndicesCount; i += 3)
							{
								auto A0 = vertexAccess.GetPos(pIndices[i]);
								auto B0 = vertexAccess.GetPos(pIndices[i + 1]);
								auto C0 = vertexAccess.GetPos(pIndices[i + 2]);

								XMVECTOR A = XMLoadFloat3((XMFLOAT3*)&A0);
								XMVECTOR B = XMLoadFloat3((XMFLOAT3*)&B0);
								XMVECTOR C = XMLoadFloat3((XMFLOAT3*)&C0);
								XMVECTOR AB = B - A;
								XMVECTOR AC = C - A;

								XMVECTOR ABxAC = DirectX::XMVector3Cross(AB, AC);
								XMFLOAT3 ABxACF3;
								XMStoreFloat3(&ABxACF3, ABxAC);
								Vector3c ABxAC_Vec3 = { ABxACF3.x, ABxACF3.y, ABxACF3.z };

								normals[pIndices[i]] += ABxAC_Vec3;
								normals[pIndices[i + 1]] += ABxAC_Vec3;
								normals[pIndices[i + 2]] += ABxAC_Vec3;
							}
						}

						// bool isTerrain = terrainInfo.get() != 0;
						// if (isTerrain) {
						// 	terrainGeometry.Vertices = new TerrainVertex[iG.m_VertexCount];
						// 	terrainGeometry.VerticesCount = iG.m_VertexCount;
						// 
						// 	terrainGeometry.SurfaceSize = {
						// 		fabsf(geoBound.m_AABBMax.x - geoBound.m_AABBMin.x),
						// 		fabsf(geoBound.m_AABBMax.y - geoBound.m_AABBMin.y)
						// 	};
						// }

						// LOG
						if (false)
						{
							if (!fileLogHandle)
							{
								std::stringstream finalPath;
								finalPath << "C:\\UV_SAMPLES\\";

								uint32 vertexFlag = (iG.m_VertexBuffer.m_VertexFlags);
								for (int eer = 0; eer < ARRAYSIZE(v::rage::g_SEMANTIC_FLAGS_STR); eer++) {
									if (v::rage::g_SEMANTIC_FLAGS_STR[eer].m_Value == vertexFlag) {
										finalPath << v::rage::g_SEMANTIC_FLAGS_STR[eer].m_Name;
										finalPath << "_";
										break;
									}
								}

								finalPath << vertexStrideInput;
								finalPath << "_";

								finalPath << filenameLog;

								fileLogHandle = CreateFileA(finalPath.str().c_str(), GENERIC_WRITE, 0, 0,
									CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0);
							}

							char bufZERO[64];
							memset(bufZERO, 0x00, 64);

							for (int vv = 0; vv < 24; vv++)
								WriteFileS(fileLogHandle, bufZERO, vertexStrideInput);

							WriteFileS(fileLogHandle, (char*)&vertexStrideInput, sizeof(vertexStrideInput));

							for (int vv = 0; vv < 12; vv++)
								WriteFileS(fileLogHandle, bufZERO, vertexStrideInput);
						}

						bool VERTEX_LOG = false;

						if (VERTEX_LOG)
							SkyPrint->print("\n\n\n\n\nGeometry %u vertex  (vertex stride: %u)\n", iG.m_VertexCount, vertexStrideInput);
						

						vertexCountDrawable += iG.m_VertexCount;
						char* vertexBuff = (char*)alloc.Alloc(VertexStrideTarget * iG.m_VertexCount);
						for (int vv = 0; vv < iG.m_VertexCount; vv++)
						{
							char* ipVertex = (char*)(iG.m_VertexBuffer.m_VertexBuff.data() + (vv * vertexStrideInput));
							char* ipVertexOut = vertexBuff + (vv * VertexStrideTarget);
							memset(ipVertexOut, 0x00, VertexStrideTarget);
							MegaVertex megaVertex = {};

							// printHexBlob(ipVertex, vertexStrideInput, 0);

							// LOG
							if (true) {
								if (fileLogHandle)
									WriteFileS(fileLogHandle, ipVertex, vertexStrideInput);
							}

							uint8 colorWritten = 0;
							uint8 uvWritten = 0;
							for (int i = 0; i < 18; i++)
							{
								switch (VertexTypeMap[i])
								{
								case eVTD_POSITION:
								{
									megaVertex.Pos.x = (*(float*)ipVertex);
									megaVertex.Pos.y = (*(float*)(ipVertex + 4));
									megaVertex.Pos.z = (*(float*)(ipVertex + 8));

									// WRITE NORMAL
									megaVertex.Normal = { normals[vv].x, normals[vv].y, normals[vv].z };

									// if (isTerrain)
									// 	terrainGeometry.Vertices[vv].Position = megaVertex.Pos + posOut;
								}
								break;

								case eVTD_NORMAL:
								{
									// uint32 vec3n = InvEnd(*(uint32*)ipVertex);
									// megaVertex.Normal = UnpackDec3N(vec3n);
								}
								break;


								case eVTD_TANGENT:
								{
									uint32 vec3 = InvEnd(*(uint32*)ipVertex);
									Vector3c vec3t = UnpackDec3N(vec3);
									megaVertex.Tangent = Vector4c(vec3t.x, vec3t.y, vec3t.z, 1.f);
								}
								break;

								case eVTD_BINORMAL:
								{
									break;
								}

								case eVTD_COLOR:
								{
									megaVertex.Colors[colorWritten] = InvEnd(*(uint32*)ipVertex);

									if (true) {
										if (colorWritten == 0)
											megaVertex.Colors[0] = 0x00FF00FF;//(0xA210CBFF);// InvEnd(*(uint32*)ipVertex);
										else if (colorWritten == 1 && (ShaderType == 1 || ShaderType == 2 || ShaderType == 3)) { }
									}

									colorWritten++;
									break;
								}


								case eVTD_UV_FLOAT:
								{
									rdr::rage::UV* iUV = (rdr::rage::UV*)ipVertex;

									megaVertex.UV[uvWritten].x = (Float16To32(InvEnd(iUV->u)));
									megaVertex.UV[uvWritten].y = (Float16To32(InvEnd(iUV->v)));

									uvWritten++;
								}
									break;

								case eVTD_UV_INT:
								{
									int16* iUV = (int16*)ipVertex;

									uint16 toUnsignedU = InvEnd(iUV[0]) + 32767;
									uint16 toUnsignedV = InvEnd(iUV[1]) + 32767;

									megaVertex.UV[uvWritten].x = (float)toUnsignedU / 65536.f;
									megaVertex.UV[uvWritten].y = (float)toUnsignedV / 65536.f;

									if (false) // experimental
									{
										if (vertexTypeIn != eVT_OTHER01)
										{
											uint16* iUV = (uint16*)ipVertex;
											megaVertex.UV[uvWritten].x = ((float)InvEnd(iUV[0]) / 65535.f);
											megaVertex.UV[uvWritten].y = ((float)InvEnd(iUV[1]) / 65535.f);
										}
										else
										{
											// EXPERIMENTAL
											uint16* iUV = (uint16*)ipVertex;

											megaVertex.UV[uvWritten].x = (((float)InvEnd(iUV[0])) / 65535.f) * 128.f;
											megaVertex.UV[uvWritten].y = (((float)InvEnd(iUV[1])) / 65535.f) * 128.f;

											if (false)
											{
												// rdr::rage::UV* iUV = (rdr::rage::UV*)ipVertex;
												uint16 uv[2];

												// essaye les bits [7;16] et relativise les coordonnées UV
												const uint16 offsetBit = 7;
												float divider = (float)(IntPower(2, 16 - offsetBit) - 1);

												uv[0] = InvEnd(iUV[0]);
												uv[1] = InvEnd(iUV[1]);

												uv[0] <<= offsetBit;
												uv[0] >>= offsetBit;
												uv[1] <<= offsetBit;
												uv[1] >>= offsetBit;

												megaVertex.UV[uvWritten].y = (((float)uv[0]) / divider);
												megaVertex.UV[uvWritten].x = (((float)uv[1]) / divider);
												// printf("%.4f, %.4f   ", ((float)uv[0]) / divider, ((float)uv[1]) / divider);
											}
										}
									}

									uvWritten++;
								}
									break;
								}

								ipVertex += vertexSizeMap[i];
							}


							// GENERATE COLOR 2
							if ((ShaderType == 1 || ShaderType == 2 || ShaderType == 3))
							{
								megaVertex.Colors[1] = 0x00000000;

								if (false)
								{
									int randColor = (rand() % 4);

									if (randColor == 0)
										megaVertex.Colors[1] = 0xFF000000; // tex 1 (noir)
									else if (randColor == 1)
										megaVertex.Colors[1] = 0xFFFF0000; // tex 2 (bleu)
									else if (randColor == 2)
										megaVertex.Colors[1] = 0xFF00FF00; // tex 3 (vert)
									else
										megaVertex.Colors[1] = 0xFFFFFF00; // tex 4 (cyan)
								}
							}


							if (ShaderType == 1)
							{
								// WRITE TANGENT
								VertexPNCCTX* outVertex = (VertexPNCCTX*)ipVertexOut;
								outVertex->Position = megaVertex.Pos;
								outVertex->Color = megaVertex.Colors[0];
								outVertex->Color2 = megaVertex.Colors[1];
								outVertex->Normal = megaVertex.Normal;
								outVertex->Tangent = megaVertex.Tangent;
								outVertex->Texcoord = megaVertex.UV[0];
								outVertex->Texcoord.x *= 256.f; // 128.f
								outVertex->Texcoord.y *= 256.f;

								megaVertex.Tangent = { 0.f, 0.f, 0.f, 1.f };
							}
							else if (ShaderType == 2) // PNCCTT  --  TERRAIN BLENDING LOD
							{
								VertexPNCCTT* outVertex = (VertexPNCCTT*)ipVertexOut;
								outVertex->Position = megaVertex.Pos;
								outVertex->Color = megaVertex.Colors[0];// megaVertex.Colors[0];
								outVertex->Color2 = megaVertex.Colors[1];// megaVertex.Colors[1];
								outVertex->Normal = megaVertex.Normal;

								/*
									UV diffuse  n'existe pas dans les terrains blend de RDR, ici on prend l'UV lookup
									et on le répète 128 fois pour répeter la texture dans le bloc de terrain
								*/
								outVertex->Texcoord = megaVertex.UV[0];
								outVertex->Texcoord.x *= 256.f;
								outVertex->Texcoord.y *= 256.f;

								// UV 0  représente le lookup dans RDR
								outVertex->Texcoord2 = megaVertex.UV[0];
								outVertex->Texcoord2.x *= 2.f;
								outVertex->Texcoord2.y *= 2.f;
							}
							else if (ShaderType == 3) // PNCCTTTX  --  TERRAIN BLENDING
							{
								VertexPNCCTTTX* outVertex = (VertexPNCCTTTX*)ipVertexOut;
								outVertex->Position = megaVertex.Pos;
								outVertex->Color = megaVertex.Colors[0];// megaVertex.Colors[0];
								outVertex->Color2 = megaVertex.Colors[1];// megaVertex.Colors[1];
								outVertex->Normal = megaVertex.Normal;
								outVertex->Tangent = megaVertex.Tangent;

								/*
									UV diffuse  n'existe pas dans les terrains blend de RDR, ici on prend l'UV lookup
									et on le répète 100 fois pour répeter la texture dans le bloc de terrain
								*/
								outVertex->Texcoord = megaVertex.UV[0];
								outVertex->Texcoord.x *= 256.f;
								outVertex->Texcoord.y *= 256.f;

								// UV 0  représente le lookup dans RDR
								outVertex->Texcoord2 = megaVertex.UV[0];
								outVertex->Texcoord2.x *= 2.f;
								outVertex->Texcoord2.y *= 2.f;

								outVertex->Texcoord3.x = outVertex->Texcoord.x;
								outVertex->Texcoord3.y = outVertex->Texcoord.y;

								megaVertex.Tangent = { 0.f, 0.f, 0.f, 1.f };
							}
							else if (ShaderType == 4)
							{
								VertexPT* outVertex = (VertexPT*)ipVertexOut;
								outVertex->Position = megaVertex.Pos;
								outVertex->Texcoord = megaVertex.UV[0];
							}
							else if (ShaderType == 0)
							{
								VertexDefault* outVertex = (VertexDefault*)ipVertexOut;
								outVertex->Position = megaVertex.Pos;
								outVertex->Color = megaVertex.Colors[0];
								outVertex->Normal = megaVertex.Normal;

								if (vertexTypeIn == eVT_OTHER01)
								{
									outVertex->Texcoord = megaVertex.UV[uvWritten - 1];
									if (uvWritten >= 2) {
										outVertex->Texcoord.x *= 64.f;
										outVertex->Texcoord.y *= 64.f;
									}
								}
								else
									outVertex->Texcoord = megaVertex.UV[0];

								if (geometryIsATile) {
									outVertex->Texcoord.x *= 2.f;
									outVertex->Texcoord.y *= 2.f;
								}

							}


							if (VERTEX_LOG)
								SkyPrint->print("\n");
						}

						iVDest->m_pVertexDataBlob = (void*)vertexBuff;
						iVDest->m_pVertexDataBlob1 = (void*)vertexBuff;

						vertexInfo->m_Stride = VertexStrideTarget;
						iVDest->m_Stride = VertexStrideTarget;
					}
					else
						isValidGeometry = false;

				}


				// TOAST MARKER
				// return BuildResult::DRAWABLE_SLOD;


				// construction de la géometrie
				v::rage::grmGeometry* iGDest = alloc.Alloc<v::rage::grmGeometry>();
				{
					iGDest->m_IndicesCount = iIDest->m_IndicesCount;
					iGDest->m_TrianglesCount = iG.m_TrianglesCount;
					iGDest->m_VertexCount = iG.m_VertexCount;
					iGDest->m_pVertexBuff = iVDest;
					iGDest->m_pIndexBuff = iIDest;
					iGDest->m_pVertexDataBlob = iVDest->m_pVertexDataBlob;
					iGDest->m_Stride = iVDest->m_Stride;
				}


				// Si la géométrie est validée, alors elle est intégrée au grmModel parent
				if (isValidGeometry)
				{
					// étend l'AABB du modèle si nécessaire
					if (geoBound.m_AABBMin.x < modelBound.m_AABBMin.x) modelBound.m_AABBMin.x = geoBound.m_AABBMin.x;
					if (geoBound.m_AABBMin.y < modelBound.m_AABBMin.y) modelBound.m_AABBMin.y = geoBound.m_AABBMin.y;
					if (geoBound.m_AABBMin.z < modelBound.m_AABBMin.z) modelBound.m_AABBMin.z = geoBound.m_AABBMin.z;

					if (geoBound.m_AABBMax.x > modelBound.m_AABBMax.x) modelBound.m_AABBMax.x = geoBound.m_AABBMax.x;
					if (geoBound.m_AABBMax.y > modelBound.m_AABBMax.y) modelBound.m_AABBMax.y = geoBound.m_AABBMax.y;
					if (geoBound.m_AABBMax.z > modelBound.m_AABBMax.z) modelBound.m_AABBMax.z = geoBound.m_AABBMax.z;

					// idem pour l'AABB du drawable
					if (geoBound.m_AABBMin.x < vDrawable.m_BBoxMin.x) vDrawable.m_BBoxMin.x = geoBound.m_AABBMin.x;
					if (geoBound.m_AABBMin.y < vDrawable.m_BBoxMin.y) vDrawable.m_BBoxMin.y = geoBound.m_AABBMin.y;
					if (geoBound.m_AABBMin.z < vDrawable.m_BBoxMin.z) vDrawable.m_BBoxMin.z = geoBound.m_AABBMin.z;

					if (geoBound.m_AABBMax.x > vDrawable.m_BBoxMax.x) vDrawable.m_BBoxMax.x = geoBound.m_AABBMax.x;
					if (geoBound.m_AABBMax.y > vDrawable.m_BBoxMax.y) vDrawable.m_BBoxMax.y = geoBound.m_AABBMax.y;
					if (geoBound.m_AABBMax.z > vDrawable.m_BBoxMax.z) vDrawable.m_BBoxMax.z = geoBound.m_AABBMax.z;


					// étend le sphere radius du drawable si nécessaire
					if (geoSphereRadius > vDrawable.m_SphereRadius)
						vDrawable.m_SphereRadius = geoSphereRadius;


					geometriesCollect.push_back(iGDest);
					geometriesBounds.push_back(geoBound);
					ShadersList.push_back(geoShader);

					validGeoCount++;
				}
			}


			if (validGeoCount > 0)
			{
				// Construction de la Geo Boundaries Collection
				if (geometriesBounds.size())
				{
					int geoBoundsCount = (geometriesBounds.size() == 1 ? 1 : (geometriesBounds.size() + 1));
					iMDest->m_GeoBounds = (GeoBound*)alloc.Alloc(sizeof(GeoBound) * geoBoundsCount);


					modelBound.m_AABBMin.w = modelBound.m_AABBMin.x;
					modelBound.m_AABBMax.w = modelBound.m_AABBMax.x;
					iMDest->m_GeoBounds[0] = modelBound;

					if (geometriesBounds.size() > 1)
					{
						int geoBoundId = 1;
						for (const auto& iGB : geometriesBounds)
						{
							GeoBound iGBCopy = iGB;
							// iGBCopy.m_AABBMin.w = 0.f;
							// iGBCopy.m_AABBMax.w = 0.f;
							iGBCopy.m_AABBMin.w = iGBCopy.m_AABBMin.x;
							iGBCopy.m_AABBMax.w = iGBCopy.m_AABBMax.x;
							iMDest->m_GeoBounds[geoBoundId++] = iGBCopy;
						}
					}
				}


				/*
					intègre les géométries à la collection du modèle
					et intègre les indices de shader des géométries au mapping du modèle
				*/
				int geoCount = geometriesCollect.size();
				iMDest->m_GeoCollection.Alloc(geoCount);
				iMDest->m_pShaderMapping = (uint16*)alloc.Alloc(sizeof(uint16) * geoCount);
				iMDest->m_ShaderMappingCount = geoCount;
				for (int w = 0; w < geoCount; w++)
				{
					iMDest->m_GeoCollection[w] = geometriesCollect[w];
					iMDest->m_pShaderMapping[w] = startShaderIndex + w;
				}


				ModelsList.push_back(iMDest);
			}
		}
	}


	// Construction des textures embed
	uint32 texturesCount = TexToEmbedMap.size() + TexToPushMap.size();
	if (texturesCount > 0)
	{
		auto texDict = alloc.Alloc<v::rage::grcTextureDictionnary>();
		texDict->m_TexturesHashs.Alloc(texturesCount);
		texDict->m_Textures.Alloc(texturesCount);

		int it = 0;
		if (TexToEmbedMap.size())
		{
			// Converti et insère les textures embed à embarqué
			for (const auto i : TexToEmbedMap)
			{
				Texture& iTex = texs.m_Texs[i.first];
				v::rage::grcTexture* oTex = i.second;

				texDict->m_TexturesHashs[it] = iTex.m_Hash;
				texDict->m_Textures[it] = oTex;
				if (ConvertTextureRdrToV(alloc, iTex, oTex)) { }

				it++;
			}
		}

		if (TexToPushMap.size())
		{
			for (const auto& i : TexToPushMap) {
				texDict->m_TexturesHashs[it] = std::get<2>(i);
				texDict->m_Textures[it] = std::get<1>(i);
				it++;
			}
		}

		shaderGroup->m_pTexDictionnary = texDict;
	}
	else
		shaderGroup->m_pTexDictionnary = 0;


	// Construction du groupe de shaders
	shaderGroup->m_ShadersCollection.Alloc(ShadersList.size());

	for (int i = 0; i < ShadersList.size(); i++)
		shaderGroup->m_ShadersCollection[i] = ShadersList[i];

	vDrawable.m_pShaderGroup = shaderGroup;


	if (ModelsList.size())
	{
		vDrawable.m_AllModels = alloc.Alloc<pgCollectionPtr<v::rage::grmModel>>();
		vDrawable.m_AllModels->Alloc(ModelsList.size()); // (error: alloc 0)
		int iM = 0;
		for (const auto i : ModelsList) {
			(*vDrawable.m_AllModels)[iM++] = i;
		}
	}

	if (fileLogHandle)
		CloseHandle(fileLogHandle);


	if (vertexCountDrawable <= 3)
		return false;

	return true;
}
