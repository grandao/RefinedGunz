#pragma once

#include "MUID.h"
#include "MTypes.h"
#include "mempool.h"

#include "SafeString.h"
#include "GlobalTypes.h"

class MCommandParamCondition;
class MCommandParamConditionMinMax;

enum MCommandParameterType{
	MPT_INT		= 0,
	MPT_UINT	= 1,
	MPT_FLOAT	= 2,
	MPT_BOOL	= 3,
	MPT_STR		= 4,
	MPT_VECTOR	= 5,
	MPT_POS		= 6,
	MPT_DIR		= 7,
	MPT_COLOR	= 8,
	MPT_UID		= 9,
	MPT_BLOB	= 10,

	MPT_CHAR	= 11,
	MPT_UCHAR	= 12,
	MPT_SHORT	= 13,
	MPT_USHORT	= 14,
	MPT_INT64	= 15,
	MPT_UINT64	= 16,

	MPT_SVECTOR	= 17,
	MPT_CMD     = 18,
	MPT_END		= 19,
};

#define MAX_BLOB_SIZE		(0x100000)

class MCommandParameterDesc{
protected:
	MCommandParameterType				m_nType;
	char								m_szDescription[64];
	std::vector<MCommandParamCondition*>		m_Conditions;
	void								InitializeConditions();
public:
	MCommandParameterDesc(MCommandParameterType nType, char* szDescription);
	virtual ~MCommandParameterDesc(void);

	MCommandParameterType GetType(void){ return m_nType; }
	const char* GetDescription(void){ return m_szDescription; }

	void AddCondition(MCommandParamCondition* pCondition);
	bool HasConditions() { return (!m_Conditions.empty()); }
	int GetConditionCount() { return (int)m_Conditions.size(); }
	MCommandParamCondition* GetCondition(int n) { return m_Conditions[n]; }

};

class MCommandParameter{
protected:
	MCommandParameterType	m_nType;
public:
	MCommandParameter(MCommandParameterType nType){ m_nType = nType; }
	virtual ~MCommandParameter(void){}

	MCommandParameterType GetType(void){ return m_nType; }

	virtual MCommandParameter* Clone(void) = 0;
	virtual void GetValue(void* p) = 0;
	virtual int GetData(char* pData, int nSize) = 0;
	virtual int SetData(const char* pData) = 0;
	virtual void *GetPointer(void) = 0; 

	virtual const char* GetClassName(void) = 0;

	template<size_t size>
	void GetString(char(&szValue)[size]) {
		GetString(szValue, size);
	}
	virtual void GetString(char* szValue, int maxlen) = 0;
	virtual int GetSize() = 0;
};

/// 정수 파라미터
class MCommandParameterInt : public MCommandParameter, public CMemPool<MCommandParameterInt> {
public:
	int		m_Value;
public:
	MCommandParameterInt(void);
	MCommandParameterInt(int Value);
	virtual ~MCommandParameterInt() override = default;

	virtual MCommandParameter* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer() override { return &m_Value; }
	virtual const char* GetClassName(void) override { return "Int"; }
	virtual void GetString(char* szValue, int maxlen) override { sprintf_safe(szValue, maxlen, "%d", m_Value); }
	virtual int GetSize() override { return sizeof(int); }
};

class MCommandParameterUInt : public MCommandParameter, public CMemPool<MCommandParameterUInt> {
public:
	unsigned int		m_Value;
public:
	MCommandParameterUInt(void);
	MCommandParameterUInt(unsigned int Value);
	virtual ~MCommandParameterUInt() override = default;

	virtual MCommandParameter* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData);
	virtual void *GetPointer() { return &m_Value; }
	virtual const char* GetClassName(void){ return "UInt"; }
	virtual void GetString(char* szValue, int maxlen){ sprintf_safe(szValue, maxlen, "%u", m_Value); }
	virtual int GetSize() { return sizeof(unsigned int); }
};

class MCommandParameterFloat : public MCommandParameter, public CMemPool<MCommandParameterFloat> {
public:
	float	m_Value;
public:
	MCommandParameterFloat(void);
	MCommandParameterFloat(float Value);

	virtual MCommandParameter* Clone(void);
	virtual void GetValue(void* p);
	virtual int GetData(char* pData, int nSize);
	virtual int SetData(const char* pData);
	virtual void *GetPointer() { return &m_Value; }
	virtual const char* GetClassName(void){ return "Float"; }
	virtual void GetString(char* szValue, int maxlen){ sprintf_safe(szValue, maxlen, "%f", m_Value); }
	virtual int GetSize() { return sizeof(float); }
};

class MCommandParameterString : public MCommandParameter{
public:
	char*	m_Value;
public:
	MCommandParameterString(void);
	MCommandParameterString(const char* Value);
	virtual ~MCommandParameterString(void) override;

	virtual MCommandParameter* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer() override { return &m_Value; }
	virtual const char* GetClassName(void) override { return "String"; }
	virtual void GetString(char* szValue, int maxlen) override
	{ 
		if( 0 != szValue )
		{
			if( 0 != m_Value )
				strcpy_safe(szValue, maxlen, m_Value); 
			else 
				strcpy_safe(szValue, maxlen, "\0" );
		}
	}
	virtual int GetSize() override;
};

template <typename AllocT>
class MCommandParameterStringCustomAlloc : public MCommandParameterString
{
public:
	explicit MCommandParameterStringCustomAlloc(AllocT& Alloc_) : Alloc(Alloc_) { }

	virtual ~MCommandParameterStringCustomAlloc() override
	{
		if (m_Value)
		{
			Alloc.deallocate((unsigned char*)m_Value, strlen(m_Value) + 1);
			m_Value = nullptr;
		}
	}

	virtual MCommandParameterStringCustomAlloc<AllocT>* Clone(void) override
	{
		return new MCommandParameterStringCustomAlloc<AllocT>(Alloc);
	}

	virtual int SetData(const char* pData) override
	{
		if (m_Value)
		{
			Alloc.deallocate((unsigned char*)m_Value, strlen(m_Value) + 1);
			m_Value = nullptr;
		}

		unsigned short nValueSize = 0;
		memcpy(&nValueSize, pData, sizeof(nValueSize));

		if ((nValueSize > (USHRT_MAX - 2)) || (0 == nValueSize))
		{
			return sizeof(nValueSize);
		}

		m_Value = (char*)Alloc.allocate(nValueSize);

		memcpy(m_Value, pData + sizeof(nValueSize), nValueSize);
		return nValueSize + sizeof(nValueSize);
	}

private:
	AllocT& Alloc;
};

class MCommandParameterVector : public MCommandParameter {
public:
	float	m_fX;
	float	m_fY;
	float	m_fZ;
public:
	MCommandParameterVector(void);
	MCommandParameterVector(float x ,float y, float z);
	virtual ~MCommandParameterVector(void) override;

	virtual MCommandParameter* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer() override { return &m_fX; }
	virtual const char* GetClassName(void) override { return "Vector"; }
	virtual void GetString(char* szValue, int maxlen) override { sprintf_safe(szValue, maxlen, "%.2f,%.2f,%.2f", m_fX, m_fY, m_fZ); }
	virtual int GetSize() override { return (sizeof(float)*3); }
};

class MCommandParameterPos : public MCommandParameterVector, public CMemPool<MCommandParameterPos> {
public:
	MCommandParameterPos(void) : MCommandParameterVector() { m_nType=MPT_POS; }
	MCommandParameterPos(float x, float y, float z) : MCommandParameterVector(x, y, z){ m_nType=MPT_POS; }
	virtual ~MCommandParameterPos() override = default;

	virtual MCommandParameter* Clone(void) override { return new MCommandParameterPos(m_fX, m_fY, m_fZ); }
	virtual const char* GetClassName(void) override { return "Pos"; }
};

class MCommandParameterDir : public MCommandParameterVector, public CMemPool<MCommandParameterDir> {
public:
	MCommandParameterDir(void) : MCommandParameterVector() { m_nType=MPT_DIR; }
	MCommandParameterDir(float x, float y, float z) : MCommandParameterVector(x, y, z){ m_nType=MPT_DIR; }
	virtual ~MCommandParameterDir() override = default;

	virtual MCommandParameter* Clone(void) override { return new MCommandParameterDir(m_fX, m_fY, m_fZ); }
	virtual const char* GetClassName(void) override { return "Dir"; }
};

class MCommandParameterColor : public MCommandParameterVector, public CMemPool<MCommandParameterColor> {
public:
	MCommandParameterColor(void) : MCommandParameterVector() { m_nType=MPT_COLOR; }
	MCommandParameterColor(float r, float g, float b) : MCommandParameterVector(r, g, b){ m_nType=MPT_COLOR; }
	virtual ~MCommandParameterColor() override = default;

	virtual MCommandParameter* Clone(void) override { return new MCommandParameterColor(m_fX, m_fY, m_fZ); }
	virtual const char* GetClassName(void) override { return "Color"; }
};

class MCommandParameterBool : public MCommandParameter, public CMemPool<MCommandParameterBool> {
	bool	m_Value;
public:
	MCommandParameterBool(void) : MCommandParameter(MPT_BOOL) { }
	MCommandParameterBool(bool bValue) : MCommandParameter(MPT_BOOL) {
		m_Value = bValue;
	}
	virtual ~MCommandParameterBool() override = default;

	virtual MCommandParameter* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer(void) override;
	virtual const char* GetClassName(void) override { return "Bool"; }
	virtual void GetString(char* szValue, int maxlen) override { if (m_Value == true) strcpy_safe(szValue, maxlen, "true"); else strcpy_safe(szValue, maxlen, "false"); }
	virtual int GetSize() override { return sizeof(bool); }
};

class MCommandParameterUID : public MCommandParameter, public CMemPool<MCommandParameterUID> {
public:
	MUID	m_Value;
public:
	MCommandParameterUID(void);
	MCommandParameterUID(const MUID& uid);
	virtual ~MCommandParameterUID(void) override;

	virtual MCommandParameterUID* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer() override { return &m_Value; }
	virtual const char* GetClassName(void) override { return "UID"; }
	virtual void GetString(char* szValue, int maxlen) override { sprintf_safe(szValue, maxlen, "%u:%u", m_Value.High, m_Value.Low); }
	virtual int GetSize() override { return sizeof(MUID); }
};

class MCommandParameterBlob : public MCommandParameter{
public:
	void*	m_Value = nullptr;
	int		m_nSize = 0;
public:
	MCommandParameterBlob(void);
	explicit MCommandParameterBlob(size_t Size);
	explicit MCommandParameterBlob(const void* Value, int nSize);
	virtual ~MCommandParameterBlob(void) override;

	virtual MCommandParameterBlob* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer() override { return m_Value; }
	virtual const char* GetClassName(void) override { return "Blob"; }
	virtual void GetString(char* szValue, int maxlen) override { sprintf_safe(szValue, maxlen, "%02X%02X..", *((unsigned char*)(m_Value)), *((unsigned char*)(m_Value)+1)); }
	virtual int GetSize() override;

	size_t GetPayloadSize() const
	{
		return m_nSize;
	}
};

template <typename AllocT>
class MCommandParameterBlobCustomAlloc : public MCommandParameterBlob
{
public:
	explicit MCommandParameterBlobCustomAlloc(AllocT& Alloc_) : Alloc(Alloc_) { }

	virtual ~MCommandParameterBlobCustomAlloc() override
	{
		if (m_Value)
		{
			Alloc.deallocate((unsigned char*)m_Value, m_nSize);
			m_Value = nullptr;
		}
	}

	virtual MCommandParameterBlobCustomAlloc<AllocT>* Clone(void) override
	{
		return new MCommandParameterBlobCustomAlloc<AllocT>(Alloc);
	}

	virtual int SetData(const char* pData) override
	{
		if (m_Value)
		{
			Alloc.deallocate((unsigned char*)m_Value, m_nSize);
			m_Value = nullptr;
		}

		memcpy(&m_nSize, pData, sizeof(m_nSize));
		if (m_nSize > MAX_BLOB_SIZE)
		{
			m_Value = nullptr;
			m_nSize = 0;
			return sizeof(m_nSize);
		}

		m_Value = Alloc.allocate(m_nSize);

		memcpy(m_Value, pData + sizeof(m_nSize), m_nSize);

		return m_nSize + sizeof(m_nSize);
	}

private:
	AllocT& Alloc;
};

class MCommandParameterChar : public MCommandParameter, public CMemPool<MCommandParameterChar>
{
public:
	char	m_Value;
public:
	MCommandParameterChar(void);
	MCommandParameterChar(char Value);
	virtual ~MCommandParameterChar() override = default;

	virtual MCommandParameter* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer() override { return &m_Value; }
	virtual const char* GetClassName(void) override { return "Char"; }
	virtual void GetString(char* szValue, int maxlen) override { sprintf_safe(szValue, maxlen, "%d", m_Value); }
	virtual int GetSize() override { return sizeof(char); }
};

class MCommandParameterUChar : public MCommandParameter, public CMemPool<MCommandParameterUChar>
{
public:
	unsigned char	m_Value;
public:
	MCommandParameterUChar(void);
	MCommandParameterUChar(unsigned char Value);
	virtual ~MCommandParameterUChar() override = default;

	virtual MCommandParameter* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer() override { return &m_Value; }
	virtual const char* GetClassName(void) override { return "UChar"; }
	virtual void GetString(char* szValue, int maxlen) override { sprintf_safe(szValue, maxlen, "%u", m_Value); }
	virtual int GetSize() override { return sizeof(unsigned char); }
};

class MCommandParameterShort : public MCommandParameter, public CMemPool<MCommandParameterShort>
{
public:
	short	m_Value;
public:
	MCommandParameterShort(void);
	MCommandParameterShort(short Value);
	virtual ~MCommandParameterShort() override = default;

	virtual MCommandParameter* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer() override { return &m_Value; }
	virtual const char* GetClassName(void) override { return "Short"; }
	virtual void GetString(char* szValue, int maxlen) override { sprintf_safe(szValue, maxlen, "%d", m_Value); }
	virtual int GetSize() override { return sizeof(short); }
};

class MCommandParameterUShort : public MCommandParameter, public CMemPool<MCommandParameterUShort>
{
public:
	unsigned short	m_Value;
public:
	MCommandParameterUShort(void);
	MCommandParameterUShort(unsigned short Value);
	virtual ~MCommandParameterUShort() override = default;

	virtual MCommandParameter* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer() override { return &m_Value; }
	virtual const char* GetClassName(void) override { return "UShort"; }
	virtual void GetString(char* szValue, int maxlen) override { sprintf_safe(szValue, maxlen, "%u", m_Value); }
	virtual int GetSize() override { return sizeof(unsigned short); }
};

class MCommandParameterInt64 : public MCommandParameter, public CMemPool<MCommandParameterInt64>
{
public:
	int64	m_Value;
public:
	MCommandParameterInt64(void);
	MCommandParameterInt64(int64 Value);
	virtual ~MCommandParameterInt64() override = default;

	virtual MCommandParameter* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer() override { return &m_Value; }
	virtual const char* GetClassName(void) override { return "Int64"; }
	virtual void GetString(char* szValue, int maxlen) override { sprintf_safe(szValue, maxlen, "%lld", m_Value); }
	virtual int GetSize() override { return sizeof(int64); }
};

class MCommandParameterUInt64 : public MCommandParameter, public CMemPool<MCommandParameterUInt64>
{
public:
	uint64	m_Value;
public:
	MCommandParameterUInt64(void);
	MCommandParameterUInt64(uint64 Value);
	virtual ~MCommandParameterUInt64() override = default;

	virtual MCommandParameter* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer() override { return &m_Value; }
	virtual const char* GetClassName(void) override { return "UInt64"; }
	virtual void GetString(char* szValue, int maxlen) override { sprintf_safe(szValue, maxlen, "%llu", m_Value); }
	virtual int GetSize() override { return sizeof(uint64); }
};

class MCommandParameterShortVector : public MCommandParameter, public CMemPool<MCommandParameterShortVector> {
public:
	short	m_nX;
	short	m_nY;
	short	m_nZ;
public:
	MCommandParameterShortVector(void);
	MCommandParameterShortVector(short x ,short y, short z);
	MCommandParameterShortVector(float x ,float y, float z);
	virtual ~MCommandParameterShortVector(void) override;

	virtual MCommandParameter* Clone(void) override;
	virtual void GetValue(void* p) override;
	virtual int GetData(char* pData, int nSize) override;
	virtual int SetData(const char* pData) override;
	virtual void *GetPointer() override { return &m_nX; }
	virtual const char* GetClassName(void) override { return "ShortVector"; }
	virtual void GetString(char* szValue, int maxlen) override { sprintf_safe(szValue, maxlen, "%d,%d,%d", m_nX, m_nY, m_nZ); }
	virtual int GetSize() override { return (sizeof(short)*3); }
};

class MCommand;
class MCommandParameterCommand;
MCommandParameterCommand* MakeOwningMCmdParamCmd(const void* Data, size_t Size);

class MCommandParameterCommand : public MCommandParameter {
public:
	char* Data = nullptr;
	size_t Size = 0;
	bool OwnsData = false;
public:
	MCommandParameterCommand(void) : MCommandParameter(MPT_CMD), Data(nullptr), Size(0), OwnsData(false) { }
	explicit MCommandParameterCommand(const MCommand& Command);

	virtual ~MCommandParameterCommand(void) override
	{
		if (OwnsData)
			delete Data;
	}

	virtual MCommandParameterCommand* Clone(void) override
	{
		return MakeOwningMCmdParamCmd(Data, Size);
	}

	virtual void GetValue(void* p) override
	{
	}

	virtual int GetData(char* pData, int nSize) override
	{
		return Size;
	}
	virtual int SetData(const char* pData) override
	{
		return Size;
	}
	virtual void *GetPointer() override { return Data; }
	virtual const char* GetClassName(void) override { return "Command"; }
	virtual void GetString(char* szValue, int maxlen) override { sprintf_safe(szValue, maxlen, "Command ID %d/0x%04X", GetID(), GetID()); }
	virtual int GetSize() override
	{
		return Size;
	}

	int GetID() const
	{
		return *(const u16*)(Data + 2);
	}
};

static inline MCommandParameterCommand* MakeOwningMCmdParamCmd(const void* Data, size_t Size)
{
	auto Param = new MCommandParameterCommand;

	Param->Data = new char[Size];
	Param->Size = Size;
	Param->OwnsData = true;
	memcpy(Param->Data, Data, Size);

	return Param;
}

static inline MCommandParameterCommand* MakeNonOwningMCmdParamCmd(const void* Data, size_t Size)
{
	auto Param = new MCommandParameterCommand;

	Param->Data = (char*)Data;
	Param->Size = Size;
	Param->OwnsData = false;

	return Param;
}

class MCommandParamCondition
{
public:
	MCommandParamCondition(void) = default;
	virtual ~MCommandParamCondition(void) = default;
	virtual bool Check(MCommandParameter* pCP) = 0;
};

class MCommandParamConditionMinMax : public MCommandParamCondition
{
private:
	int m_nMin;
	int m_nMax;
public:
	MCommandParamConditionMinMax(int nMin, int nMax) : m_nMin(nMin), m_nMax(nMax) {}
	virtual ~MCommandParamConditionMinMax(void) override = default;
	virtual bool Check(MCommandParameter* pCP) override;
};



// Short Name
typedef MCommandParameterBlob			MCmdParamBlob;
typedef	MCommandParameterUID			MCmdParamUID;
typedef MCommandParameter				MCmdParam;
typedef MCommandParameterDesc			MCmdParamDesc;
typedef MCommandParameterInt			MCmdParamInt;
typedef MCommandParameterUInt			MCmdParamUInt;
typedef MCommandParameterFloat			MCmdParamFloat;
typedef MCommandParameterString			MCmdParamStr;
typedef MCommandParameterVector			MCmdParamVector;
typedef MCommandParameterPos			MCmdParamPos;
typedef MCommandParameterDir			MCmdParamDir;
typedef MCommandParameterColor			MCmdParamColor;
typedef MCommandParameterBool			MCmdParamBool;
typedef MCommandParameterChar			MCmdParamChar;
typedef MCommandParameterUChar			MCmdParamUChar;
typedef MCommandParameterShort			MCmdParamShort;
typedef MCommandParameterUShort			MCmdParamUShort;
typedef MCommandParameterInt64			MCmdParamInt64;
typedef MCommandParameterUInt64			MCmdParamUInt64;
typedef MCommandParameterShortVector	MCmdParamShortVector;
