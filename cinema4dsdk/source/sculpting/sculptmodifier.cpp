#include "c4d.h"
#include "lib_sculptbrush.h"
#include "c4d_symbols.h"
#include "main.h"

#define ID_SCULPT_BRUSH_EXAMPLE_MODIFIER 1030975

class ExampleSculptModifier : public SculptBrushModifierData
{
public:
	INSTANCEOF(ExampleSculptModifier, SculptBrushModifierData)
public:

	static NodeData *Alloc()
	{
		return NewObjClear(ExampleSculptModifier);
	}

	virtual Bool ApplyModifier(BrushDabData *dab, const BaseContainer &modifierData)
	{
		if (dab->GetBrushStrength() == 0) return true;

		Int32 a;

		Bool usePreview = dab->IsPreviewDab();

		Float brushRadius = dab->GetBrushRadius();
		if (brushRadius <= 0) return false;

		PolygonObject *polyObj = dab->GetPolygonObject();
		if (!polyObj) return false;

		//Start Gravity Code
		Float val = dab->GetBrushStrength();
		Vector gravity = Vector(0, -1, 0) * val;

		Int32 count = dab->GetPointCount();
		const BrushPointData *pPointData = dab->GetPointData();
		for (a = 0; a < count; ++a)
		{
			Int32 pointIndex = pPointData[a].pointIndex;
			Float fallOff = dab->GetBrushFalloff(a);
			if (fallOff == 0) continue;
			Vector res = fallOff * gravity;
			if (!usePreview)
				dab->OffsetPoint(pointIndex, res, nullptr);
			else
				dab->OffsetPreviewPoint(pointIndex, res);
		}
		return true;
	}
};

Bool RegisterSculptModifiers()
{
	//This modifier is not useful for the pull brush or the grab brush. So we will filter them out so they do not show up on these brushes.
	maxon::BaseArray<Int32> filters;
	filters.Append(SCULPTBRUSHID_PULL);
	filters.Append(SCULPTBRUSHID_GRAB);
	if (!RegisterBrushModifier(ID_SCULPT_BRUSH_EXAMPLE_MODIFIER, GeLoadString(IDS_SCULPT_BRUSH_EXAMPLE_MODIFIER),  ExampleSculptModifier::Alloc, SCULPTBRUSHMODE_NORMAL, SCULPTBRUSHDATATYPE_POINT, "bmexample", false, &filters)) return false;
	return true;
}
