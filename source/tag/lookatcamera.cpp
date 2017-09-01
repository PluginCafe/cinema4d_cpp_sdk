// "look at editor camera" expression example

#include "c4d.h"
#include "c4d_symbols.h"
#include "tlookatcameraexp.h"
#include "main.h"

#include "lib_description.h"
#include "customgui_priority.h"

class LookAtCamera : public TagData
{
public:
	virtual Bool Init(GeListNode* node);

	virtual EXECUTIONRESULT Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags);
	virtual Bool GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags);

	static NodeData* Alloc() { return NewObjClear(LookAtCamera); }
};

Bool LookAtCamera::Init(GeListNode* node)
{
	BaseTag*			 tag	= (BaseTag*)node;
	BaseContainer* data = tag->GetDataInstance();

	data->SetBool(LOOKATCAMERAEXP_PITCH, true);
	GeData d;
	if (node->GetParameter(DescLevel(EXPRESSION_PRIORITY), d, DESCFLAGS_GET_0))
	{
		PriorityData* pd = (PriorityData*)d.GetCustomDataType(CUSTOMGUI_PRIORITY_DATA);
		if (pd)
			pd->SetPriorityValue(PRIORITYVALUE_CAMERADEPENDENT, GeData(true));
		node->SetParameter(DescLevel(EXPRESSION_PRIORITY), d, DESCFLAGS_SET_0);
	}

	return true;
}


Bool LookAtCamera::GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags)
{
	if (!description->LoadDescription(1001165))
		return false;
	const DescID* singleid = description->GetSingleDescID();

	DescID cid = DescLevel(6001, DTYPE_GROUP, 0);
	if (!singleid || cid.IsPartOf(*singleid, nullptr))	// important to check for speedup c4d!
	{
		BaseContainer maingroup = GetCustomDataTypeDefault(DTYPE_GROUP);
		maingroup.SetString(DESC_NAME, "Main Group");
		if (!description->SetParameter(cid, maingroup, DescLevel(0)))
			return true;
	}

	cid = DescLevel(6002, DTYPE_GROUP, 0);
	if (!singleid || cid.IsPartOf(*singleid, nullptr))	// important to check for speedup c4d!
	{
		BaseContainer subgroup = GetCustomDataTypeDefault(DTYPE_GROUP);
		subgroup.SetString(DESC_NAME, "Sub Group");
		if (!description->SetParameter(cid, subgroup, DescLevel(6001)))
			return true;
	}

	cid = DescLevel(6003, DTYPE_BOOL, 0);
	if (!singleid || cid.IsPartOf(*singleid, nullptr))	// important to check for speedup c4d!
	{
		BaseContainer locked = GetCustomDataTypeDefault(DTYPE_BOOL);
		locked.SetString(DESC_NAME, "SPECIAL Locked");
		locked.SetBool(DESC_DEFAULT, true);
		if (!description->SetParameter(cid, locked, DescLevel(6002)))
			return true;
	}

	cid = DescLevel(6004, DTYPE_BOOL, 0);
	if (!singleid || cid.IsPartOf(*singleid, nullptr))	// important to check for speedup c4d!
	{
		BaseContainer locked = GetCustomDataTypeDefault(DTYPE_BOOL);
		locked = GetCustomDataTypeDefault(DTYPE_LONG);
		locked.SetString(DESC_NAME, "SPECIAL Long");
		locked.SetBool(DESC_DEFAULT, true);
		if (!description->SetParameter(cid, locked, DescLevel(6002)))
			return true;
	}

	flags |= DESCFLAGS_DESC_LOADED;

	return true;
}


EXECUTIONRESULT LookAtCamera::Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags)
{
	BaseDraw* bd = doc->GetRenderBaseDraw();
	if (!bd)
		return EXECUTIONRESULT_OK;
	BaseObject* cp = bd->GetSceneCamera(doc);
	if (!cp)
		cp = bd->GetEditorCamera();
	if (!cp)
		return EXECUTIONRESULT_OK;

	Vector local = (~(op->GetUpMg() * op->GetFrozenMln())) * cp->GetMg().off - op->GetRelPos();
	Vector hpb = VectorToHPB(local);

	if (!tag->GetData().GetBool(LOOKATCAMERAEXP_PITCH))
		hpb.y = op->GetRelRot().y;

	hpb.z = op->GetRelRot().z;

	op->SetRelRot(hpb);

	return EXECUTIONRESULT_OK;
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_LOOKATCAMERATAG 1001165

Bool RegisterLookAtCamera()
{
	return RegisterTagPlugin(ID_LOOKATCAMERATAG, GeLoadString(IDS_LOOKATCAMERA), TAG_EXPRESSION | TAG_VISIBLE, LookAtCamera::Alloc, "Tlookatcameraexp", AutoBitmap("lookatcamera.tif"), 0);
}
