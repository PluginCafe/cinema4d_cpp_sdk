/*
This is an example brush that will allow you to draw splines on a PolygonObject.
The brush will work with all the symmetry options.
*/

#include "lib_sculptbrush.h"
#include "toolsculptsplinebrush.h"
#include "c4d_symbols.h"
#include "c4d.h"
#include "lib_modeling.h"
#include "ge_dynamicarray.h"
#include "main.h"

#include "lib_sculpt.h"														//This is only required for the helper IsObjectEnabled(BaseObject *op) function.

#define SCULPTBRUSH_SDK_EXAMPLE_SPLINE 1032294 	//You MUST get your own ID from www.plugincafe.com

struct SplineStrokeData
{
	Bool	 firstHit;
	Bool	 firstPointDone;
	Vector hitPoint;

	Int32	 strokeId;

	SplineObject *_spline;

	SplineStrokeData()
	{
		_spline = nullptr;
		strokeId = -1;
		firstHit = false;
		firstPointDone = false;
	}
};

class SculptSplineBrush : public SculptBrushToolData
{
public:
	SculptSplineBrush(SculptBrushParams* pParams) : SculptBrushToolData(pParams) { }
	~SculptSplineBrush() { }

	virtual Int32 GetToolPluginId();
	virtual const String GetResourceSymbol();

	virtual void StartStroke(Int32 strokeCount, const BaseContainer& data);
	virtual void EndStroke();

	virtual void MouseData(Int32 strokeInstanceID, const BaseContainer &brushData, const SculptMouseData &md);

	virtual void PostInitDefaultSettings(BaseDocument* doc, BaseContainer& data);

	virtual void StartStrokeInstance(Int32 strokeInstanceID);
	static Bool MovePointsFunc(BrushDabData* dab);


public:
	GeDynamicArray<SplineStrokeData> _SplineStrokeData;
	BaseDocument*							 _doc;
	Int32											 _strokeCounter;
	SplineObject*							 _spline;
};

Int32 SculptSplineBrush::GetToolPluginId()
{
	return SCULPTBRUSH_SDK_EXAMPLE_SPLINE;
}

const String SculptSplineBrush::GetResourceSymbol()
{
	//Return the name of the .res file, in the res/description and res/strings folder, for this tool.
	return String("toolsculptsplinebrush");
}

void SculptSplineBrush::PostInitDefaultSettings(BaseDocument* doc, BaseContainer& data)
{
	//When the brush is first initialized we will turn on stamp spacing and set its value to 25%. Otherwise the spline points will be too close together.
	data.SetBool(MDATA_SCULPTBRUSH_SETTINGS_STAMPSPACING, true);
	data.SetFloat(MDATA_SCULPTBRUSH_SETTINGS_STAMPSPACING_VALUE, 25);
}

void SculptSplineBrush::StartStroke(Int32 strokeCount, const BaseContainer& data)
{
	_strokeCounter = 0;

	_SplineStrokeData.ReSize(strokeCount);
	//At the start of the brush stroke we get the active document and call StartUndo on it since we are handling Undo ourselves.

	_doc = GetActiveDocument();
	_doc->StartUndo();

		//Create a null object to store all the splines under.
	BaseObject *nullObject = BaseObject::Alloc(Onull);
	if(!nullObject) return;

	//Add the null object to the document.
	_doc->InsertObject(nullObject, nullptr, nullptr);

	//Add an undo event for this null object.
	_doc->AddUndo(UNDOTYPE_NEW, nullObject);

	for(Int i=0; i < strokeCount; i++)
	{
		//Create a new spline object for every stroke and add it under the null object
		_SplineStrokeData[i]._spline = SplineObject::Alloc(1, SPLINETYPE_CUBIC);
		if (!_SplineStrokeData[i]._spline) break;

		//Add the spline object to the document.
		_doc->InsertObject(_SplineStrokeData[i]._spline, nullObject, nullptr);

		//Add an undo event for this null object.
		_doc->AddUndo(UNDOTYPE_NEW, _SplineStrokeData[i]._spline);
	}
}

void SculptSplineBrush::MouseData(Int32 strokeInstanceID, const BaseContainer &brushData, const SculptMouseData &md)
{
	SplineStrokeData* pData = nullptr;
	Int				count = _SplineStrokeData.GetCount();
	Int				i;
	for (i = 0; i < count; i++)
	{
		if (_SplineStrokeData[i].strokeId == strokeInstanceID)
		{
			pData = &_SplineStrokeData[i];
		}
	}

	if (!pData || !pData->_spline)
	{
		return;
	}

	Int32 pcnt = pData->_spline->GetPointCount();
	if(pcnt > 0)
	{
		Vector *pPoints = pData->_spline->GetPointW();
		pPoints[pcnt-1] = md.hitPoint + (Vector)md.normal;
		pData->_spline->Message(MSG_POINTS_CHANGED);
		pData->_spline->Message(MSG_UPDATE);

		DRAWFLAGS common = DRAWFLAGS_ONLY_ACTIVE_VIEW|DRAWFLAGS_NO_ANIMATION|DRAWFLAGS_NO_THREAD|DRAWFLAGS_NO_REDUCTION|DRAWFLAGS_NO_EXPRESSIONS;
		DrawViews(common|DRAWFLAGS_PRIVATE_NO_WAIT_GL_FINISHED);
	}
}

void SculptSplineBrush::EndStroke()
{
	_SplineStrokeData.FreeArray();
	//When the stroke ends (which happens on mouse up) we end the Undo for this brush stroke.
	_doc->EndUndo();
	_doc = nullptr;
}

void SculptSplineBrush::StartStrokeInstance(Int32 strokeInstanceID)
{
	if (_strokeCounter < _SplineStrokeData.GetCount())
	{
		_SplineStrokeData[_strokeCounter++].strokeId = strokeInstanceID;
	}
}

//This method gets called for every brush dab for every symmetrical brush.
Bool SculptSplineBrush::MovePointsFunc(BrushDabData* dab)
{
	//Since we have enabled brush access via the call to EnableBrushAccess(true) we can now access the brush directly from this static MovePointFunc method.
	//This lets us access the member variables of the brush.
	SculptSplineBrush* pBrush = (SculptSplineBrush*)dab->GetBrush();
	if (!pBrush)
		return false;

	//Get the correct SplineStrokeData for this dab
	SplineStrokeData* pData = nullptr;
	Int32				count = (Int32)pBrush->_SplineStrokeData.GetCount();
	Int32				i;
	for (i = 0; i < count; i++)
	{
		if (pBrush->_SplineStrokeData[i].strokeId == dab->GetStrokeInstanceID())
		{
			pData = &pBrush->_SplineStrokeData[i];
		}
	}

	if (!pData || !pData->_spline)
	{
		return false;
	}

	if (!pData->firstHit)
	{
		pData->firstHit = true;
		pData->hitPoint = dab->GetHitPoint() + dab->GetNormal();

		SplineObject *pSpline = pData->_spline;
		Vector *pPoints = pSpline->GetPointW();
		if(pPoints)
		{
			pPoints[0] = pData->hitPoint;
		}
		return true;
	}

	Int32 pcnt = pData->_spline->GetPointCount();
	Int32	  *map = (pcnt > 0) ? NewMemClear(Int32, pcnt+1) : nullptr; 
	if (!map && pcnt) 
	{
		return false;
	}

	for (i = 0; i < pcnt; i++)
	{
		map[i] = i;
	}

	VariableChanged vc;
	vc.old_cnt = pcnt;
	vc.new_cnt = pcnt+1;
	vc.map = map;
	if (!pData->_spline->Message(MSG_POINTS_CHANGED, &vc)) 
		return false;

	Vector *pPoints = pData->_spline->GetPointW();
	pPoints[pcnt] = dab->GetHitPoint() + dab->GetNormal();

	pData->_spline->Message(MSG_UPDATE);

	DeleteMem(map);
	return true;
}

Bool RegisterSculptBrushSpline()
{
	SculptBrushParams* pParams = SculptBrushParams::Alloc();
	if (!pParams)
		return false;

	//Since we are using StartStroke/EndStroke calls then we need to set this to true.
	pParams->EnableBrushAccess(true);
	pParams->EnableMouseData(true);

	pParams->EnableStencil(false);
	pParams->EnableStamp(false);

	pParams->EnablePressureHUD(false);

	//We want to handle undo/redo ourselves so we tell the Sculpting System that should not do anything with its Undo System.
	pParams->SetUndoType(SCULPTBRUSHDATATYPE_NONE);

	//Set the MovePointFunc to call for each dab.
	pParams->SetMovePointFunc(&SculptSplineBrush::MovePointsFunc);

	//Register the tool with Cinema4D.
	return RegisterToolPlugin(SCULPTBRUSH_SDK_EXAMPLE_SPLINE, GeLoadString(IDS_SCULPTBRUSH_SPLINETOOL), PLUGINFLAG_TOOL_SCULPTBRUSH | PLUGINFLAG_TOOL_NO_OBJECTOUTLINE, nullptr, GeLoadString(IDS_SCULPTBRUSH_SPLINETOOL), NewObjClear(SculptSplineBrush, pParams));
}
