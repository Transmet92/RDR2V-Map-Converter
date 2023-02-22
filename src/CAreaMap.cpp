#include "CMapBuilder.h"


bool CArea::CoordsIsInArea(const Vector3c& coords) {
	return m_Bound.CoordsIsIn(coords);
}

void CArea::AddDrawable(uint32 uniqueHash, const SkyString& filename, uint8* pData, uint32 size)
{
	m_Files.LockStore();

	// supprime si existe déjà
	for (auto i = m_Files.begin(); i != m_Files.end(); i++) {
		if (std::get<3>(*i) == uniqueHash)
		{
			delete[] std::get<1>(*i);
			m_Files.erase(i);
			break;
		}
	}

	m_Files.push_back(std::make_tuple(filename, pData, size, uniqueHash));
	m_Files.UnlockStore();
}

void CArea::AddEntityDef(const EntityDef& entDef, const ArchetypeDef& arch, bool lock)
{
	if (lock) m_Entities.LockStore();
	m_Entities.push_back(std::make_tuple(entDef, arch));
	if (lock) m_Entities.UnlockStore();
}

void CArea::AddArchetypeDef(const ArchetypeDef& arch)
{
	m_Archetypes.LockStore();

	// supprime si existe déjà
	for (auto i = m_Archetypes.begin(); i != m_Archetypes.end(); i++) {
		if (i->m_DrawableHash == arch.m_DrawableHash) {
			m_Archetypes.erase(i);
			break;
		}
	}

	m_Archetypes.push_back(arch);
	m_Archetypes.UnlockStore();
}

void CArea::AddGrassBatch(const GrassBatchDef& grass)
{
	m_GrassBatches.LockStore();
	m_GrassBatches.push_back(grass);
	m_GrassBatches.UnlockStore();
}


void CArea::AddYTYP(const SkyString& ytyp, bool lock)
{
	if (lock) m_YTYPdeps.LockStore();
	bool add = true;
	for (const auto& i : m_YTYPdeps) {
		if (ytyp == i) {
			add = false;
			break;
		}
	}
	if (add)
		m_YTYPdeps.push_back(ytyp);
	if (lock) m_YTYPdeps.UnlockStore();
}


void CArea::CalculateBounds()
{
	m_Entities.LockStore();
	{
		m_EntitiesBounds.Min = GetMaxVec3();
		m_EntitiesBounds.Max = GetMinVec3();

		m_StreamBounds.Min = GetMaxVec3();
		m_StreamBounds.Max = GetMinVec3();

		for (const auto& i : m_Entities)
		{
			const auto& iEnt = std::get<0>(i);
			const auto& iArch = std::get<1>(i);

			Vector3c iBBMin = iArch.m_AABB.Min * iEnt.m_Scale;
			Vector3c iBBMax = iArch.m_AABB.Max * iEnt.m_Scale;

			Vector3c eBoxBases[8] =
			{
				iBBMin,
				iBBMax,
				{ iBBMin.x, iBBMin.y, iBBMax.z },
				{ iBBMin.x, iBBMax.y, iBBMin.z },
				{ iBBMin.x, iBBMax.y, iBBMax.z },
				{ iBBMax.x, iBBMin.y, iBBMin.z },
				{ iBBMax.x, iBBMin.y, iBBMax.z },
				{ iBBMax.x, iBBMax.y, iBBMin.z },
			};

			iBBMin = (iArch.m_AABB.Min * iEnt.m_Scale) - iEnt.m_LODdist;
			iBBMax = (iArch.m_AABB.Max * iEnt.m_Scale) + iEnt.m_LODdist;

			Vector3c sBoxBases[8] =
			{
				iBBMin,
				iBBMax,
				{ iBBMin.x, iBBMin.y, iBBMax.z },
				{ iBBMin.x, iBBMax.y, iBBMin.z },
				{ iBBMin.x, iBBMax.y, iBBMax.z },
				{ iBBMax.x, iBBMin.y, iBBMin.z },
				{ iBBMax.x, iBBMin.y, iBBMax.z },
				{ iBBMax.x, iBBMax.y, iBBMin.z },
			};

			Vector3c ebMin = GetMaxVec3();
			Vector3c ebMax = GetMinVec3();
			Vector3c sbMin = GetMaxVec3();
			Vector3c sbMax = GetMinVec3();

			// calcul les coordonnées max et min pour chaque sommet de la boite
			// de l'entité en appliquant la rotation de celle-ci
			for (int x = 0; x < 8; x++)
			{
				Vector3c eBoxRot = iEnt.m_Rot.Product(eBoxBases[x]) + iEnt.m_Pos;
				ebMin = ebMin.Min(eBoxRot);
				ebMax = ebMax.Max(eBoxRot);

				Vector3c sBoxRot = iEnt.m_Rot.Product(sBoxBases[x]) + iEnt.m_Pos;
				sbMin = sbMin.Min(sBoxRot);
				sbMax = sbMax.Max(sBoxRot);
			}

			m_EntitiesBounds.Min = m_EntitiesBounds.Min.Min(ebMin);
			m_EntitiesBounds.Max = m_EntitiesBounds.Max.Max(ebMax);

			m_StreamBounds.Min = m_StreamBounds.Min.Min(sbMin);
			m_StreamBounds.Max = m_StreamBounds.Max.Max(sbMax);
		}
	}
	m_Entities.UnlockStore();
}



void CArea::CalculateProcBounds()
{
	m_GrassBatches.LockStore();
	{
		m_ProcEntitiesBounds.Min = GetMaxVec3();
		m_ProcEntitiesBounds.Max = GetMinVec3();

		m_ProcStreamBounds.Min = GetMaxVec3();
		m_ProcStreamBounds.Max = GetMinVec3();

		for (const auto& i : m_GrassBatches)
		{
			Vector3c iBBMin = i.m_AABB.Min;
			Vector3c iBBMax = i.m_AABB.Max;

			Vector3c eBoxBases[8] =
			{
				iBBMin,
				iBBMax,
				{ iBBMin.x, iBBMin.y, iBBMax.z },
				{ iBBMin.x, iBBMax.y, iBBMin.z },
				{ iBBMin.x, iBBMax.y, iBBMax.z },
				{ iBBMax.x, iBBMin.y, iBBMin.z },
				{ iBBMax.x, iBBMin.y, iBBMax.z },
				{ iBBMax.x, iBBMax.y, iBBMin.z },
			};

			iBBMin = i.m_AABB.Min - i.m_LODdist;
			iBBMax = i.m_AABB.Max + i.m_LODdist;

			Vector3c sBoxBases[8] =
			{
				iBBMin,
				iBBMax,
				{ iBBMin.x, iBBMin.y, iBBMax.z },
				{ iBBMin.x, iBBMax.y, iBBMin.z },
				{ iBBMin.x, iBBMax.y, iBBMax.z },
				{ iBBMax.x, iBBMin.y, iBBMin.z },
				{ iBBMax.x, iBBMin.y, iBBMax.z },
				{ iBBMax.x, iBBMax.y, iBBMin.z },
			};

			Vector3c ebMin = GetMaxVec3();
			Vector3c ebMax = GetMinVec3();
			Vector3c sbMin = GetMaxVec3();
			Vector3c sbMax = GetMinVec3();

			for (int x = 0; x < 8; x++)
			{
				ebMin = ebMin.Min(eBoxBases[x]);
				ebMax = ebMax.Max(eBoxBases[x]);

				sbMin = sbMin.Min(sBoxBases[x]);
				sbMax = sbMax.Max(sBoxBases[x]);
			}

			m_ProcEntitiesBounds.Min = m_ProcEntitiesBounds.Min.Min(ebMin);
			m_ProcEntitiesBounds.Max = m_ProcEntitiesBounds.Max.Max(ebMax);

			m_ProcStreamBounds.Min = m_ProcStreamBounds.Min.Min(sbMin);
			m_ProcStreamBounds.Max = m_ProcStreamBounds.Max.Max(sbMax);
		}
	}
	m_GrassBatches.UnlockStore();
}
