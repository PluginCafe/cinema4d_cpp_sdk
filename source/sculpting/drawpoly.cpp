#include "c4d.h"
#include "toolsculpting.h"
#include "c4d_symbols.h"
#include "lib_sculpt.h"
#include "lib_modeling.h"
#include "lib_ngon.h"
#include "toolsculptdrawpoly.h"
#include "main.h"

#define ID_SCULPT_DRAWPOLY_TOOL	1027981

class SculptDrawPolyTool : public DescriptionToolData
{
public:
	SculptDrawPolyTool();
	virtual ~SculptDrawPolyTool();

private:
	virtual Int32 GetToolPluginId() { return ID_SCULPT_DRAWPOLY_TOOL; }
	virtual const String GetResourceSymbol() { return String("ToolSculptDrawPoly"); }
	virtual Bool InitTool(BaseDocument* pDoc, BaseContainer& data, BaseThread* bt);
	virtual void FreeTool(BaseDocument* pDoc, BaseContainer& data);
	virtual void InitDefaultSettings(BaseDocument* doc, BaseContainer& data);
	virtual Bool GetCursorInfo(BaseDocument* pDoc, BaseContainer& data, BaseDraw* pDraw, Float x, Float y, BaseContainer& bc);
	virtual Bool MouseInput(BaseDocument* pDoc, BaseContainer& data, BaseDraw* pDraw, EditorWindow* win, const BaseContainer& msg);
	virtual TOOLDRAW Draw(BaseDocument* doc, BaseContainer& data, BaseDraw* bd, BaseDrawHelp* bh, BaseThread* bt, TOOLDRAWFLAGS flags);

	SculptObject* m_pLastObject;
	Float					m_rLastMouseX, m_rLastMouseY;
};

SculptDrawPolyTool::SculptDrawPolyTool()
{
}

SculptDrawPolyTool::~SculptDrawPolyTool()
{
}

Bool SculptDrawPolyTool::InitTool(BaseDocument* pDoc, BaseContainer& data, BaseThread* bt)
{
	if (!DescriptionToolData::InitTool(pDoc, data, bt))
		return false;

	m_rLastMouseX = m_rLastMouseY = -1.0f;
	m_pLastObject = nullptr;
	return true;
}

void SculptDrawPolyTool::FreeTool(BaseDocument* pDoc, BaseContainer& data)
{
	m_pLastObject = nullptr;
	DescriptionToolData::FreeTool(pDoc, data);
}

void SculptDrawPolyTool::InitDefaultSettings(BaseDocument* doc, BaseContainer& data)
{
	data.SetFloat(SCULPTDRAWPOLY_POLYGONSIZE, 5.0);
	DescriptionToolData::InitDefaultSettings(doc, data);
}

Bool SculptDrawPolyTool::GetCursorInfo(BaseDocument* pDoc, BaseContainer& data, BaseDraw* pDraw, Float x, Float y, BaseContainer& bc)
{
	if (bc.GetId() != BFM_CURSORINFO_REMOVE)
	{
		if (!pDoc)
			return true;

		if (pDraw != pDoc->GetActiveBaseDraw())
			return true;

		SculptObject* pSculpt = GetSelectedSculptObject(pDoc);
		if (!pSculpt)
		{
			m_pLastObject = nullptr;
			return true;
		}

		if (pSculpt != m_pLastObject)
		{
			m_pLastObject = pSculpt;

			// Unfreeze the object so that we can use collision detection
			if (m_pLastObject->IsFrozen())
			{
				m_pLastObject->SetFrozen(false);
			}

			// Request a collision update
			m_pLastObject->NeedCollisionUpdate();
		}

		if (m_pLastObject)
		{
			// Update the collision data
			m_pLastObject->UpdateCollision();
		}
	}
	else
	{
		SpecialEventAdd(EVMSG_UPDATEHIGHLIGHT);
	}

	return true;
}

TOOLDRAW SculptDrawPolyTool::Draw(BaseDocument* doc, BaseContainer& data, BaseDraw* bd, BaseDrawHelp* bh, BaseThread* bt, TOOLDRAWFLAGS flags)
{
	return TOOLDRAW_HIGHLIGHTS;
}


Bool SculptDrawPolyTool::MouseInput(BaseDocument* pDoc, BaseContainer& data, BaseDraw* pDraw, EditorWindow* win, const BaseContainer& msg)
{
	if (!pDoc)
		return true;
	if (pDraw != pDoc->GetActiveBaseDraw())
		return true;

	if (!m_pLastObject)
		return true;

	Float distance = data.GetFloat(SCULPTDRAWPOLY_POLYGONSIZE);

	Int32 lMouseX = msg.GetInt32(BFM_INPUT_X);
	Int32 lMouseY = msg.GetInt32(BFM_INPUT_Y);
	Int32 lLeft, lTop, lRight, lBottom;
	pDraw->GetFrame(&lLeft, &lTop, &lRight, &lBottom);

	Float					rMouseX = (Float)lMouseX;
	Float					rMouseY = (Float)lMouseY;
	Float					dx, dy;
	Bool					bFirst = true;
	BaseContainer bcDevice;

	PolygonObject* poly = PolygonObject::Alloc(0, 0);
	if (!poly)
		return false;

	pDoc->StartUndo();

	pDoc->InsertObject(poly, nullptr, nullptr);
	pDoc->AddUndo(UNDOTYPE_NEW, poly);

	pDoc->EndUndo();

	poly->SetBit(BIT_ACTIVE);

	EventAdd();

	AutoAlloc<Modeling> mod;
	if (!mod || !mod->InitObject(poly))
		return false;

	Vector p1, p2, p3, p4;
	Int32	 index1, index2, index3, index4;
	index1 = index2 = index3 = index4 = 0;

	Bool	 firstHit = false;
	Bool	 firstPointDone = false;
	Vector hitPoint;

	win->MouseDragStart(KEY_MLEFT, rMouseX, rMouseY, MOUSEDRAGFLAGS_DONTHIDEMOUSE);
	while (win->MouseDrag(&dx, &dy, &bcDevice) == MOUSEDRAGRESULT_CONTINUE)
	{
		if (!bFirst && dx == 0.0f && dy == 0.0f)
			continue;

		bFirst = false;

		rMouseX += dx;
		rMouseY += dy;

		if (!firstHit)
		{
			SculptHitData hitData;
			if (m_pLastObject->Hit(pDraw, rMouseX, rMouseY, false, hitData))
			{
				firstHit = true;
				hitPoint = hitData.localHitPoint + (Vector64)hitData.localHitNormal;
				continue;
			}
		}

		if (firstHit)
		{
			SculptHitData hitData;
			if (m_pLastObject->Hit(pDraw, rMouseX, rMouseY, false, hitData))
			{
				Vector normal = (Vector64)hitData.localHitNormal;
				Vector newP = hitData.localHitPoint + (Vector64)hitData.localHitNormal;
				Vector diff = hitPoint - newP;
				Float	 len	= diff.GetLength();
				if (len > distance)
				{
					Vector cross = Cross(normal, diff.GetNormalized());
					Vector gap = cross * distance * 0.5;
					if (!firstPointDone)
					{
						firstPointDone = true;
						p1 = hitPoint - gap;
						p2 = hitPoint + gap;

						index1 = mod->AddPoint(poly, p1);
						index2 = mod->AddPoint(poly, p2);
					}

					p3 = newP + gap;
					p4 = newP - gap;

					index3 = mod->AddPoint(poly, p3);
					index4 = mod->AddPoint(poly, p4);

					Int32 padr[4] = { index1, index2, index3, index4 };
					Int32 i = mod->CreateNgon(poly, padr, 4, MODELING_SETNGON_FLAG_FIXEDQUADS);
					if (!i)
						return true;

					if (!mod->Commit(poly, MODELING_COMMIT_UPDATE))
						return true;

					index1 = (-index4) - 1;
					index2 = (-index3) - 1;

					hitPoint = newP;
				}
			}
		}
		DrawViews(DRAWFLAGS_ONLY_ACTIVE_VIEW | DRAWFLAGS_NO_THREAD | DRAWFLAGS_NO_ANIMATION);
	}
	win->MouseDragEnd();

	DrawViews(DRAWFLAGS_ONLY_ACTIVE_VIEW | DRAWFLAGS_NO_THREAD | DRAWFLAGS_NO_ANIMATION);
	return true;
}

Bool RegisterSculptDrawPolyTool()
{
	return RegisterToolPlugin(ID_SCULPT_DRAWPOLY_TOOL, GeLoadString(IDS_SCULPTDRAWPOLY_TOOL), 0, nullptr, GeLoadString(IDS_SCULPTDRAWPOLY_TOOL), NewObjClear(SculptDrawPolyTool));
}
