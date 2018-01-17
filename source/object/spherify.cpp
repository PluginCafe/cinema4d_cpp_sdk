// deformer object example

#include "c4d.h"
#include "c4d_symbols.h"
#include "main.h"
#include "ospherifydeformer.h"

#define HANDLE_CNT 2

class Spherify : public ObjectData
{
public:
	virtual Bool Init(GeListNode* node);

	virtual Bool Message				(GeListNode* node, Int32 type, void* data);
	virtual void GetDimension		(BaseObject* op, Vector* mp, Vector* rad);
	virtual DRAWRESULT Draw			(BaseObject* op, DRAWPASS type, BaseDraw* bd, BaseDrawHelp* bh);
	virtual void GetHandle(BaseObject* op, Int32 i, HandleInfo& info);
	virtual Int32 DetectHandle		(BaseObject* op, BaseDraw* bd, Int32 x, Int32 y, QUALIFIER qualifier);
	virtual Bool MoveHandle			(BaseObject* op, BaseObject* undo, const Vector& mouse_pos, Int32 hit_id, QUALIFIER qualifier, BaseDraw* bd);
	virtual Bool ModifyObject   (BaseObject* op, BaseDocument* doc, BaseObject* mod, const Matrix& op_mg, const Matrix& mod_mg, Float lod, Int32 flags, BaseThread* thread);

	static NodeData* Alloc(void) { return NewObjClear(Spherify); }
};

Bool Spherify::Message(GeListNode* node, Int32 type, void* data)
{
	if (type == MSG_MENUPREPARE)
	{
		((BaseObject*)node)->SetDeformMode(true);
	}
	return true;
}

Bool Spherify::ModifyObject(BaseObject* mod, BaseDocument* doc, BaseObject* op, const Matrix& op_mg, const Matrix& mod_mg, Float lod, Int32 flags, BaseThread* thread)
{
	BaseContainer* data = mod->GetDataInstance();

	Vector	 p, *padr = nullptr;
	Matrix	 m, im;
	Int32		 i, pcnt;
	Float		 rad = data->GetFloat(SPHERIFYDEFORMER_RADIUS), strength = data->GetFloat(SPHERIFYDEFORMER_STRENGTH);
	Float		 s;
	Float32* weight = nullptr;

	if (!op->IsInstanceOf(Opoint))
		return true;

	padr = ToPoint(op)->GetPointW();
	pcnt = ToPoint(op)->GetPointCount();
	if (!pcnt)
		return true;

	weight = ToPoint(op)->CalcVertexMap(mod);

	m	 = (~mod_mg) * op_mg;	// op  ->  world  ->  modifier
	im = ~m;

	for (i = 0; i < pcnt; i++)
	{
		if (thread && !(i & 63) && thread->TestBreak())
			break;
		p = m * padr[i];
		s = strength;
		if (weight)
			s *= weight[i];
		p = s * (!p * rad) + (1.0 - s) * p;
		padr[i] = im * p;
	}

	DeleteMem(weight);
	op->Message(MSG_UPDATE);

	return true;
}

void Spherify::GetDimension(BaseObject* op, Vector* mp, Vector* rad)
{
	BaseContainer* data = op->GetDataInstance();
	*mp	 = Vector(0.0);
	*rad = Vector(data->GetFloat(SPHERIFYDEFORMER_RADIUS));
}

DRAWRESULT Spherify::Draw(BaseObject* op, DRAWPASS drawpass, BaseDraw* bd, BaseDrawHelp* bh)
{
	if (drawpass == DRAWPASS_OBJECT)
	{
		BaseContainer* data = op->GetDataInstance();
		Float	 rad = data->GetFloat(SPHERIFYDEFORMER_RADIUS);
		Vector h;
		Matrix m = bh->GetMg();

		m.v1 *= rad;
		m.v2 *= rad;
		m.v3 *= rad;

		bd->SetMatrix_Matrix(nullptr, Matrix());
		bd->SetPen(bd->GetObjectColor(bh, op));
		bd->DrawCircle(m);
		h = m.v2; m.v2 = m.v3; m.v3 = h;
		bd->DrawCircle(m);
		h = m.v1; m.v1 = m.v3; m.v3 = h;
		bd->DrawCircle(m);
	}
	else if (drawpass == DRAWPASS_HANDLES)
	{
		Int32			 i;
		Int32			 hitid = op->GetHighlightHandle(bd);
		HandleInfo info;

		bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
		bd->SetMatrix_Matrix(op, bh->GetMg());
		for (i = 0; i < HANDLE_CNT; i++)
		{
			GetHandle(op, i, info);
			if (hitid == i)
				bd->SetPen(GetViewColor(VIEWCOLOR_SELECTION_PREVIEW));
			else
				bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
			bd->DrawHandle(info.position, DRAWHANDLE_BIG, 0);
		}

		GetHandle(op, 1, info);
		bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
		bd->DrawLine(info.position, Vector(0.0), 0);
	}
	return DRAWRESULT_OK;
}

void Spherify::GetHandle(BaseObject* op, Int32 i, HandleInfo& info)
{
	BaseContainer* data = op->GetDataInstance();
	if (!data)
		return;

	switch (i)
	{
		case 0:
			info.position.x	 = data->GetFloat(SPHERIFYDEFORMER_RADIUS);
			info.direction.x = 1.0;
			info.type = HANDLECONSTRAINTTYPE_LINEAR;
			break;
		case 1:
			info.position.x	 = data->GetFloat(SPHERIFYDEFORMER_STRENGTH) * 1000.0;
			info.direction.x = 1.0;
			info.type = HANDLECONSTRAINTTYPE_LINEAR;
		default: break;
	}
}

Int32 Spherify::DetectHandle(BaseObject* op, BaseDraw* bd, Int32 x, Int32 y, QUALIFIER qualifier)
{
	if (qualifier & QUALIFIER_CTRL)
		return NOTOK;

	HandleInfo info;
	Matrix		 mg = op->GetMg();
	Int32			 i, ret = NOTOK;
	Vector		 p;

	for (i = 0; i < HANDLE_CNT; i++)
	{
		GetHandle(op, i, info);
		if (bd->PointInRange(mg * info.position, x, y))
		{
			ret = i;
			if (!(qualifier & QUALIFIER_SHIFT))
				break;
		}
	}
	return ret;
}

Bool Spherify::MoveHandle(BaseObject* op, BaseObject* undo, const Vector& mouse_pos, Int32 hit_id, QUALIFIER qualifier, BaseDraw* bd)
{
	BaseContainer* dst = op->GetDataInstance();

	HandleInfo info;

	Float val = mouse_pos.x;
	GetHandle(op, hit_id, info);

	if (bd)
	{
		Matrix mg	 = op->GetUpMg() * undo->GetMl();
		Vector pos = bd->ProjectPointOnLine(mg * info.position, mg.TransformVector(info.direction), mouse_pos.x, mouse_pos.y);
		val = Dot(~mg * pos, info.direction);
	}

	switch (hit_id)
	{
		case 0:
			dst->SetFloat(SPHERIFYDEFORMER_RADIUS, ClampValue(val, (Float) 0.0, (Float) MAXRANGE));
			break;

		case 1:
			dst->SetFloat(SPHERIFYDEFORMER_STRENGTH, Clamp01(val * 0.001));
			break;

		default:
			break;
	}
	return true;
}

Bool Spherify::Init(GeListNode* node)
{
	BaseObject*		 op = (BaseObject*)node;
	BaseContainer* data = op->GetDataInstance();

	data->SetFloat(SPHERIFYDEFORMER_RADIUS, 200.0);
	data->SetFloat(SPHERIFYDEFORMER_STRENGTH, 0.5);

	return true;
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_SPHERIFYOBJECT 1001158

Bool RegisterSpherify(void)
{
	return RegisterObjectPlugin(ID_SPHERIFYOBJECT, GeLoadString(IDS_SPHERIZE), OBJECT_MODIFIER, Spherify::Alloc, "Ospherifydeformer", AutoBitmap("spherify.tif"), 0);
}
