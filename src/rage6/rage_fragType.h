#pragma once
#include "rage_fwrap.h"
#include "rmcDrawable_rdr.h"

namespace rdr
{
	namespace rage
	{
		const uint32 FRAGDRAWABLE_VTABLES[] = {
			// frag drawable
			0xA4407000, 0x0C927000, 0x8CA27000, 0x34B57000, 0x04B67000, 0x84A47000,
			0x44917000,
			// 0x54B97000,
			0x5CB97000,
			0x84B97000, 0xECB97000, 0xECA27000,

			// base drawable
			0xE0147200, 0x68EE7100, 0x60187200, 0x58EE7100,
			0x40FF7100, 0x48127200, 0xC8017200, 0x38187200,
			0x48FF7100, 0x10027200, 0xF8127200, 0xC0017200,
			0xC8157200, 0x38187200, 0xD0287200, 0x88FF7100,
			0xC0EE7100
		};
		static uint32 g_FragDrawablesVTables[32]{ 0 };

		const char PATTERN_NAME[] = "/rdr2/";

		static int LoadFragDrawable(Drawables& drawablesOut, const char* filename,
			char* sys, int sysSize,
			char* gfx, int gfxSize,
			bool onlyTexDict = false)
		{
			// search for xft name (hash the name)
			uint32 DRAWABLE_HASH = 0;
			const int PAT_LEN = sizeof(PATTERN_NAME) - 1;
			char* sysA = sys;
			for (int i = 0; i < sysSize - PAT_LEN; i++, sysA++) {
				bool ok = true;
				for (int y = 0; y < PAT_LEN; y++) {
					if (sysA[y] != PATTERN_NAME[y]) {
						ok = false;
						break;
					}
				}
				if (ok)
				{
					int strLen = strlen(sysA);
					for (int t = strLen;; t--) {
						if (sysA[t] == '/') {
							DRAWABLE_HASH = JoaatStr(&sysA[t + 1]);
							break;
						}
					}
					break;
				}
			}



			// search for drawable vtables
			atSingleton<CShaderHashMap>::GetRef().Init();

			if (g_FragDrawablesVTables[0] == 0)
			{
				for (int i = 0; i < ARRAYSIZE(FRAGDRAWABLE_VTABLES); i++)
					g_FragDrawablesVTables[i] = InvEnd(FRAGDRAWABLE_VTABLES[i]);
			}


			try
			{
				rmcDrawable* iDrawable = 0;
				for (int64 i = 0; i < sysSize - 4; i += 4)
				{
					uint32 magic_vt = *(uint32*)(sys + i);

					bool found = false;
					for (int i = 0; i < ARRAYSIZE(FRAGDRAWABLE_VTABLES); i++) {
						if (magic_vt == g_FragDrawablesVTables[i]) {
							found = true;
							break;
						}
					}

					if (found)
					{
						iDrawable = (rmcDrawable*)(sys + i);
						ParseDrawable(drawablesOut, iDrawable, DRAWABLE_HASH, sys, sysSize, gfx, gfxSize);
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