#pragma once
#include "SkyCommons/SkyString.h"
#include "SkyCommons/SkyMath.h"

#define MAP_SCALE 2.f

// Center Pos of map RDR (scale) :   4795.25f, -4490.16f, 0.05f
const Vector3c MAP_OFFSET = { -6047.f, 5646.f, 50.f };

const float LOD_DISTANCE = 1200.f;
const float PROPS_LOD_DISTANCE = 300.f;
const float GRASS_LOD_DIST = 800.f;

const float SizeAreas = 400.f; // défini la taille des zones pour les YMAP génerés

#define CONV_DRAWABLE_THREAD_COUNT 32
#define BUILD_MAP_THREAD_COUNT 32
#define SUBPACK_COUNT 8

const wchar_t OUT_DIRECTORY[] = L"C:\\Program Files\\Rockstar Games\\Grand Theft Auto V\\mods\\update\\x64\\dlcpacks\\lasventuras\\";
const SkyString SRC_DIRECTORY = "C:\\RDR1_to_V\\";

// Durée de vie des textures en RAM (après ça, la texture est déchargé
// jusqu'à ce qu'elle sois de nouveau nécessaire et donc recharger si besoin)
#define TEXTURE_MEMORY_LIFESPAN 60 * 2 * 1000//3600 * 1000

