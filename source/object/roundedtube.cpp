// generator object example (with no input objects)

#include "c4d.h"
#include "c4d_symbols.h"
#include "main.h"
#include "oroundedtube.h"

class RoundedTube : public ObjectData
{
public:
	String test;

	virtual Bool Init		(GeListNode* node);
	virtual Bool Read		(GeListNode* node, HyperFile* hf, Int32 level);
	virtual Bool Write	(GeListNode* node, HyperFile* hf);

	virtual void GetDimension		(BaseObject* op, Vector* mp, Vector* rad);
	virtual DRAWRESULT Draw			(BaseObject* op, DRAWPASS type, BaseDraw* bd, BaseDrawHelp* bh);
	virtual Int32 GetHandleCount(BaseObject* op);
	virtual void GetHandle(BaseObject* op, Int32 i, HandleInfo& info);
	virtual void SetHandle(BaseObject* op, Int32 i, Vector p, const HandleInfo& info);
	virtual BaseObject* GetVirtualObjects(BaseObject* op, HierarchyHelp* hh);
	virtual Bool Message(GeListNode* node, Int32 type, void* t_data);

	static NodeData* Alloc(void) { return NewObjClear(RoundedTube); }
};

Bool RoundedTube::Message(GeListNode* node, Int32 type, void* t_data)
{
	if (type == MSG_DESCRIPTION_VALIDATE)
	{
		BaseContainer* data = ((BaseObject*)node)->GetDataInstance();

		CutReal(*data, TUBEOBJECT_IRADX, 0.0, data->GetFloat(TUBEOBJECT_RAD));
		CutReal(*data, TUBEOBJECT_ROUNDRAD, 0.0, data->GetFloat(TUBEOBJECT_IRADX));
	}
	else if (type == MSG_MENUPREPARE)
	{
		((BaseObject*)node)->SetPhong(true, false, 0.0);
	}
	return true;
}

void RoundedTube::GetDimension(BaseObject* op, Vector* mp, Vector* rad)
{
	BaseContainer* data = op->GetDataInstance();

	Float rado, radx, rady;
	rado = data->GetFloat(TUBEOBJECT_RAD);
	radx = data->GetFloat(TUBEOBJECT_IRADX);
	rady = data->GetFloat(TUBEOBJECT_IRADY);

	*mp = Vector(0.0);
	switch (data->GetInt32(PRIM_AXIS))
	{
		case 0:
		case 1: *rad = Vector(rady, rado + radx, rado + radx); break;
		case 2:
		case 3: *rad = Vector(rado + radx, rady, rado + radx); break;
		case 4:
		case 5: *rad = Vector(rado + radx, rado + radx, rady); break;
	}
}

static BaseObject* GenerateLathe(Vector* cpadr, Int32 cpcnt, Int32 sub, BaseThread* bt)
{
	PolygonObject* op = nullptr;
	UVWStruct			 us;
	UVWTag*				 tag	= nullptr;
	Vector*				 padr = nullptr;
	CPolygon*			 vadr = nullptr;
	Int32			i, j, pcnt, vcnt, a, b, c, d;
	Float			len = 0.0, sn, cs, v1, v2, *uvadr = nullptr;
	UVWHandle	uvwptr;

	pcnt = cpcnt * sub;
	vcnt = cpcnt * sub;

	op = PolygonObject::Alloc(pcnt, vcnt);
	if (!op)
		goto error;

	tag	= (UVWTag*)op->MakeVariableTag(Tuvw, vcnt);
	if (!tag)
		goto error;

	padr = op->GetPointW();
	vadr = op->GetPolygonW();

	if (cpcnt + 1 > 0)
		uvadr = NewMemClear(Float, cpcnt + 1);
	if (!uvadr)
		goto error;

	uvadr[0] = 0.0;
	for (i = 0; i < cpcnt; i++)
	{
		uvadr[i] = len;
		len += Len(cpadr[(i + 1) % cpcnt] - cpadr[i]);
	}

	if (len > 0.0)
		len = 1.0 / len;
	for (i = 0; i < cpcnt; i++)
		uvadr[i] *= len;

	uvadr[cpcnt] = 1.0;

	vcnt = 0;

	uvwptr = tag->GetDataAddressW();
	for (i = 0; i < sub; i++)
	{
		SinCos(PI2 * Float(i) / Float(sub), sn, cs);

		v1 = Float(i) / Float(sub);
		v2 = Float(i + 1) / Float(sub);

		if (bt && bt->TestBreak())
			goto error;

		for (j = 0; j < cpcnt; j++)
		{
			a = cpcnt * i + j;
			padr[a] = Vector(cpadr[j].x * cs, cpadr[j].y, cpadr[j].x * sn);

			if (i < sub)
			{
				b = cpcnt * i + ((j + 1) % cpcnt);
				c = cpcnt * ((i + 1) % sub) + ((j + 1) % cpcnt);
				d = cpcnt * ((i + 1) % sub) + j;

				us = UVWStruct(Vector(v1, 1.0 - uvadr[j], 0.0), Vector(v1, 1.0 - uvadr[j + 1], 0.0), Vector(v2, 1.0 - uvadr[j + 1], 0.0), Vector(v2, 1.0 - uvadr[j], 0.0));
				tag->Set(uvwptr, vcnt, us);

				vadr[vcnt++] = CPolygon(a, b, c, d);
			}
		}
	}

	DeleteMem(uvadr);

	op->Message(MSG_UPDATE);
	op->SetPhong(true, true, Rad(80.0));
	return op;

error:
	DeleteMem(uvadr);
	blDelete(op);
	return nullptr;
}

static LineObject* GenerateIsoLathe(Vector* cpadr, Int32 cpcnt, Int32 sub)
{
	Int32 i;

	LineObject* op = LineObject::Alloc(cpcnt * 4 + sub * 4, 8);
	if (!op)
		return nullptr;
	Segment* sadr = op->GetSegmentW();
	Vector*	 padr = op->GetPointW();

	for (i = 0; i < 4; i++)
	{
		sadr[i].cnt = cpcnt;
		sadr[i].closed = true;
	}
	for (i = 0; i < 4; i++)
	{
		sadr[4 + i].cnt = sub;
		sadr[4 + i].closed = true;
	}

	Float	sn, cs;
	Int32	j;

	for (i = 0; i < 4; i++)
	{
		SinCos(Float(i) * PI05, sn, cs);
		for (j = 0; j < cpcnt; j++)
			padr[i * cpcnt + j] = Vector(cpadr[j].x * cs, cpadr[j].y, cpadr[j].x * sn);
	}

	for (i = 0; i < sub; i++)
	{
		SinCos(Float(i) / sub * PI2, sn, cs);
		for (j = 0; j < 4; j++)
			padr[4 * cpcnt + j * sub + i] = Vector(cpadr[cpcnt / 4 * j].x * cs, cpadr[cpcnt / 4 * j].y, cpadr[cpcnt / 4 * j].x * sn);
	}

	op->Message(MSG_UPDATE);
	return op;
}

Bool RoundedTube::Init(GeListNode* node)
{
	test = String("Test");

	BaseObject*		 op = (BaseObject*)node;
	BaseContainer* data = op->GetDataInstance();

	data->SetFloat(TUBEOBJECT_RAD, 200.0);
	data->SetFloat(TUBEOBJECT_IRADX, 50.0);
	data->SetFloat(TUBEOBJECT_IRADY, 50.0);
	data->SetInt32(TUBEOBJECT_SUB, 1);
	data->SetInt32(TUBEOBJECT_ROUNDSUB, 8);
	data->SetFloat(TUBEOBJECT_ROUNDRAD, 10.0);
	data->SetInt32(TUBEOBJECT_SEG, 36);
	data->SetInt32(PRIM_AXIS, PRIM_AXIS_YP);

	return true;
}

Bool RoundedTube::Read(GeListNode* node, HyperFile* hf, Int32 level)
{
	if (level >= 0)
	{
		hf->ReadString(&test);
	}
	return true;
}

Bool RoundedTube::Write(GeListNode* node, HyperFile* hf)
{
	hf->WriteString(test);
	return true;
}

static void SetAxis(BaseObject* obj, Int32 axis)
{
	PointObject* op = ToPoint(obj);
	if (axis == 2)
		return;

	Vector* padr = op->GetPointW();
	Int32		pcnt = op->GetPointCount(), i;

	switch (axis)
	{
		case 0:	// +X
			for (i = 0; i < pcnt; i++)
				padr[i] = Vector(padr[i].y, -padr[i].x, padr[i].z);
			break;

		case 1:	// -X
			for (i = 0; i < pcnt; i++)
				padr[i] = Vector(-padr[i].y, padr[i].x, padr[i].z);
			break;

		case 3:	// -Y
			for (i = 0; i < pcnt; i++)
				padr[i] = Vector(-padr[i].x, -padr[i].y, padr[i].z);
			break;

		case 4:	// +Z
			for (i = 0; i < pcnt; i++)
				padr[i] = Vector(padr[i].x, -padr[i].z, padr[i].y);
			break;

		case 5:	// -Z
			for (i = 0; i < pcnt; i++)
				padr[i] = Vector(padr[i].x, padr[i].z, -padr[i].y);
			break;
	}

	op->Message(MSG_UPDATE);
}

BaseObject* RoundedTube::GetVirtualObjects(BaseObject* op, HierarchyHelp* hh)
{
	LineObject* lop = nullptr;
	BaseObject* ret = nullptr;

	Bool dirty = op->CheckCache(hh) || op->IsDirty(DIRTYFLAGS_DATA);
	if (!dirty)
		return op->GetCache(hh);

	BaseContainer* data = op->GetDataInstance();

	Float rad = data->GetFloat(TUBEOBJECT_RAD, 200.0);
	Float iradx = data->GetFloat(TUBEOBJECT_IRADX, 50.0);
	Float irady = data->GetFloat(TUBEOBJECT_IRADY, 50.0);
	Float rrad	= data->GetFloat(TUBEOBJECT_ROUNDRAD, 10.0);
	Int32 sub	 = CalcLOD(data->GetInt32(TUBEOBJECT_SUB, 1), hh->GetLOD(), 1, 1000);
	Int32 rsub = CalcLOD(data->GetInt32(TUBEOBJECT_ROUNDSUB, 8), hh->GetLOD(), 1, 1000);
	Int32 seg	 = CalcLOD(data->GetInt32(TUBEOBJECT_SEG, 36), hh->GetLOD(), 3, 1000);
	Int32 axis = data->GetInt32(PRIM_AXIS);
	Int32 i;
	Float sn, cs;

	Int32 cpcnt = 4 * (sub + rsub);
	if (cpcnt <= 0)
		return nullptr;
	Vector* cpadr = NewMemClear(Vector, cpcnt);
	if (!cpadr)
		return nullptr;

	for (i = 0; i < sub; i++)
	{
		cpadr[i] = Vector(rad - iradx, (1.0 - Float(i) / Float(sub) * 2.0) * (irady - rrad), 0.0);
		cpadr[i + sub + rsub] = Vector(rad + (Float(i) / Float(sub) * 2.0 - 1.0) * (iradx - rrad), -irady, 0.0);
		cpadr[i + 2 * (sub + rsub)] = Vector(rad + iradx, (Float(i) / Float(sub) * 2.0 - 1.0) * (irady - rrad), 0.0);
		cpadr[i + 3 * (sub + rsub)] = Vector(rad + (1.0 - Float(i) / Float(sub) * 2.0) * (iradx - rrad), irady, 0.0);
	}
	for (i = 0; i < rsub; i++)
	{
		SinCos(Float(i) / Float(rsub) * PI05, sn, cs);
		cpadr[i + sub             ] = Vector(rad - (iradx - rrad + cs * rrad), -(irady - rrad + sn * rrad), 0.0);
		cpadr[i + sub + (sub + rsub)] = Vector(rad + (iradx - rrad + sn * rrad), -(irady - rrad + cs * rrad), 0.0);
		cpadr[i + sub + 2 * (sub + rsub)] = Vector(rad + (iradx - rrad + cs * rrad), +(irady - rrad + sn * rrad), 0.0);
		cpadr[i + sub + 3 * (sub + rsub)] = Vector(rad - (iradx - rrad + sn * rrad), +(irady - rrad + cs * rrad), 0.0);
	}

	ret = GenerateLathe(cpadr, cpcnt, seg, hh->GetThread());
	if (!ret)
		goto error;
	SetAxis(ret, axis);
	ret->KillTag(Tphong);
	if (!op->CopyTagsTo(ret, true, false, false, nullptr))
		goto error;

	ret->SetName(op->GetName());

	if (hh->GetBuildFlags() & BUILDFLAGS_ISOPARM)
	{
		lop = GenerateIsoLathe(cpadr, cpcnt, seg);
		if (!lop)
			goto error;
		SetAxis(lop, axis);
		ret->SetIsoparm(lop);
	}

	DeleteMem(cpadr);
	return ret;

error:
	DeleteMem(cpadr);
	blDelete(ret);
	return nullptr;
}

static Vector SwapPoint(const Vector& p, Int32 axis)
{
	switch (axis)
	{
		case 0: return Vector(p.y, -p.x, p.z); break;
		case 1: return Vector(-p.y, p.x, p.z); break;
		case 3: return Vector(-p.x, -p.y, p.z); break;
		case 4: return Vector(p.x, -p.z, p.y); break;
		case 5: return Vector(p.x, p.z, -p.y); break;
	}
	return p;
}

Int32 RoundedTube::GetHandleCount(BaseObject* op)
{
	return 5;
}
void RoundedTube::GetHandle(BaseObject* op, Int32 id, HandleInfo& info)
{
	BaseContainer* data = op->GetDataInstance();

	Float	rad = data->GetFloat(TUBEOBJECT_RAD);
	Float	iradx = data->GetFloat(TUBEOBJECT_IRADX);
	Float	irady = data->GetFloat(TUBEOBJECT_IRADY);
	Float	rrad	= data->GetFloat(TUBEOBJECT_ROUNDRAD);
	Int32	axis	= data->GetInt32(PRIM_AXIS);

	switch (id)
	{
		case 0: info.position = Vector(rad, 0.0, 0.0); info.direction = Vector(1.0, 0.0, 0.0); break;
		case 1: info.position = Vector(rad + iradx, 0.0, 0.0); info.direction = Vector(1.0, 0.0, 0.0); break;
		case 2: info.position = Vector(rad, irady, 0.0); info.direction = Vector(0.0, 1.0, 0.0); break;
		case 3: info.position = Vector(rad + iradx, irady - rrad, 0.0); info.direction = Vector(0.0, -1.0, 0.0); break;
		case 4: info.position = Vector(rad + iradx - rrad, irady, 0.0); info.direction = Vector(-1.0, 0.0, 0.0); break;
	}

	info.type = HANDLECONSTRAINTTYPE_LINEAR;
	info.position	 = SwapPoint(info.position, axis);
	info.direction = SwapPoint(info.direction, axis);
}

void RoundedTube::SetHandle(BaseObject* op, Int32 i, Vector p, const HandleInfo& info)
{
	BaseContainer* data = op->GetDataInstance();
	if (!data)
		return;

	HandleInfo inf;
	GetHandle(op, i, inf);

	Float val = Dot(p - inf.position, info.direction);

	switch (i)
	{
		case 0: data->SetFloat(TUBEOBJECT_RAD, ClampValue(data->GetFloat(TUBEOBJECT_RAD) + val, data->GetFloat(TUBEOBJECT_IRADX), (Float) MAXRANGE)); break;
		case 1: data->SetFloat(TUBEOBJECT_IRADX, ClampValue(data->GetFloat(TUBEOBJECT_IRADX) + val, data->GetFloat(TUBEOBJECT_ROUNDRAD), data->GetFloat(TUBEOBJECT_RAD))); break;
		case 2: data->SetFloat(TUBEOBJECT_IRADY, ClampValue(data->GetFloat(TUBEOBJECT_IRADY) + val, data->GetFloat(TUBEOBJECT_ROUNDRAD), (Float) MAXRANGE)); break;
		case 3:
		case 4: data->SetFloat(TUBEOBJECT_ROUNDRAD, ClampValue(data->GetFloat(TUBEOBJECT_ROUNDRAD) + val, (Float) 0.0, FMin(data->GetFloat(TUBEOBJECT_IRADX), data->GetFloat(TUBEOBJECT_IRADY)))); break;
		default: break;
	}
}

DRAWRESULT RoundedTube::Draw(BaseObject* op, DRAWPASS drawpass, BaseDraw* bd, BaseDrawHelp* bh)
{
	if (drawpass != DRAWPASS_HANDLES)
		return DRAWRESULT_SKIP;

	BaseContainer* data = op->GetDataInstance();

	Int32 i;
	Float	rad = data->GetFloat(TUBEOBJECT_RAD);
	Float	iradx = data->GetFloat(TUBEOBJECT_IRADX);
	Float	irady = data->GetFloat(TUBEOBJECT_IRADY);
	Int32	axis	= data->GetInt32(PRIM_AXIS);

	bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));

	HandleInfo info;
	Int32			 hitid = op->GetHighlightHandle(bd);
	bd->SetMatrix_Matrix(op, bh->GetMg());

	for (i = GetHandleCount(op) - 1; i >= 0; --i)
	{
		GetHandle(op, i, info);

		if (i == hitid)
			bd->SetPen(GetViewColor(VIEWCOLOR_SELECTION_PREVIEW));
		else
			bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
		bd->DrawHandle(info.position, DRAWHANDLE_BIG, 0);

		//Draw lines to the handles
		bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
		switch (i)
		{
			case 0:
			{
				HandleInfo p2;
				GetHandle(op, 1, p2);
				bd->DrawLine(info.position, p2.position, 0);
				GetHandle(op, 2, p2);
				bd->DrawLine(info.position, p2.position, 0);
				break;
			}
			case 3:
				bd->DrawLine(info.position, SwapPoint(Vector(rad + iradx, irady, 0.0), axis), 0);
				break;
			case 4:
				bd->DrawLine(info.position, SwapPoint(Vector(rad + iradx, irady, 0.0), axis), 0);
				break;
			default: break;
		}
	}

	return DRAWRESULT_OK;
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_ROUNDEDTUBEOBJECT 1001157

Bool RegisterRoundedTube(void)
{
	return RegisterObjectPlugin(ID_ROUNDEDTUBEOBJECT, GeLoadString(IDS_ROUNDED_TUBE), OBJECT_GENERATOR, RoundedTube::Alloc, "Oroundedtube", AutoBitmap("roundedtube.tif"), 0);
}
