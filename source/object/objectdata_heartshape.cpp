#include "c4d_baseobject.h"
#include "c4d_includes.h"
#include "c4d_objectdata.h"

// Includes from cinema4dsdk
#include "c4d_symbols.h"
#include "main.h"

// Local resources
#include "oheartshape.h"

/**A unique plugin ID. You must obtain this from http://www.plugincafe.com. Use this ID to create new instances of this object.*/
const static Int32 ID_SDKEXAMPLE_OBJECTDATA_HEARTSHAPE = 1038224;

namespace HeartShapeHelpers
{
	//------------------------------------------------------------------------------------------------
	/// Global function responsible to create an heart shape.
	/// @brief Global function responsible to create an heart shape.
	/// @param[out] splineObj 				Polygon object provided to store created geometry. @callerOwnsPointed{base object}
	/// @param[in] radiusValue 				Radius of the circle circumscribing the polygonal curve.
	/// @return												True if creation process succeeds.
	//------------------------------------------------------------------------------------------------
	const Bool CreateHeartShape(SplineObject& splineObj, const Float& radiusValue);
	const Bool CreateHeartShape(SplineObject& splineObj, const Float& radiusValue)
	{

		if (radiusValue == 0)
			return false;

		// Set the closed/open status of the SplineObject instance accessing its BaseContainer instance
		BaseContainer* splineObjBCPtr = splineObj.GetDataInstance();
		if (splineObjBCPtr)
			splineObjBCPtr->SetBool(SPLINEOBJECT_CLOSED, true);

		// Set the number of segments composing your spline.
		// NOTE:The number of segments represents the number of opened (or closed)
		//			curves which are included in the SplineObject returned
		const Int32 crvsCnt = 1;
		if (!splineObj.MakeVariableTag(Tsegment, crvsCnt))
			return false;

		// Access the array of the points representing the curve passing points.
		Vector* splinePntsPtr = splineObj.GetPointW();
		if (nullptr == splinePntsPtr || splineObj.GetPointCount() != 6)
			return false;

		// Access the array of the tangents (left and right) at  the curve passing points.
		// NOTE:The tangents naming refers the following scheme (vl)<--(point)-->(vr)
		Tangent* splineTgtsPtr = splineObj.GetTangentW();
		if (nullptr == splineTgtsPtr || splineObj.GetTangentCount() != 6)
			return false;

		// Set the control vertexes' position and tangents accordingly.
		splinePntsPtr[0] = Vector(0, radiusValue * 0.5, 0);
		splineTgtsPtr[0].vr = Vector(radiusValue * 0.05, radiusValue * 0.25, 0);
		splineTgtsPtr[0].vl = Vector(-radiusValue * 0.05, radiusValue * 0.25, 0);

		splinePntsPtr[1] = Vector(radiusValue * 0.5, radiusValue, 0);
		splineTgtsPtr[1].vl = Vector(-radiusValue * 0.15, 0, 0);
		splineTgtsPtr[1].vr = Vector(radiusValue * 0.2, 0, 0);

		splinePntsPtr[2] = Vector(radiusValue, radiusValue * 0.5, 0);
		splineTgtsPtr[2].vl = Vector(0, radiusValue * 0.2, 0);
		splineTgtsPtr[2].vr = Vector(0, -radiusValue * 0.4, 0);

		splinePntsPtr[3] = Vector(0, -radiusValue, 0);
		splineTgtsPtr[3].vl = Vector(radiusValue * 0.15, radiusValue * 0.4, 0);
		splineTgtsPtr[3].vr = Vector(-radiusValue * 0.15, radiusValue * 0.4, 0);

		splinePntsPtr[4] = Vector(-radiusValue, radiusValue * 0.5, 0);
		splineTgtsPtr[4].vl = Vector(0, -radiusValue * 0.4, 0);
		splineTgtsPtr[4].vr = Vector(0, radiusValue * 0.2, 0);

		splinePntsPtr[5] = Vector(-radiusValue * 0.5, radiusValue, 0);
		splineTgtsPtr[5].vl = Vector(-radiusValue * 0.2, 0, 0);
		splineTgtsPtr[5].vr = Vector(radiusValue * 0.15, 0, 0);

		// Access the curve's segments array.
		Segment* splineSegsPtr = splineObj.GetSegmentW();
		if (nullptr == splineSegsPtr)
			return false;

		// Set the closure status and the number of CVs for the only one segment existing.
		splineSegsPtr[0].closed = true;
		splineSegsPtr[0].cnt = 6;

		return true;
	}
}

//------------------------------------------------------------------------------------------------
/// ObjectData implementation generating a parameter-based heart-shaped closed contour.
///
/// The example, by overriding the GetCountour method, delivers a tool to generate heart-shaped 
/// contours by specifying the overall object radius.
//------------------------------------------------------------------------------------------------

/*! \brief A simple object generator creating a heart-shaped curve.
*/
class HeartShape : public ObjectData
{
	INSTANCEOF(HeartShape, ObjectData)

public:
	static NodeData* Alloc(void){ return NewObjClear(HeartShape); }
	virtual Bool Init(GeListNode* node);
	virtual void GetDimension(BaseObject* op, Vector* mp, Vector* rad);
	virtual SplineObject* GetContour(BaseObject* op, BaseDocument* doc, Float lod, BaseThread* bt);
};

/// @name ObjectData functions
/// @{
Bool HeartShape::Init(GeListNode* node)
{
	if (nullptr == node)
		return false;

	//	Retrieve the BaseContainer object belonging to the generator.
	BaseObject* baseObjectPtr = (BaseObject*)node;
	BaseContainer* objectDataPtr = baseObjectPtr->GetDataInstance();

	//	Fill the retrieve BaseContainer object with initial values.
	objectDataPtr->SetFloat(SDK_EXAMPLE_HEARTSHAPE_RADIUS, 200);

	return true;
}

void HeartShape::GetDimension(BaseObject* op, Vector* mp, Vector* rad)
{
	// Check the passed pointers.
	if (nullptr == op || nullptr == mp || nullptr == rad)
		return;

	//	Reset the barycenter position and the bbox radius vector.
	mp->SetZero();
	rad->SetZero();

	// Set the barycenter position to match the generator center.
	const Vector objGlobalOffset = op->GetMg().off;
	mp->x = objGlobalOffset.x;
	mp->y = objGlobalOffset.y;
	mp->z = objGlobalOffset.z;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* objectDataPtr = op->GetDataInstance();
	if (nullptr == objectDataPtr)
		return;

	// Set radius values accordingly to the bbox values stored during the init.
	rad->x = objectDataPtr->GetFloat(SDK_EXAMPLE_HEARTSHAPE_RADIUS);
	rad->y = objectDataPtr->GetFloat(SDK_EXAMPLE_HEARTSHAPE_RADIUS);
}

SplineObject* HeartShape::GetContour(BaseObject* op, BaseDocument* doc, Float lod, BaseThread* bt)
{
	// Check the passed pointer.
	if (nullptr == op)
		return nullptr;

	//	Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* objectDataPtr = op->GetDataInstance();

	//	Fill the retrieve BaseContainer object with initial values.
	const Float heartRadius = objectDataPtr->GetFloat(SDK_EXAMPLE_HEARTSHAPE_RADIUS);

	// Alloc a SplineObject and check it.
	// NOTE: in order to use and set tangents the spline type should be anything but linear
	SplineObject* splineObjPtr = SplineObject::Alloc(6, SPLINETYPE_BEZIER);
	if (nullptr == splineObjPtr)
		return nullptr;

	// Invoke the helper function to set the SplineObject object member accordingly.
	if (HeartShapeHelpers::CreateHeartShape(*splineObjPtr, heartRadius))
		return splineObjPtr;
	else
	{
		SplineObject::Free(splineObjPtr);
		return nullptr;
	}
}
/// @}

Bool RegisterHeartShape()
{
	String registeredName = GeLoadString(IDS_OBJECTDATA_HEARTSHAPE);
	if (!registeredName.Content() || registeredName == "StrNotFound")
		registeredName = "C++ SDK - Heart Shape Generator Example";

	return RegisterObjectPlugin(ID_SDKEXAMPLE_OBJECTDATA_HEARTSHAPE, registeredName, OBJECT_GENERATOR | OBJECT_ISSPLINE, HeartShape::Alloc, "oheartshape", AutoBitmap("heartshape.tif"), 0);
}