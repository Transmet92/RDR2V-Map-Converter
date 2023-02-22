#include "CTextureLinker.h"
#include "IOFile.h"
#include "rage_fwrap.h"
#include "rmcDrawable_rdr.h"
#include "ConversionsConstParameters.h"


void CTextureLinker::ClearWorkCache()
{
	for (auto i : m_Dictionnaries)
	{
		i->Lock();

		if (i->m_pOpenedTxd)
			i->m_pOpenedTxd = 0;

		i->Unlock();
	}
}

void CTextureLinker::DoWork()
{
	return;

	// Garbage collecting opened textures
	for (auto i : m_Dictionnaries)
	{
		i->Lock();
		if (i->m_pOpenedTxd)
		{
			if ((GetTickCount64() - i->m_LastTimeUsed) > TEXTURE_MEMORY_LIFESPAN)
				i->m_pOpenedTxd = 0;
		}
		i->Unlock();
	}
}



SkySharedPtr<TXD> CTexDictDef::GetOrOpenTexDict()
{
	SkySharedPtr<TXD> txdOut = 0;

	Lock();
	{
		if (m_pOpenedTxd)
		{
			m_LastTimeUsed = GetTickCount64();
			txdOut = m_pOpenedTxd;
		}
		else
		{
			char gpuFilepath[MAX_PATH + 128];
			lstrcpyA(gpuFilepath, m_Path.c_str());
			lstrcpyA(strstr(gpuFilepath, ".cpu"), ".gpu");

			std::shared_ptr<char[]> sysBuff;
			std::shared_ptr<char[]> gfxBuff;
			uint32 sysSize = 0;
			uint32 gfxSize = 0;
			bool sysLoaded = LoadFile(m_Path.c_str(), sysSize, sysBuff);
			bool gfxLoaded = LoadFile(gpuFilepath, gfxSize, gfxBuff);

			if (sysLoaded)
			{
				txdOut = SkySharedPtr<TXD>(new TXD());

				// CHARGE LE DICTIONNAIRE DE TEXTURE A PARTIR D'UN XTD
				if (!m_DictIsADrawable) {
					*txdOut = rdr::rage::LoadTextureDictionnary(m_Path.c_str(), sysBuff.get(), sysSize, gfxBuff.get(), gfxSize);
				}

				// CHARGE LE DICTIONNAIRE DE TEXTURE DU DRAWABLE
				else
				{
					rdr::rage::Drawables drawables;
					if (rdr::rage::LoadDrawable(drawables, m_Path.c_str(), sysBuff.get(), sysSize, gfxBuff.get(), gfxSize, true))
						*txdOut = drawables.m_EmbedTex;
				}

				m_pOpenedTxd = txdOut;
				m_LastTimeUsed = GetTickCount64();
			}
		}
	}
	Unlock();

	return txdOut;
}




void CTexDictDef::GetTexsInThisDictionnary(SkyVector<TexDefinition*>& matches, const CTexMatcher& matcher)
{
	for (const auto& i : m_Texs) {
		if (matcher.m_TexHash == i.m_TexHash ||
			matcher.m_Texname == i.m_TexName) {
			matches.push_back((TexDefinition*)&i);
		}
	}
}

const TexDefinition* CTexDictDef::GetTexInThisDictionnary(const CTexMatcher& matcher) const
{
	for (const auto& i : m_Texs) {
		if (matcher.m_TexHash == i.m_TexHash ||
			matcher.m_Texname == i.m_TexName) {
			return &i;
		}
	}

	return 0;
}


bool CTexDictDef::IsTexInThisDictionnary(const CTexMatcher& matcher) const
{
	for (const auto& i : m_Texs) {
		if (matcher.m_TexHash == i.m_TexHash ||
			matcher.m_Texname == i.m_TexName) {
			return true;
		}
	}

	return false;
}

bool CTexDictDef::IsTexInThisDictionnary(const SkyString& texname) const
{
	for (const auto& i : m_Texs) {
		if (texname == i.m_TexName)
			return true;
	}
	return false;
}

bool CTexDictDef::IsTexInThisDictionnary(uint32 hash) const
{
	for (const auto& i : m_Texs) {
		if (hash == i.m_TexHash)
			return true;
	}
	return false;
}


void CTexDictDef::AddRef(const SkyString& drawable) {
	m_DrawablesReferences.push_back(drawable);
}




void CTextureLinker::LoadFromFile(const char* path, bool dictDwd)
{
	std::shared_ptr<char[]> listBuff;
	uint32 listSize = 0;
	bool sysLoaded = LoadFile(path, listSize, listBuff);
	if (sysLoaded)
	{
		char* pList = listBuff.get();
		char* endList = pList + listSize - 4;
		char* currentLine = pList;

		bool inDictCreationStep = true;
		CTexDictDef* pNewTexDict = new CTexDictDef();

		for (; pList < endList;)
		{
			if (pList[0] == 0x0D && pList[1] == 0x0A)
			{
				char* pRestore = &pList[0];
				*pRestore = 0x00;
				pList += 2;

				int iLineSize = (pList - 2) - currentLine;
				if (iLineSize == 0)
				{	
					m_Dictionnaries.push_back(pNewTexDict);
					pNewTexDict = new CTexDictDef();
					pNewTexDict->m_DictIsADrawable = dictDwd;
					inDictCreationStep = true;
				}
				else
				{
					if (inDictCreationStep)
					{
						pNewTexDict->m_Path = currentLine;
						inDictCreationStep = false;
					}
					else
					{
						auto& splits = Split8(currentLine, ':');
						
						uint16 width = atoi(splits[1].c_str());
						uint16 height = atoi((char*)strstr(splits[1].c_str(), "x") + 1);

						pNewTexDict->m_Texs.push_back({ splits[2], InvEnd(strtoul(currentLine, 0, 16)), width, height });
					}
				}

				*pRestore = 0x0D;
				currentLine = pList;
			}
			else
				pList++;
		}
	}
}



CTexDictDef* CTextureLinker::FindTextureByMatcher(const CTexMatcher& matcher, DetailLevel level, uint16* width, uint16* height)
{
	SkyVector<SkyTuple<CTexDictDef*, uint32, uint16, uint16>> candidats; // tuple:  texDict, resolution

	Lock();
	for (const auto i : m_Dictionnaries)
	{
		SkyVector<TexDefinition*> matches;
		i->GetTexsInThisDictionnary(matches, matcher);

		for (auto t : matches)
			candidats.push_back(std::make_tuple(i, t->m_Width * t->m_Height, t->m_Width, t->m_Height));
	}
	Unlock();


	uint32 candidatsCount = candidats.size();
	if (candidatsCount)
	{
		std::sort(candidats.begin(), candidats.end(),
			[](const SkyTuple<CTexDictDef*, uint32, uint16, uint16>& a,
			const SkyTuple<CTexDictDef*, uint32, uint16, uint16>& b) -> bool
		{
			return std::get<1>(a) > std::get<1>(b);
		});

		if (level == DetailLevel::DRAWABLE_HD)
		{
			if (width)
				*width = std::get<2>(candidats.front());
			if (height)
				*height = std::get<3>(candidats.front());

			return std::get<0>(candidats.front());
		}
		else
		{
			if (width)
				*width = std::get<2>(candidats.back());
			if (height)
				*height = std::get<3>(candidats.back());

			return std::get<0>(candidats.back());
		}
	}

	return 0;
}

CTexDictDef* CTextureLinker::FindTextureByName(const SkyString& texname)
{
	CTexDictDef* out = 0;
	Lock();
	for (const auto i : m_Dictionnaries)
	{
		if (i->IsTexInThisDictionnary(texname))
			out = i;
	}
	Unlock();
	return out;
}

CTexDictDef* CTextureLinker::FindTextureByHash(uint32 hash)
{
	CTexDictDef* out = 0;
	Lock();
	for (const auto i : m_Dictionnaries)
	{
		if (i->IsTexInThisDictionnary(hash))
			out = i;
	}
	Unlock();
	return out;
}
