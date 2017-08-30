// 3D STL loader and saver example

#include <stdio.h>
#include <string.h>
#include "c4d.h"
#include "c4d_symbols.h"
#include "fsdkstlimport.h"
#include "fsdkstlexport.h"
#include "main.h"

class STLLoaderData : public SceneLoaderData
{
public:
	virtual Bool Init(GeListNode* node);
	virtual Bool Identify(BaseSceneLoader* node, const Filename& name, UChar* probe, Int32 size);
	virtual FILEERROR Load(BaseSceneLoader* node, const Filename& name, BaseDocument* doc, SCENEFILTER filterflags, String* error, BaseThread* bt);

	static NodeData* Alloc(void) { return NewObjClear(STLLoaderData); }
};

Bool STLLoaderData::Init(GeListNode* node)
{
	BaseContainer* data = ((BaseList2D*)node)->GetDataInstance();

	AutoAlloc<UnitScaleData> unit;
	if (unit)
		data->SetData(SDKSTLIMPORTFILTER_SCALE, GeData(CUSTOMDATATYPE_UNITSCALE, *unit));

	return true;
}

class STLSaverData : public SceneSaverData
{
public:
	virtual Bool Init(GeListNode* node);
	virtual FILEERROR Save(BaseSceneSaver* node, const Filename& name, BaseDocument* doc, SCENEFILTER filterflags);

	static NodeData* Alloc(void) { return NewObjClear(STLSaverData); }
};

Bool STLSaverData::Init(GeListNode* node)
{
	BaseContainer* data = ((BaseList2D*)node)->GetDataInstance();

	AutoAlloc<UnitScaleData> unit;
	if (unit)
		data->SetData(SDKSTLEXPORTFILTER_SCALE, GeData(CUSTOMDATATYPE_UNITSCALE, *unit));

	return true;
}

#define ARG_MAXCHARS 256
#define	STL_SPEEDUP	 4096

struct ZPolygon
{
	Vector a, b, c;
};

class STLLOAD
{
public:
	BaseFile*			 file;
	Int32					 flags;
	Char					 str[ARG_MAXCHARS], speedup[STL_SPEEDUP];
	Int						 filepos, filelen, vbuf, vcnt;
	PolygonObject* op;
	ZPolygon*			 vadr;

	STLLOAD(void);
	~STLLOAD(void);

	Bool ReadArg(void);
	Bool ReadFloat(Float& l);
};

Bool STLLOAD::ReadArg(void)
{
	Char* pos = str;
	Bool	ok	= true, trenner;
	Int32 mw, cnt = 0, len = ARG_MAXCHARS - 1;

	do
	{
		mw = filepos % STL_SPEEDUP;

		if (!mw)
		{
			if (filepos + STL_SPEEDUP >= filelen)
				ok = file->ReadBytes(speedup, filelen - filepos) > 0;
			else
				ok = file->ReadBytes(speedup, STL_SPEEDUP) > 0;

			if (flags & SCENEFILTER_PROGRESSALLOWED)
				StatusSetBar(Int32(Float(filepos) / Float(filelen) * 100.0));
		}

		*pos = speedup[mw];
		filepos++;

		trenner	 = *pos == 10 || *pos == 13 || *pos == ' ' || *pos == 9;

		if (!trenner)
		{
			pos++;
			cnt++;
			len--;
		}

	} while (ok && cnt + 1 < ARG_MAXCHARS && (!trenner || cnt == 0) && filepos < filelen && len > 0);

	if (!ok)
	{
		return false;
	}
	else
	{
		*pos = 0;
		return true;
	}
}

Bool STLLOAD::ReadFloat(Float& r)
{
	double z;
	if (!ReadArg() || sscanf(str, "%lf", &z) != 1)
	{
		file->SetError(FILEERROR_WRONG_VALUE);
		return false;
	}
	r = (Float)z;
	return true;
}

STLLOAD::STLLOAD(void)
{
	flags = 0;
	op = nullptr;
	str[0]	= 0;
	filepos	= 0;
	filelen	= 0;
	file = BaseFile::Alloc();
	vadr = nullptr;
	vcnt = 0;
	vbuf = 500;
}

STLLOAD::~STLLOAD(void)
{
	BaseFile::Free(file);
	blDelete(op);
	DeleteMem(vadr);
}

static void CapString(Char* s)
{
	Int32 i;
	for (i = 0; s[i]; i++)
		if (s[i] >= 'a' && s[i] <= 'z')
			s[i] -= 32;
}

static Int32 LexCompare(const Char* s1, const Char* s2)
{
	Char a1[500], a2[500];
	strcpy(a1, s1); CapString(a1);
	strcpy(a2, s2); CapString(a2);
	return strcmp(a1, a2);
}

Bool STLLoaderData::Identify(BaseSceneLoader* node, const Filename& name, UChar* probe, Int32 size)
{
	Int32 pos = 0;
	Char	str[6];

	CopyMem(probe, str, 5);
	str[5] = 0;

	if (!name.CheckSuffix("stl"))
		return false;
	if (LexCompare("solid", str) == 0)
		return true;

	pos = 84 + 48;
	while (pos + 1 < size)
	{
		if (probe[pos] != 0 || probe[pos + 1] != 0)
			return false;
		pos += 50;
	}

	return true;
}

FILEERROR STLLoaderData::Load(BaseSceneLoader* node, const Filename& name, BaseDocument* doc, SCENEFILTER flags, String* error, BaseThread* thread)
{
	BaseContainer bc;
	Int32					mode = 0, pnt = 0, pcnt = 0, i, cnt, index;
	Vector				v[3], *padr = nullptr;
	CPolygon*			vadr = nullptr;
	STLLOAD				stl;
	Char c;

	if (!(flags & SCENEFILTER_OBJECTS))
		return FILEERROR_NONE;

	if (!stl.file)
		return FILEERROR_OUTOFMEMORY;
	if (!stl.file->Open(name, FILEOPEN_READ, FILEDIALOG_NONE, BYTEORDER_INTEL))
		return stl.file->GetError();

	Float scl = 1.0;
	const UnitScaleData* src = (const UnitScaleData*)node->GetDataInstance()->GetCustomDataType(SDKSTLIMPORTFILTER_SCALE, CUSTOMDATATYPE_UNITSCALE);
	if (src)
		scl = CalculateTranslationScale(src, (const UnitScaleData*)doc->GetDataInstance()->GetCustomDataType(DOCUMENT_DOCUNIT, CUSTOMDATATYPE_UNITSCALE));

	stl.flags = flags;
	stl.filelen = (Int)stl.file->GetLength();

	if (stl.ReadArg() && !LexCompare("solid", stl.str))	// ASCII mode
	{
		while (stl.filepos < stl.filelen && stl.ReadArg())
		{
			if (thread && thread->TestBreak())
			{
				stl.file->SetError(FILEERROR_USERBREAK); break;
			}

			for (index = 0; index < (Int32)strlen(stl.str); index++)
			{
				Char chr = stl.str[index];
				if (chr >= 1 && chr <= 7)
					goto Binary;
			}

			if (!LexCompare(stl.str, "facet"))
			{
				mode = 1;
			}
			else if (!LexCompare(stl.str, "outer") && mode == 1)
			{
				mode = 2;
			}
			else if (!LexCompare(stl.str, "loop") && mode == 2)
			{
				mode = 3;
			}
			else if (!LexCompare(stl.str, "vertex") && mode == 3 && pnt < 3)
			{
				stl.ReadFloat(v[pnt].x);
				stl.ReadFloat(v[pnt].z);
				stl.ReadFloat(v[pnt].y);
				pnt++;
			}
			else if (!LexCompare(stl.str, "endloop") || !LexCompare(stl.str, "endfacet"))
			{
				if (pnt == 3)
				{
					if (!stl.vadr || stl.vcnt >= stl.vbuf)
					{
						ZPolygon* nvadr = nullptr;
						if (stl.vbuf > 0)
							nvadr = NewMemClear(ZPolygon, stl.vbuf * 2);
						if (!nvadr)
							return FILEERROR_OUTOFMEMORY;
						CopyMemType(stl.vadr, nvadr, stl.vcnt);
						DeleteMem(stl.vadr);
						stl.vadr	= nvadr;
						stl.vbuf *= 2;
					}

					stl.vadr[stl.vcnt].a = v[0];
					stl.vadr[stl.vcnt].b = v[1];
					stl.vadr[stl.vcnt].c = v[2];
					stl.vcnt++;
				}
				mode = 0;
				pnt	 = 0;
			}
		}
	}
	else
	{
Binary:

		stl.file->Seek(80, FILESEEK_START);
		Int32 val;
		if (!stl.file->ReadInt32(&val) || val <= 0)
			return FILEERROR_WRONG_VALUE;
		stl.vbuf = val;

		if (stl.vbuf > 0)
			stl.vadr = NewMemClear(ZPolygon, stl.vbuf);
		else
			stl.vadr = nullptr;
		if (!stl.vadr)
			return FILEERROR_OUTOFMEMORY;

		for (cnt = 0; cnt < stl.vbuf; cnt++)
		{
			if (thread && thread->TestBreak())
			{
				stl.file->SetError(FILEERROR_USERBREAK); break;
			}

			if (stl.file->GetPosition() >= stl.filelen)
				return FILEERROR_WRONG_VALUE;

			stl.file->Seek(12);

			Float32 tmp;
			stl.file->ReadFloat32(&tmp); stl.vadr[stl.vcnt].a.x = tmp;
			stl.file->ReadFloat32(&tmp); stl.vadr[stl.vcnt].a.z = tmp;
			stl.file->ReadFloat32(&tmp); stl.vadr[stl.vcnt].a.y = tmp;
			stl.file->ReadFloat32(&tmp); stl.vadr[stl.vcnt].b.x = tmp;
			stl.file->ReadFloat32(&tmp); stl.vadr[stl.vcnt].b.z = tmp;
			stl.file->ReadFloat32(&tmp); stl.vadr[stl.vcnt].b.y = tmp;
			stl.file->ReadFloat32(&tmp); stl.vadr[stl.vcnt].c.x = tmp;
			stl.file->ReadFloat32(&tmp); stl.vadr[stl.vcnt].c.z = tmp;
			stl.file->ReadFloat32(&tmp); stl.vadr[stl.vcnt].c.y = tmp;
			stl.file->ReadChar(&c);
			if (c != 0)
				return FILEERROR_WRONG_VALUE;
			stl.file->ReadChar(&c);
			if (c != 0)
				return FILEERROR_WRONG_VALUE;
			stl.vcnt++;

			if (!(cnt & 63) && flags & SCENEFILTER_PROGRESSALLOWED)
				StatusSetBar(Int32(Float(cnt) / Float(stl.vbuf) * 100.0));
		}
	}

	Filename nn = name;
	nn.ClearSuffix();
	stl.op = PolygonObject::Alloc(Int32(stl.vcnt * 3), (Int32)stl.vcnt);
	if (!stl.op)
		return FILEERROR_OUTOFMEMORY;
	stl.op->SetName(nn.GetFileString());

	padr = stl.op->GetPointW();
	vadr = stl.op->GetPolygonW();

	for (i = 0; i < stl.vcnt; i++)
	{
		vadr[i] = CPolygon(pcnt, pcnt + 2, pcnt + 1);
		padr[pcnt++] = stl.vadr[i].a;
		padr[pcnt++] = stl.vadr[i].b;
		padr[pcnt++] = stl.vadr[i].c;
	}

	pcnt = stl.op->GetPointCount();
	for (i = 0; i < pcnt; i++)
		padr[i] *= scl;

	stl.op->Message(MSG_UPDATE);
	doc->InsertObject(stl.op, nullptr, nullptr);

	ModelingCommandData mdat;
	mdat.doc = doc;
	mdat.op	 = stl.op;
	mdat.bc	 = &bc;

	bc.SetBool(MDATA_OPTIMIZE_POINTS, true);
	bc.SetBool(MDATA_OPTIMIZE_POLYGONS, true);
	bc.SetFloat(MDATA_OPTIMIZE_TOLERANCE, 0.0);
	if (stl.file->GetError() != FILEERROR_NONE || !SendModelingCommand(MCOMMAND_OPTIMIZE, mdat))
		blDelete(stl.op);

	stl.op = nullptr;	// detach object from structure

	return stl.file->GetError();
}

class STLSAVE
{
public:
	BaseFile*			file;
	BaseDocument* doc;
	Int32					pos, cnt, flags;

	STLSAVE(void);
	~STLSAVE(void);

};

STLSAVE::STLSAVE(void)
{
	doc	= nullptr;
	pos	= 0;
	cnt	= 0;
	flags = 0;
	file	= BaseFile::Alloc();
}

STLSAVE::~STLSAVE(void)
{
	BaseFile::Free(file);
	BaseDocument::Free(doc);
}

static void CountPolygons(STLSAVE& stl, BaseObject* op)
{
	while (op)
	{
		if (op->GetType() == Opolygon)
		{
			const CPolygon* vadr = ToPoly(op)->GetPolygonR();
			Int32 i, vcnt = ToPoly(op)->GetPolygonCount();

			stl.cnt += vcnt;
			for (i = 0; i < vcnt; i++)
				if (vadr[i].c != vadr[i].d)
					stl.cnt++;
		}
		CountPolygons(stl, op->GetDown());
		op = op->GetNext();
	}
}

static void WritePolygon(STLSAVE& stl, const Vector& pa, const Vector& pb, const Vector& pc)
{
	Vector n = !Cross(pb - pa, pc - pa);

	stl.file->WriteFloat32((Float32)n.x); stl.file->WriteFloat32((Float32)n.z);	stl.file->WriteFloat32((Float32)n.y);
	stl.file->WriteFloat32((Float32)pa.x); stl.file->WriteFloat32((Float32)pa.z);	stl.file->WriteFloat32((Float32)pa.y);
	stl.file->WriteFloat32((Float32)pc.x); stl.file->WriteFloat32((Float32)pc.z);	stl.file->WriteFloat32((Float32)pc.y);
	stl.file->WriteFloat32((Float32)pb.x); stl.file->WriteFloat32((Float32)pb.z);	stl.file->WriteFloat32((Float32)pb.y);
	stl.file->WriteChar(0);
	stl.file->WriteChar(0);

	stl.pos++;
	if (!(stl.pos & 63) && stl.flags & SCENEFILTER_PROGRESSALLOWED)
		StatusSetBar(Int32(Float(stl.pos) / Float(stl.cnt) * 100.0));
}

static void WriteObjects(STLSAVE& stl, BaseObject* op, Matrix up)
{
	Matrix mg;
	while (op)
	{
		mg = up * op->GetMl();
		if (op->GetType() == Opolygon)
		{
			const Vector*		padr = ToPoly(op)->GetPointR();
			const CPolygon* vadr = ToPoly(op)->GetPolygonR();
			Int32 i, vcnt = ToPoly(op)->GetPolygonCount();

			for (i = 0; i < vcnt; i++)
			{
				WritePolygon(stl, mg * padr[vadr[i].a], mg * padr[vadr[i].b], mg * padr[vadr[i].c]);

				if (vadr[i].c != vadr[i].d)
					WritePolygon(stl, mg * padr[vadr[i].a], mg * padr[vadr[i].c], mg * padr[vadr[i].d]);
			}
		}
		WriteObjects(stl, op->GetDown(), mg);
		op = op->GetNext();
	}
}

FILEERROR STLSaverData::Save(BaseSceneSaver* node, const Filename& name, BaseDocument* doc, SCENEFILTER flags)
{
	if (!(flags & SCENEFILTER_OBJECTS))
		return FILEERROR_NONE;

	Float		scl;
	STLSAVE stl;
	Char		header[80];

	const UnitScaleData* scale = (const UnitScaleData*)node->GetDataInstance()->GetCustomDataType(SDKSTLEXPORTFILTER_SCALE, CUSTOMDATATYPE_UNITSCALE);
	scl = CalculateTranslationScale((const UnitScaleData*)doc->GetDataInstance()->GetCustomDataType(DOCUMENT_DOCUNIT, CUSTOMDATATYPE_UNITSCALE), scale);

	stl.flags = flags;
	stl.doc = doc->Polygonize();

	if (!stl.doc || !stl.file)
		return FILEERROR_OUTOFMEMORY;

	CountPolygons(stl, stl.doc->GetFirstObject());

	if (!stl.file->Open(name, FILEOPEN_WRITE, FILEDIALOG_NONE, BYTEORDER_INTEL))
		return stl.file->GetError();

	ClearMem(header, sizeof(header));
	name.GetFileString().GetCString(header, 78, STRINGENCODING_7BIT);
	stl.file->WriteBytes(header, 80);
	stl.file->WriteInt32(stl.cnt);

	WriteObjects(stl, stl.doc->GetFirstObject(), MatrixScale(Vector(scl)));

	return stl.file->GetError();
}

Bool RegisterSTL(void)
{
	String name = GeLoadString(IDS_STL);
	if (!RegisterSceneLoaderPlugin(1000984, name, 0, STLLoaderData::Alloc, "Fsdkstlimport", nullptr))
		return false;
	if (!RegisterSceneSaverPlugin(1000958, name, 0, STLSaverData::Alloc, "Fsdkstlexport", "stl"))
		return false;
	return true;
}
