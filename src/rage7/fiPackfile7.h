#pragma once
#include "SkyCommons/types.h"
#include <windows.h>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include "zlib.h"
#include "CMapFile.h"


namespace rage
{
	enum OpType {
		eOT_ADD_FILE, eOT_ADD_DIRECTORY, eOT_REMOVE
	};
	struct OperationRpf
	{
		OpType Type;
		std::string path;

		uint8* fileBuf;
		uint32 fileSize;
		uint32 uncompressed = 0; // only if file bin is compressed
	};

	class CPackageOpQueue
	{
	protected:
		std::vector<OperationRpf> m_Operations;

	public:
		std::vector<OperationRpf> GetQueue() const { return m_Operations; };

		uint64 GetApproxBytesCount();

		void AddFile(const char* targetPathInRPF, const uint8* bin, int size, uint32 uncompressed = 0);
		void AddDirectory(const char* pathInRPF);
		void Remove(const char* pathInRPF); // fichier ou dossier
	};
}




class CRagePackFile;
struct ContainerInfo;

struct EntryInfo {
	std::string m_Name;
	CRagePackFile* m_ParentPack;
	ContainerInfo* m_Parent;
};

// can not be a RPF
struct FileInfo : public EntryInfo {
	uint32 m_FileSize;
	uint32 m_FileOffset;
	uint32 m_UncompressedFileSize;
	bool m_IsRsc;
	uint8 m_RscVersion;
	bool m_IsRPF;
	bool m_IsEncrypted;

	uint32 GetRealFileSize() const {
		return (m_UncompressedFileSize ? m_UncompressedFileSize : m_FileSize);
	}

	bool IsCompressed() const {
		return m_UncompressedFileSize && m_FileSize;
	}

	bool IsEncrypted() const {
		return m_IsEncrypted;
	}
};

// Directory only
struct ContainerInfo : public EntryInfo
{
	// CRagePackFile* m_RPF; // 0 if is a directory

	// childrens
	std::vector<FileInfo*> m_Files; // rsc or file list
	std::vector<ContainerInfo*> m_Containers;

	void Release()
	{
		for (const auto i : m_Files)
			delete i;

		for (const auto i : m_Containers) {
			i->Release();
			delete i;
		}
	}
};




class CRagePackFile : public rage::CPackageOpQueue
{
protected:
	std::wstring m_RpfPath;
	std::string m_RpfName;

public:
	virtual bool Open(uint8* pRPF, uint64 size, std::string rpfName) = 0;
	virtual bool Open(const wchar_t* rootRpfPath) = 0;

	virtual ContainerInfo* GetRoot() = 0;
	virtual std::wstring GetRpfPath() const { return m_RpfPath; }
	virtual std::string GetRpfName() const { return m_RpfName; };

	virtual CRagePackFile* GetOrOpenChildren(FileInfo* pFile) = 0;

	// return -1 : not found
	// return 0  : is directory
	// return 1  : is file
	virtual int GetEntryAt(std::string path, void*& pEntry) = 0;
	virtual uint32 ReadDataEntry(FileInfo* pFile, void* buffer, uint32 offset, uint32 size, bool decompress = true) = 0;

	virtual void Close() = 0;
};


namespace rage7
{
#pragma pack(push, 1)
	struct fwRSC7Head
	{
		uint32 m_Magic;
		uint32 m_Version;
		uint32 m_SysFlags;
		uint32 m_GfxFlags;
	};


	struct fwRpfHead
	{
		uint32 m_Magic;
		uint32 m_Entries; // entries count
		uint32 m_NamesSize;
		uint32 m_EncryptionFlag;
	};


	struct fwRpfDirectory
	{
		uint16 m_NameOffset = 0;
		uint16 m_Unk0 = 0;
		uint32 m_EntryType = 0;
		uint32 m_EntryIndex = 0;
		uint32 m_EntriesCount = 0;
	};

	struct fwRpfBinFile
	{
		uint16 m_NameOffset = 0;
		uint24 m_FileSize = {}; // Taille compressé (si il n'y a pas de compression, il faut utiliser uncompressedSize)
		uint24 m_FileOffset = {};
		uint32 m_UncompressedSize = 0; // used to describe the uncompressed size OR THE FILE SIZE IF THE FILE IS NOT COMPRESSED
		uint32 m_IsEncrypted = 0; // 0 = no, 1 = yes

		fwRpfBinFile() {};

		uint32 GetPhysicalSize()
		{
			if (IsCompressed())
				return m_FileSize;

			return m_UncompressedSize;
		};

		bool IsCompressed() const {
			return m_FileSize != 0 && m_UncompressedSize != 0;
		};
	};

	struct fwRpfRscFile
	{
		uint16 m_NameOffset = 0;
		uint24 m_FileSize = {};
		uint24 m_FileOffset = {};
		uint32 m_SysFlags = 0;
		uint32 m_GfxFlags = 0;
	};

	struct fwRpfEntry
	{
		union {
			fwRpfDirectory dir;
			fwRpfBinFile file;
			fwRpfRscFile rsc;
		} entry;
	};
#pragma pack(pop)



	enum EncryptFlags : uint32 {
		EF_OPEN = (uint32)'NEPO',
		EF_AES = 0xFFFFFF9,
		EF_ROCK = 0xFEFFFFF,
		EF_NONE = 0
	};

	// GTA V  RPF7
	class CRagePackFile : public ::CRagePackFile
	{
	private:
		// for root RPF
		CRagePackFile* m_Parent = 0; // null if is a root rpf
		CMapFile m_MapFile = {};

		// for all RPF
		std::map<std::string, fwRpfEntry> m_Entries; // name entry, entry
		ContainerInfo m_Root = {};

		std::map<FileInfo*, CRagePackFile*> m_ChildrensOpened;

		uint8* m_pData = 0;
		uint64 m_Size = 0;

		bool m_IsOPEN;
		bool m_IsAES;
		bool m_IsROCK;

		// following data is dedicated to the root RPF
		bool m_IsRootRPF;

	public:
		CRagePackFile() {};
		~CRagePackFile() {
			m_Root.Release();
		}

		void Reset() {
			m_Entries.clear();
		}

		virtual ContainerInfo* GetRoot() override { return &m_Root; };
		virtual CRagePackFile* GetOrOpenChildren(FileInfo* pFile) override;

		// rpfName is not necessary if the RPF is OPEN
		virtual bool Open(uint8* pRPF, uint64 size, std::string rpfName) override;
		virtual bool Open(const wchar_t* rootRpfPath) override;

		// return -1 : not found
		// return 0  : is directory
		// return 1  : is file
		virtual int GetEntryAt(std::string path, void*& pEntry);
		virtual uint32 ReadDataEntry(FileInfo* pFile, void* buffer, uint32 offset, uint32 size, bool decompress) override;

		virtual void Close() override;
	};




	struct sFolder;
	struct sFile;

	struct Entry
	{
		std::string name;
		bool isFolder;
		void* pEntry = 0; // sFile* or sFolder*

		~Entry();
	};

	struct sFolder
	{
		std::vector<Entry*> entries;

		~sFolder();
		void sort();
	};
	struct sFile {
		uint8* data;
		uint32 size; // physical size
		uint32 uncompressed = 0; // size uncompressed (only if file is compressed)
	};


	class CPackageBuilder7
	{
	private:
		Entry m_Root = {};
		uint64 m_Size = 0;
		uint32 m_DataOffset = 0; // offset of file data begin (size of RPF total head)
		uint32 m_EntriesCount = 0;

	public:
		bool BuildMap(std::vector<rage::OperationRpf> operations);

		uint64 GetSize() const { return m_Size; }

		// écrit les données du RPF dans le tampon "dest" (le tampon doit au moins faire la taille GetSize())
		void WriteAt(void* dest);
	};
}