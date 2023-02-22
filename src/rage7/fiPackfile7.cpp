#define _CRT_SECURE_NO_WARNINGS
#include "fiPackfile7.h"
#include <algorithm>
#include <memory>
#include <tuple>
#include <vector>

#include "SkyCommons/SkyString.h"


namespace rage
{
	uint64 CPackageOpQueue::GetApproxBytesCount()
	{
		uint64 sum = 0;
		for (const auto& i : m_Operations) {
			if (i.Type == eOT_ADD_FILE)
				sum += i.fileSize;
		}
		return sum;
	}

	void CPackageOpQueue::AddFile(const char* targetPathInRPF, const uint8* bin, int size, uint32 uncompressed)
	{
		OperationRpf op = {};
		op.Type = eOT_ADD_FILE;
		op.path = targetPathInRPF;
		op.fileBuf = (uint8*)bin;
		op.fileSize = size;
		op.uncompressed = uncompressed;
		m_Operations.push_back(op);
	}

	void CPackageOpQueue::AddDirectory(const char* pathInRPF)
	{
		OperationRpf op = {};
		op.Type = eOT_ADD_DIRECTORY;
		op.path = pathInRPF;
		m_Operations.push_back(op);
	}

	void CPackageOpQueue::Remove(const char* pathInRPF)
	{
		OperationRpf op = {};
		op.Type = eOT_REMOVE;
		op.path = pathInRPF;
		m_Operations.push_back(op);
	}
}


namespace rage7
{
	Entry::~Entry() {
		if (pEntry) {
			if (isFolder)
				delete ((sFolder*)pEntry);
			else
				delete ((sFile*)pEntry);
		}
	}

	sFolder::~sFolder() {
		for (auto i : entries) {
			delete i;
		}
	}


	void sFolder::sort() {
		std::sort(entries.begin(), entries.end(), [](const Entry* a, const Entry* b) -> bool {
			return a->name < b->name;
		});
	}


	bool CPackageBuilder7::BuildMap(std::vector<rage::OperationRpf> operations)
	{
		m_Root.name = "";
		m_Root.isFolder = true;
		m_Root.pEntry = (void*)new sFolder();


		uint64 totalHeadSz = 16;
		uint64 filesDataSz = 0;

		for (auto& i : operations)
		{
			if (i.Type == rage::OpType::eOT_ADD_DIRECTORY ||
				i.Type == rage::OpType::eOT_ADD_FILE)
			{
				Entry* parentDir = &m_Root;

				const auto& pathParts = SplitPath(i.path);
				if (pathParts.size() > 1)
				{
					for (auto it = pathParts.begin(); it != (pathParts.end() - 1); it++)
					{
						Entry* match = 0;

						sFolder* thisFolder = (sFolder*)parentDir->pEntry;
						for (const auto& x : thisFolder->entries)
						{
							if (x->name == *it)
							{
								match = x;
								break;
							}
						}

						if (match)
							parentDir = match;
						else
						{
							Entry* newFolder = new Entry{};
							newFolder->isFolder = true;
							newFolder->name = *it;
							newFolder->pEntry = (void*)new sFolder();
							thisFolder->entries.push_back(newFolder);
							parentDir = newFolder;

							m_EntriesCount++;
						}
					}
				}

				const auto& entryName = pathParts.back();
				totalHeadSz += 16;
				totalHeadSz += entryName.size() + 1;

				m_EntriesCount++;

				if (i.Type == rage::OpType::eOT_ADD_DIRECTORY)
				{
					sFolder* parentFolder = (sFolder*)parentDir->pEntry;
					Entry* newFolder = new Entry{};
					newFolder->isFolder = true;
					newFolder->name = entryName;
					newFolder->pEntry = (void*)new sFolder();
					parentFolder->entries.push_back(newFolder);
				}
				else if (i.Type == rage::OpType::eOT_ADD_FILE)
				{
					uint64 fszAlign = i.fileSize;
					fszAlign += (512 - (fszAlign % 512));
					filesDataSz += fszAlign;

					sFolder* parentFolder = (sFolder*)parentDir->pEntry;
					Entry* newFile = new Entry{};
					newFile->isFolder = false;
					newFile->name = entryName;
					sFile* thisFile = new sFile();
					newFile->pEntry = (void*)thisFile;
					parentFolder->entries.push_back(newFile);

					thisFile->data = i.fileBuf;
					thisFile->size = i.fileSize;
					thisFile->uncompressed = i.uncompressed;
				}
			}
		}

		totalHeadSz += (512 - (totalHeadSz % 512));
		m_Size = totalHeadSz + filesDataSz;
		m_DataOffset = totalHeadSz;

		return true;
	}


	void CPackageBuilder7::WriteAt(void* dest)
	{
		// trie les entrées par noms (nécessité algorithmique du jeu)
		struct {
			Entry* pEntry;
			uint64 iterator;
		} stack[64];

		stack[0].iterator = 0;
		stack[0].pEntry = &m_Root;
		((sFolder*)m_Root.pEntry)->sort();

		for (int i = 0;;)
		{
			if (i == -1)
				break;

			auto& it = stack[i];
			sFolder* itF = (sFolder*)it.pEntry->pEntry;

			if (it.iterator != itF->entries.size())
			{
				auto newEntry = itF->entries[it.iterator];
				if (newEntry->isFolder)
				{
					i++;
					stack[i].iterator = 0;
					stack[i].pEntry = newEntry;
					((sFolder*)newEntry->pEntry)->sort();
				}
				it.iterator++;
			}
			else
				i--;
		}

		m_EntriesCount++;

		rage7::fwRpfHead* pHead = (rage7::fwRpfHead*)dest;
		pHead->m_Magic = (uint32)'RPF7';
		pHead->m_EncryptionFlag = (uint32)'NEPO';
		pHead->m_Entries = m_EntriesCount;

		char* strSection = ((char*)dest + sizeof(rage7::fwRpfHead) + sizeof(fwRpfEntry) * m_EntriesCount);
		char* iString = strSection;

		fwRpfEntry* pFwEntrySection = (fwRpfEntry*)((uint8*)dest + sizeof(rage7::fwRpfHead));
		fwRpfEntry* pFwEntry = pFwEntrySection;

		sFolder* itF = (sFolder*)m_Root.pEntry;
		pFwEntry->entry.dir.m_NameOffset = 0;
		pFwEntry->entry.dir.m_EntryType = 0x7FFFFF00;
		pFwEntry->entry.dir.m_EntriesCount = itF->entries.size();

		iString[0] = 0x00;
		iString++;

		uint8* pFileData = ((uint8*)dest + m_DataOffset);

		std::vector<std::tuple<Entry*, fwRpfEntry*>> WorkStack;
		WorkStack.push_back(std::make_tuple(&m_Root, pFwEntry++));

		for (; WorkStack.size();)
		{
			auto& i = WorkStack.back();
			WorkStack.pop_back();

			sFolder* itF = (sFolder*)(std::get<0>(i)->pEntry);
			fwRpfEntry* ipEntry = std::get<1>(i);

			ipEntry->entry.dir.m_EntryIndex = ((uint32)((uint8*)pFwEntry - (uint8*)pFwEntrySection)) / sizeof(fwRpfEntry);

			for (auto x : itF->entries)
			{
				strcpy(iString, x->name.c_str());
				pFwEntry->entry.file.m_NameOffset = (uint16)(iString - strSection);
				iString += x->name.size() + 1;

				if (x->isFolder)
				{
					pFwEntry->entry.dir.m_EntryType = 0x7FFFFF00;
					pFwEntry->entry.dir.m_EntriesCount = ((sFolder*)x->pEntry)->entries.size();
					WorkStack.push_back(std::make_tuple(x, pFwEntry));
				}
				else
				{
					bool isRsc = false;
					sFile* iFile = (sFile*)x->pEntry;

					uint32 iFileOffset = (uint32)(pFileData - (uint8*)dest) / 512;
					memcpy(pFileData, iFile->data, iFile->size);
					pFileData += iFile->size + (512 - (iFile->size % 512));

					if (iFile->size >= sizeof(fwRSC7Head))
					{
						if (*(uint32*)iFile->data == '7CSR') // RSC7
						{
							auto rscHead = (fwRSC7Head*)iFile->data;
							pFwEntry->entry.rsc.m_GfxFlags = rscHead->m_GfxFlags;
							pFwEntry->entry.rsc.m_SysFlags = rscHead->m_SysFlags;
							pFwEntry->entry.rsc.m_FileSize = (iFile->size > 0xFFFFFF ? 0xFFFFFF : iFile->size);
							pFwEntry->entry.rsc.m_FileOffset = iFileOffset;
							pFwEntry->entry.rsc.m_FileOffset[2] |= 0x80;
							isRsc = true;
						}
					}

					// is binary file
					if (!isRsc)
					{
						pFwEntry->entry.file.m_FileOffset = iFileOffset;
						pFwEntry->entry.file.m_IsEncrypted = 0;

						// is compressed
						if (iFile->uncompressed) {
							pFwEntry->entry.file.m_FileSize = iFile->size;
							pFwEntry->entry.file.m_UncompressedSize = iFile->uncompressed;
						}
						else {
							pFwEntry->entry.file.m_FileSize = 0;
							pFwEntry->entry.file.m_UncompressedSize = iFile->size;
						}
					}
				}

				pFwEntry++;
			}

		}

		pHead->m_NamesSize = (uint32)(iString - strSection);
		pHead->m_NamesSize += (16 - (pHead->m_NamesSize % 16));
	}
}