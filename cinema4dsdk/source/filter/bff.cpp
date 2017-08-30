// BFF (=basic file format) image loader and saver example

#include "c4d.h"
#include "c4d_symbols.h"
#include "main.h"

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_BFFLOADER 1000983
#define ID_BFFSAVER	 1000957

class BFFLoaderData : public BitmapLoaderData
{
public:
	virtual Bool Identify(const Filename& name, UChar* probe, Int32 size);
	virtual IMAGERESULT Load(const Filename& name, BaseBitmap* bm, Int32 frame);
	virtual Int32 GetSaver(void) { return ID_BFFSAVER; }
};

class BFFSaverData : public BitmapSaverData
{
public:
	virtual IMAGERESULT Save(const Filename& name, BaseBitmap* bm, BaseContainer* data, SAVEBIT savebits);
	virtual Int32 GetMaxResolution(Bool layers) { return LIMIT<Int32>::MAX; }
};

Bool BFFLoaderData::Identify(const Filename& name, UChar* probe, Int32 size)
{
	if (!probe || size < 4)
		return false;
	UInt32* p = (UInt32*)probe, v1 = p[0];
	lMotor(&v1);
	return v1 == 0x42464600;
}

IMAGERESULT BFFLoaderData::Load(const Filename& name, BaseBitmap* bm, Int32 frame)
{
	Int32	 y, bw, bh, header;
	Bool	 ok = true;
	UChar* line = nullptr;
	AutoAlloc<BaseFile>	file;
	if (!file)
		return IMAGERESULT_OUTOFMEMORY;

	if (!file->Open(name, FILEOPEN_READ, FILEDIALOG_NONE, BYTEORDER_MOTOROLA))
		return IMAGERESULT_FILEERROR;

	file->ReadInt32(&header);
	file->ReadInt32(&bw);
	file->ReadInt32(&bh);

	if (bw > 0)
		line = NewMemClear(UChar, 3 * bw);

	if (bm->Init(bw, bh, 24) != IMAGERESULT_OK || !line)
	{
		DeleteMem(line);
		return IMAGERESULT_OUTOFMEMORY;
	}

	for (y = 0; y < bh && ok; y++)
	{
		// check if current thread is canceled and return IMAGERESULT_THREADCANCELED
		file->ReadBytes(line, bw * 3);
		ok = bm->SetPixelCnt(0, y, bw, line, COLORBYTES_RGB, COLORMODE_RGB, PIXELCNT_0);
	}

	ok = ok && file->GetError() == FILEERROR_NONE;
	DeleteMem(line);

	return ok ? IMAGERESULT_OK : IMAGERESULT_FILEERROR;
}

IMAGERESULT BFFSaverData::Save(const Filename& name, BaseBitmap* bm, BaseContainer* data, SAVEBIT savebits)
{
	Int32	 y, bw, bh;
	Bool	 ok = true;
	UChar* line = nullptr;
	AutoAlloc<BaseFile> file;
	if (!file)
		return IMAGERESULT_OUTOFMEMORY;

	if (!file->Open(name, FILEOPEN_WRITE, FILEDIALOG_NONE, BYTEORDER_MOTOROLA))
		return IMAGERESULT_FILEERROR;

	bw = bm->GetBw();
	bh = bm->GetBh();

	file->WriteInt32(0x42464600);
	file->WriteInt32(bw);
	file->WriteInt32(bh);

	if (bw > 0)
		line = NewMemClear(UChar, 3 * bw);
	if (!line)
	{
		DeleteMem(line);
		return IMAGERESULT_OUTOFMEMORY;
	}

	for (y = 0; y < bh; y++)
	{
		bm->GetPixelCnt(0, y, bw, line, COLORBYTES_RGB, COLORMODE_RGB, PIXELCNT_0);
		file->WriteBytes(line, bw * 3);
	}

	ok = file->GetError() == FILEERROR_NONE;
	DeleteMem(line);

	return ok ? IMAGERESULT_OK : IMAGERESULT_FILEERROR;
}

Bool RegisterBFF(void)
{
	String name = GeLoadString(IDS_BFF);
	if (!RegisterBitmapLoaderPlugin(ID_BFFLOADER, name, 0, NewObjClear(BFFLoaderData)))
		return false;
	if (!RegisterBitmapSaverPlugin(ID_BFFSAVER, name, PLUGINFLAG_BITMAPSAVER_SUPPORT_8BIT, NewObjClear(BFFSaverData), "bff"))
		return false;
	return true;
}

