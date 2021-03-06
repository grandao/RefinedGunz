#pragma once

#include <list>
#include <unordered_map>
#include <string>
#include "MZip.h"

#define DEF_EXT	"mrs"

struct MZFILEDESC
{
	char m_szFileName[_MAX_PATH];
	char m_szZFileName[_MAX_PATH];
	size_t m_iSize;
	unsigned int m_crc32;
	unsigned long m_modTime;
	char *CachedContents = nullptr;
};

using ZFLIST = std::unordered_map<std::string, MZFILEDESC*>;
using ZFLISTITOR = ZFLIST::iterator;

template<size_t size> void GetRefineFilename(char(&szRefine)[size], const char *szSource) {
	GetRefineFilename(szRefine, size, szSource); }
void GetRefineFilename(char *szRefine, int maxlen, const char *szSource);
unsigned MGetCRC32(const char *data, int nLength);

#ifdef WIN32
class MMappedFile
{
public:
	MMappedFile() {}
	MMappedFile(const char* Filename);
	MMappedFile(const MMappedFile&) = delete;
	MMappedFile(MMappedFile&&);
	~MMappedFile();
	MMappedFile& operator =(MMappedFile&& src)
	{
		this->~MMappedFile();
		new (this) MMappedFile(std::move(src));
		return *this;
	}

	bool Dead() const { return bDead; }
	auto GetPointer() const { return reinterpret_cast<const char*>(View); }
	auto GetSize() const { return Size; }

private:
	bool bDead = true;
	HANDLE File = INVALID_HANDLE_VALUE;
	HANDLE Mapping = INVALID_HANDLE_VALUE;
	HANDLE View = INVALID_HANDLE_VALUE;
	size_t Size = 0;
};
#endif

#define ARCHIVE_CACHE_MMAP

struct Archive
{
#ifdef ARCHIVE_CACHE_MMAP
	Archive(const char* Filename) : File(Filename) {}
	MMappedFile File;
#endif

	int Index = 0;
	std::vector<MZFILEDESC*> Files;
#ifdef _DEBUG
	std::string Name;
#endif
};

class MZFileSystem final
{
public:
	MZFileSystem();
	~MZFileSystem();

	bool Create(const char* szBasePath,const char* szUpdateName=NULL);
	void Destroy(void);

	int GetFileCount(void) const;
	const char* GetFileName(int i);
	const MZFILEDESC* GetFileDesc(int i);

	auto* GetBasePath() const { return m_szBasePath; }

	MZFILEDESC* GetFileDesc(const char* szFileName);

	unsigned int GetCRC32(const char* szFileName);
	unsigned int GetTotalCRC();

	int GetFileLength(const char* szFileName);
	int GetFileLength(int i);

	bool IsZipFile(const char* szFileName);

	bool ReadFile(const char* szFileName, void* pData, int nMaxSize);

	void SetFileCheckList(class MZFileCheckList *pCheckList) { m_pCheckList = pCheckList; }
	MZFileCheckList *GetFileCheckList()	{ return m_pCheckList; }

	int CacheArchive(const char* Filename);
	void ReleaseArchive(int Index);

protected:
	bool AddItem(MZFILEDESC*);

	void RemoveFileList(void);
	void RefreshFileList(const char* szBasePath);

	int GetUpdatePackageNumber(const char *szPackageFileName);

private:
	char		m_szBasePath[256];
	ZFLIST		m_ZFileList;
	ZFLISTITOR	m_iterator;
	int			m_nIndex;

	char		m_szUpdateName[256];

	MZFileCheckList *m_pCheckList;

	std::vector<Archive> ArchiveCache;
	int ArchiveIndexCounter = 0;
};

struct MZFileBuffer
{
	MZFileBuffer(char* ptr, bool owned) : ptr(ptr), owned(owned) {}
	MZFileBuffer(MZFileBuffer&& src) : ptr(src.ptr), owned(src.owned) { src.ptr = nullptr; }
	~MZFileBuffer() { Destroy(); }

	MZFileBuffer& operator =(MZFileBuffer&& src)
	{
		this->~MZFileBuffer();
		new (this) MZFileBuffer{ std::move(src) };
		return *this;
	}

	void Destroy()
	{
		if (owned && ptr)
		{
			delete[] ptr;
			ptr = nullptr;
		}
	}

	auto get() { return ptr; }

private:
	char* ptr{};
	bool owned{};
};

class MZFile final
{
public:
	MZFile();
	~MZFile();

	bool Open(const char* szFileName, MZFileSystem* pZFS = nullptr);
	bool Open(const char* szFileName, const char* szZipFileName,
		bool bFileCheck = false, unsigned int crc32 = 0);

	bool Seek(long off, int mode = current);
	i64 Tell() const;

	void Close();

	static void SetReadMode(unsigned long mode) { m_dwReadMode = mode; }
	static unsigned long GetReadMode() { return m_dwReadMode; }
	static bool isMode(unsigned long mode) { return (m_dwReadMode & mode) != 0; }

	auto GetLength() const { return m_nFileSize; }
	bool Read(void* pBuffer, int nMaxSize);
	template <typename T>
	bool Read(T& dest) {
		return Read(&dest, sizeof(dest));
	}
	bool LoadFile();
	MZFileBuffer Release();
	auto IsCachedData() const { return CachedData; }

	enum SeekPos { begin = 0x0, current = 0x1, end = 0x2 };

protected:
	void ReleaseData();

	FILE*	m_fp;

	MZip	m_Zip;

	char*	m_pData;
	int		m_nDataSize;

	int		m_nIndexInZip;
	unsigned int m_crc32;

	int		m_nPos;
	int		m_nFileSize;

	char	m_FileName[256];
	char	m_ZipFileName[256];

	bool m_IsZipFile;
	bool CachedData = false;

	static unsigned long m_dwReadMode;
};

class MZFileCheckList
{
public:
	bool Open(const char *szFileName, MZFileSystem *pfs = nullptr);

	unsigned int GetCRC32(const char *szFileName) const;
	unsigned int GetCRC32()	const { return m_crc32; }

private:
	unsigned int m_crc32;
	std::unordered_map<std::string, unsigned int> m_fileList;
};