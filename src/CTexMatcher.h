#pragma once
#include "SkyCommons/SkyCommons.h"


class CTexMatcher
{
public:
	SkyString m_Texname;
	uint32 m_TexHash;

public:
	CTexMatcher(const SkyString& texname, uint32 texHash) : m_Texname(texname), m_TexHash(texHash) {}

	bool operator==(const CTexMatcher& ref) const {
		return (ref.m_TexHash == m_TexHash) || (ref.m_Texname == m_Texname);
	}
};
