#pragma once


namespace rdr
{
	namespace rage
	{
		static const char* g_ShadersList[] =
		{
			"rage_curvedmodel",
			"rage_lightprepass",
			"rdr2_alpha",
			"rdr2_alpha_bspec_ao",
			"rdr2_alpha_bspec_ao_cloth",
			"rdr2_alpha_bspec_ao_cpv",
			"rdr2_alpha_bspec_ao_shared",
			"rdr2_alpha_bspec_ao_shareduv",
			"rdr2_alpha_bspec_ao_shareduv_character",
			"rdr2_alpha_bspec_ao_shareduv_character_cutscene",
			"rdr2_alpha_bspec_ao_shareduv_character_hair",
			"rdr2_alpha_cloth",
			"rdr2_alpha_foliage",
			"rdr2_alpha_foliage_no_fade",
			"rdr2_alpha_hair_a",
			"rdr2_atmoscatt",
			"rdr2_beacon",
			"rdr2_billboard",
			"rdr2_binoculars_barrel",
			"rdr2_binoculars_lens",
			"rdr2_breakableglass",
			"rdr2_bump",
			"rdr2_bump_ambocc",
			"rdr2_bump_ambocc_alphaao",
			"rdr2_bump_ambocc_cpv",
			"rdr2_bump_cloth_ambocc_shareduv_character",
			"rdr2_bump_spec",
			"rdr2_bump_spec_alpha",
			"rdr2_bump_spec_ambocc_cpv",
			"rdr2_bump_spec_ambocc_reflection_shared",
			"rdr2_bump_spec_ambocc_shared",
			"rdr2_bump_spec_ambocc_shareduv",
			"rdr2_bump_spec_ambocc_shareduv_character",
			"rdr2_bump_spec_ambocc_shareduv_character_2sided",
			"rdr2_bump_spec_ambocc_shareduv_character_cloth",
			"rdr2_bump_spec_ambocc_shareduv_character_cutscene",
			"rdr2_bump_spec_ambocc_shareduv_character_skin",
			"rdr2_bump_spec_ambocc_smooth_shared",
			"rdr2_bump_spec_ao_cloth",
			"rdr2_bump_spec_ao_dirt_cloth",
			"rdr2_cati",
			"rdr2_character_cloth_2sided",
			"rdr2_character_glow",
			"rdr2_character_glow_skin",
			"rdr2_character_lod",
			"rdr2_cliffwall",
			"rdr2_cliffwall_alpha",
			"rdr2_cliffwall_alpha2",
			"rdr2_cliffwall_ao",
			"rdr2_cliffwall_ao_low_lod",
			"rdr2_clouds_anim",
			"rdr2_clouds_animsoft",
			"rdr2_clouds_fast",
			"rdr2_clouds_fog",
			"rdr2_clouds_soft",
			"rdr2_debris",
			"rdr2_diffuse",
			"rdr2_diffuse_cloth",
			"rdr2_dome_clouds",
			"rdr2_door_glow",
			"rdr2_flattenterrain",
			"rdr2_flattenterrain_blend",
			"rdr2_footprint",
			"rdr2_fur",
			"rdr2_glass_decal",
			"rdr2_glass_glow",
			"rdr2_glass_nodistortion_bump_spec_ao",
			"rdr2_glass_nodistortion_bump_spec_ao_shared",
			"rdr2_glass_notint",
			"rdr2_glass_notint_nodistortion",
			"rdr2_glass_notint_shared",
			"rdr2_glow",
			"rdr2_gpurain_render",
			"rdr2_graffiti",
			"rdr2_grass",
			"rdr2_gravestone_text",
			"rdr2_horsehair",
			"rdr2_im",
			"rdr2_injury",
			"rdr2_layer_2_nospec_ambocc",
			"rdr2_layer_2_nospec_ambocc_bridge",
			"rdr2_layer_2_nospec_ambocc_decal",
			"rdr2_layer_3_nospec_normal_ambocc",
			"rdr2_lightcone",
			"rdr2_lightglow",
			"rdr2_litdecal",
			"rdr2_lod4_water",
			"rdr2_low_lod",
			"rdr2_low_lod_decal",
			"rdr2_low_lod_nodirt",
			"rdr2_low_lod_nodirt_singlesided",
			"rdr2_low_lod_singlesided",
			"rdr2_map",
			"rdr2_mirror",
			"rdr2_optics",
			"rdr2_pond_water",
			"rdr2_poster",
			"rdr2_postfx",
			"rdr2_pppelements",
			"rdr2_river_water",
			"rdr2_river_water_joint",
			"rdr2_rmptfx_lit",
			"rdr2_scope_barrel",
			"rdr2_scope_lens",
			"rdr2_scope_lens_distortion",
			"rdr2_shadowonly",
			"rdr2_shareduv_hair",
			"rdr2_taa",
			"rdr2_terrain",
			"rdr2_terrain4",
			"rdr2_terrain_blend",
			"rdr2_terrain_low_lod",
			"rdr2_terrain_shoreline",
			"rdr2_traintrack",
			"rdr2_traintrack_low_lod",
			"rdr2_treerock_prototype",
			"rdr2_waterdecal",
			"rdr2_waterjointdecal",
			"rdr2_weapon",
			"rdr2_wheeltrack",
			"rdr2_window_glow",
			"rdr2_wire",
			"rdr2_worldmovie",
		};


		class CShaderHashMap
		{
		public:
			uint32 m_ShadersListMap[ARRAYSIZE(g_ShadersList)];

		public:
			void Init()
			{
				if (m_ShadersListMap[0] == 0) {
					for (int i = 0; i < ARRAYSIZE(g_ShadersList); i++) {
						m_ShadersListMap[i] = JOAAT_Str(g_ShadersList[i], true);
					}
				}
			}

			const char* operator[](uint32 hash)
			{
				const char* out = "NOT_RESOLVED";
				for (int i = 0; i < ARRAYSIZE(g_ShadersList); i++) {
					if (m_ShadersListMap[i] == hash) {
						out = g_ShadersList[i];
						break;
					}
				}
				return out;
			}
		};
	}
}