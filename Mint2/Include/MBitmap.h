#pragma once

#include "CMList.h"
#include "MTypes.h"

#define MBM_Normal 0
#define MBM_FlipLR 1
#define MBM_FlipUD 1<<1
#define MBM_RotL90 1<<2
#define MBM_RotR90 1<<3

#define MBITMAP_NAME_LENGTH		128

class MBitmap{
public:
#ifdef _DEBUG
	int		m_nTypeID;
#endif

public:
	char	m_szName[MBITMAP_NAME_LENGTH];
	DWORD	m_DrawMode;
public:
	MBitmap(void);
	virtual ~MBitmap(void);

	void CreatePartial(MBitmap *pBitmap,MRECT rt,const char *szName);

	virtual bool Create(const char* szName);
	virtual void Destroy();

	virtual void SetDrawMode(DWORD md) { m_DrawMode = md; }
	virtual DWORD GetDrawMode() { return m_DrawMode; }

	virtual int GetX() { return 0; }
	virtual int GetY() { return 0; }
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;

	virtual MBitmap *GetSourceBitmap() { return this; }
};

class MPartialBitmap : public MBitmap {
	MBitmap	*m_pSource;
	MRECT	m_Rect;
public:
	MPartialBitmap();
	MPartialBitmap(MBitmap *pBitmap, MRECT rt);

	virtual int GetX() override { return m_Rect.x; }
	virtual int GetY() override { return m_Rect.y; }
	virtual int GetWidth() override { return m_Rect.w; }
	virtual int GetHeight() override { return m_Rect.h; }

	virtual MBitmap *GetSourceBitmap() { return m_pSource; }
};

class MAniBitmap{
protected:
#ifdef _DEBUG
	friend class MDrawContext;
	int		m_nTypeID;
#endif
public:
	char	m_szName[MBITMAP_NAME_LENGTH];
private:
	CMLinkedList<MBitmap>	m_Bitmaps;
protected:
	int		m_nCurFrame;
	int		m_nDelay;
public:
	MAniBitmap();
	virtual ~MAniBitmap();
	bool Create(const char* szName);
	void Destroy();

	void Add(MBitmap* pBitmap);
	MBitmap* Get(int nFrame);
	MBitmap* Get();

	int GetFrameCount();
	int GetCurFrame();

	bool MoveNext();
	bool MovePrevious();
	void MoveFirst();
	void MoveLast();
	bool Move(int nFrame);

	int GetDelay();
	void SetDelay(int nDelay) { m_nDelay = nDelay; }
};