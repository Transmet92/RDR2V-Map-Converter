#pragma once
#include "VegetationGenerator.h"
#include "SkyCommons/SkyBase.h"

static HANDLE g_DumpVertexFile = 0;
static void Dump(Vector3c vertex)
{
	if (g_DumpVertexFile == 0)
		g_DumpVertexFile = CreateFileA("C:\\Users\\Unknown8192\\Desktop\\Nouveau dossier (5)\\DUMP_VERTEX_LIST.txt", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	char dump[256];
	sprintf_s(dump, "%.3f %.3f %.3f\n", vertex.x, vertex.y, vertex.z);

	DWORD nullle;
	WriteFile(g_DumpVertexFile, dump, strlen(dump), &nullle, 0);
}
static void DumpFlush() {
	FlushFileBuffers(g_DumpVertexFile);
}


void GenerateVegetation(CArea* pArea, const gtaDrawablePG& drawable, const Vector3c& pos)
{
	return;

	// iterate models of drawable
	for (const auto i : *drawable.m_AllModels)
	{
		// iterate geometries
		int geometriesCount = i->m_GeoCollection.GetCount();
		for (int y = 0; y < geometriesCount; y++)
		{
			const auto& iGeoShader = drawable.m_pShaderGroup->m_ShadersCollection[i->m_pShaderMapping[y]];
			const auto& iGeo = *(i->m_GeoCollection[y]);

			// const char* vertexBuff = ((char*)iGeo.m_pVertexBuff->m_pVertexDataBlob);
			// for (int i = 0; i < iGeo.m_VertexCount; i++)
			// {
			// 	Vector3c* pVertex = (Vector3c*)(vertexBuff + (i * iGeo.m_Stride));
			// 
			// 	Vector3c vertexFinal = *pVertex;
			// 	vertexFinal += pos;
			// 	Dump(vertexFinal);
			// }

			bool isGrassable = false;

			bool isGeoDry = false;
			bool isGeoMidDry = false;
			iGeoShader->ForeachTextures([&](v::rage::grcTextureBase* tex)
			{
				if (strstr(tex->m_pName, "die_0")) {
					isGeoDry = true;
					isGrassable = true;
				}
				else if (strstr(tex->m_pName, "hen_01")) {
					isGeoDry = true;
					isGrassable = true;
				}
				else if (strstr(tex->m_pName, "cho_01")) {
					isGeoMidDry = true;
					isGrassable = true;
				}
				else if (strstr(tex->m_pName, "gre_01")) {
					isGeoMidDry = true;
					isGrassable = true;
				}
				else if (strstr(tex->m_pName, "gri_01")) {
					isGeoMidDry = true;
					isGrassable = true;
				}
				else if (strstr(tex->m_pName, "gap_01")) {
					isGeoMidDry = true;
					isGrassable = true;
				}
				else if (strstr(tex->m_pName, "pun_03")) {
					isGeoMidDry = true;
					isGrassable = true;
				}
				else if (strstr(tex->m_pName, "per_01")) {
					isGeoMidDry = true;
					isGrassable = true;
				}
				else if (strstr(tex->m_pName, "rio_01")) {
					isGeoMidDry = true;
					isGrassable = true;
				}
			});


			if (isGrassable)
			{
				SkyVector<Vector3c> vertices = {};

				const char* vertexBuff = ((char*)iGeo.m_pVertexBuff->m_pVertexDataBlob);
				for (int i = 0; i < iGeo.m_VertexCount; i++) {
					Vector3c* pVertex = (Vector3c*)(vertexBuff + (i * iGeo.m_Stride));
					vertices.push_back(*pVertex);
				}


				// subdivision du mesh
				uint16* indexBuff = iGeo.m_pIndexBuff->m_pIndexDataBlob;
				for (int i = 0; i < iGeo.m_IndicesCount; i += 3)
				{
					Triangle tri = {
						*(Vector3c*)(vertexBuff + (indexBuff[i] * iGeo.m_Stride)),
						*(Vector3c*)(vertexBuff + (indexBuff[i + 1] * iGeo.m_Stride)),
						*(Vector3c*)(vertexBuff + (indexBuff[i + 2] * iGeo.m_Stride))
					};

					struct SubDiv {
						Triangle tris[3];
					};
					static auto SubdivTri = [](const Triangle& tri) -> SubDiv
					{
						static auto VSlerp = [](const Vector3c& a, const Vector3c& b) -> Vector3c {
							return { a.x + ((b.x - a.x) * 0.5f), a.y + ((b.y - a.y) * 0.5f), a.z + ((b.z - a.z) * 0.5f) };
						};
						Vector3c centroid = {
							(tri.vertex[0].x + tri.vertex[1].x + tri.vertex[2].x) / 3.f,
							(tri.vertex[0].y + tri.vertex[1].y + tri.vertex[2].y) / 3.f,
							(tri.vertex[0].z + tri.vertex[1].z + tri.vertex[2].z) / 3.f
						};

						SubDiv toret;
						toret.tris[0] = { centroid, tri.vertex[0], tri.vertex[1] };
						toret.tris[1] = { centroid, tri.vertex[1], tri.vertex[2] };
						toret.tris[2] = { centroid, tri.vertex[0], tri.vertex[2] };
						return toret;
					};

					const int SUBDIV_ITERATIONS = 5;

					CSkyDynStack<SubDiv> trianglesStack;
					trianglesStack.Push(SubdivTri(tri));

					for (int it = 1; trianglesStack.GetDepth();)
					{
						auto iSubDiv = trianglesStack.Pop();
						vertices.push_back(iSubDiv.tris[0].vertex[0]);

						if (it++ == SUBDIV_ITERATIONS)
							break;

						for (int u = 0; u < 3; u++)
							trianglesStack.Push(SubdivTri(iSubDiv.tris[u]));
					}
				}


				/*
					Lissage des probabilité indépendament de la densité regionale des sommets
				 
					METHODE 1:
					- calcul une position aléatoire dans les limites X et Y de la géometrie
					- lance un rayon sur l'axe Z pour determiner des points exacts
				*/

				int indexInGeoBounds = y + 1;
				if (geometriesCount == 1)
					indexInGeoBounds = 0;

				const auto& geoBounds = i->m_GeoBounds[indexInGeoBounds];
				float surfaceSize = (fabsf(geoBounds.m_AABBMin.x) + fabsf(geoBounds.m_AABBMax.x)) *
					(fabsf(geoBounds.m_AABBMin.y) + fabsf(geoBounds.m_AABBMax.y));

				Vector3c drawableBoxMin = drawable.m_BBoxMin;
				Vector3c drawableBoxMax = drawable.m_BBoxMax;

				Vector3c drawableSize = drawableBoxMin - drawableBoxMax;


				struct Proc {
					const char* name;
					float density; // [0,1]
					int type; // 0 = desert
				};

				Proc procs[] = {
					{ "proc_desert_sage_01", 1.f, 0 },
					{ "proc_drygrasses01", 1.f, 0 },
					{ "proc_drygrasses01b", 1.f, 0 },
					{ "proc_dry_plants_01", 0.2f, 0 },
					{ "proc_stones_02", 0.1f, 0 },
					{ "proc_grassdandelion01", 0.5f, 0 },
					{ "proc_scrub_bush01", 0.2f, 0 },
				};

				int procMatchesCount = ARRAYSIZE(procs);
				for (int e = 0; e < ARRAYSIZE(procs); e++)
				{
					GrassBatchDef grassBatch = {};
					grassBatch.m_AABB.Min = drawableBoxMin + pos;
					grassBatch.m_AABB.Max = drawableBoxMax + pos;
					grassBatch.m_ModelName = procs[e].name;
					grassBatch.m_LODdist = 300;
					grassBatch.m_Scale = { 1.2f, 1.2f, 1.f };


					int grassCount = (int)((surfaceSize * 0.15f) * procs[e].density); //+ (((-0.5f + randf()) * 0.2f) * 200.f));
					if (grassCount)
					{
						SkyVector<SkyTuple<Vector3c, Vector3c>> grassPos; // 1: pos relative, 2: pos global

						for (int ig = 0; ig < grassCount; ig++)
						{
							Vector3c targetVertex = vertices[rand() % vertices.size()];
							grassPos.push_back(std::make_tuple(targetVertex, targetVertex + pos));
						}


						// [postop] Uniformisation de la distribution des procedurales
						const float CLEAN_MIN_DISTANCE = 0.15f;
						auto& grassList = grassBatch.m_GrassList;
						for (int q = 0; q < grassPos.size(); q++)
						{
							const auto& qVec = std::get<1>(grassPos[q]);
							for (int r = 0; r < grassPos.size(); r++)
							{
								// supprime tout les elements proche
								const auto& rVec = std::get<1>(grassPos[r]);
								if (rVec.VDIST(qVec) < CLEAN_MIN_DISTANCE)
								{
									grassPos.erase(grassPos.begin() + r);
								}
							}
						}


						if (grassPos.size())
						{
							for (const auto& q : grassPos)
							{
								const Vector3c& qPos = std::get<0>(q);
								Vector3c posRelInBox = (qPos - drawableBoxMin) / drawableSize;

								uint16 XGrass = (uint16)(posRelInBox.x * 65535.f);
								uint16 YGrass = (uint16)(posRelInBox.y * 65535.f);
								uint16 ZGrass = (uint16)(posRelInBox.z * 65535.f);


								const ColorRGB colors[] = {
									{ 161, 153, 124 },// desert (dry)
									{ 191, 176, 149 },// desert (dry)
									{ 160, 151, 118 },// desert (dry)
									{ 164, 156, 124 },// desert (dry)
								};

								ColorRGB thisColor = colors[rand() % ARRAYSIZE(colors)];

								uint8 scale = 230 + (rand() % 20);

								grassBatch.m_GrassList.push_back({
									XGrass, YGrass, ZGrass,
									thisColor, // color
									126, 128, // normal xy
									scale, // scale
									255 // AO
								});
							}

							pArea->AddGrassBatch(grassBatch);
						}
					}
				}
			}


		}
	}
	// DumpFlush();
}
