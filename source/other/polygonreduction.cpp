/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) MAXON Computer GmbH, all rights reserved            //
/////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// Polygon Reduction API example
////////////////////////////////////////////////////////////////////////////////////

#include "c4d.h"
#include "lib_polygonreduction.h"
#include "c4d_symbols.h"
#include "main.h"

static const Int32 ID_POLYREDUCTON_TEST = 1038949;

class PolygonReductionCommand : public CommandData
{
public:
	virtual Int32 GetState(BaseDocument* doc);
	virtual Bool Execute(BaseDocument* doc);
};


Int32 PolygonReductionCommand::GetState(BaseDocument* doc)
{
	BaseObject* activeObject = doc->GetActiveObject();
	if (!doc || !activeObject || !activeObject->IsInstanceOf(Opolygon))
		return 0;

	return CMD_ENABLED;
}

Bool PolygonReductionCommand::Execute(BaseDocument* doc)
{
	if (!doc)
		return false;

	// Get the selected object in the scene.
	BaseObject* activeObject = doc->GetActiveObject();
	if (!activeObject)
		return true;

	// Ensure the object  is a polygon object.
	// We need to make sure that only polygon objects are passed to the polygon reduction.
	if (!activeObject->IsInstanceOf(Opolygon))
		return true;

	doc->StartUndo();

	// Make a copy of the active object.
	BaseObject* activeClone = static_cast<BaseObject*>(activeObject->GetClone(COPYFLAGS_0, nullptr));
	if (!activeClone)
		return false;

	doc->InsertObject(activeClone, nullptr, nullptr);

	// Undo for new object must be called always after the object is inserted in the document.
	doc->AddUndo(UNDOTYPE_NEW, activeClone);

	// Hide the original object
	doc->AddUndo(UNDOTYPE_CHANGE, activeObject);
	activeObject->SetEditorMode(MODE_OFF);

	// Prepare the polygon reduction data and allocate the polygon reduction instance.
	// Notice that thread is nullptr in PolygonReductionData.  This means that the reduction will occur immediately and synchronously in the current thread.
	PolygonReductionData polyReductionData = PolygonReductionData(doc, ToPoly(activeClone), nullptr);
	AutoAlloc<PolygonReduction> polygonReduction;

	if (!polygonReduction)
	{
		BaseObject::Free(activeClone);
		doc->DoUndo();
		return false;
	}

	// Prepare the reduction cache.
	if (!polygonReduction->PreProcess(polyReductionData))
	{
		doc->DoUndo();
		return false;
	}

	// Set the desired reduction level.
	polygonReduction->SetReductionStrengthLevel(0.9);

	doc->EndUndo();
	activeClone->Message(MSG_UPDATE);
	EventAdd();

	return true;
}

Bool RegisterPolygonReductionTest()
{
	return RegisterCommandPlugin(ID_POLYREDUCTON_TEST,  String("Sync Polygon Reduction"), 0, nullptr, String("Apply Polygon Reduction to the selected Polygon Object"), NewObjClear(PolygonReductionCommand));
}
