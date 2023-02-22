#pragma once
#include "SkyCommons/SkyContainer.h"
#include "SkyCommons/SkyAtom.h"
#include "grcTexture_rdr.h"
#include "CTexMatcher.h"

using namespace rdr::rage;


struct TexDefinition {
	SkyString m_TexName;
	uint32 m_TexHash;
	uint16 m_Width;
	uint16 m_Height;
};

class CTexDictDef : public CSecure
{
public:
	SkyString m_Path;
	// SkyVector<uint32> m_TexHashes;
	SkyVector<TexDefinition> m_Texs;
	SkyVector<SkyString> m_DrawablesReferences; // liste des drawables utilisant le dico
	bool m_DictIsADrawable;

	SkySharedPtr<TXD> m_pOpenedTxd = 0;
	uint64 m_LastTimeUsed = 0; // GetTickCount64()

public:
	bool IsTexInThisDictionnary(const CTexMatcher& matcher) const;
	bool IsTexInThisDictionnary(const SkyString& texname) const;
	bool IsTexInThisDictionnary(uint32 hash) const;

	const TexDefinition* GetTexInThisDictionnary(const CTexMatcher& matcher) const;
	void GetTexsInThisDictionnary(SkyVector<TexDefinition*>& matches, const CTexMatcher& matcher);

	void AddRef(const SkyString& drawable);

	SkySharedPtr<TXD> GetOrOpenTexDict();
};


class CTextureLinker : public CSecure
{
public:
	SkyVector<CTexDictDef*> m_Dictionnaries;

public:
	void LoadFromFile(const char* path, bool dictDwd);

	CTexDictDef* FindTextureByMatcher(const CTexMatcher& matcher, DetailLevel level, uint16* width = 0, uint16* height = 0);
	CTexDictDef* FindTextureByName(const SkyString& texname);
	CTexDictDef* FindTextureByHash(uint32 hash);

	void ClearWorkCache();

	void DoWork();
};
