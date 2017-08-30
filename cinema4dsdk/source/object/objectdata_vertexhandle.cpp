#include "c4d_basedraw.h"
#include "c4d_baseobject.h"
#include "c4d_objectdata.h"

// Includes from cinema4dsdk
#include "c4d_symbols.h"
#include "main.h"

// Local resources
#include "overtexhandle.h"

/**A unique plugin ID. You must obtain this from http://www.plugincafe.com. Use this ID to create new instances of this object.*/
static const Int32 ID_SDKEXAMPLE_OBJECTDATA_VERTEXHANDLE = 1038237;

//------------------------------------------------------------------------------------------------
/// Basic ObjectData implementation showing how to dynamically draw handles on a triangular
/// face and modify its shape by moving them on the plane. The position of the vertex can also be
/// changed by specifying the distance of the vertex from the origin along one of the three 
/// equiangular directions.
//------------------------------------------------------------------------------------------------
class VertexHandle : public ObjectData
{
	INSTANCEOF(VertexHandle, ObjectData)

public:
	static NodeData* Alloc(void){ return NewObjClear(VertexHandle); }
	virtual Bool Init(GeListNode *node);
	virtual Int32 GetHandleCount(BaseObject* op);
	virtual void GetHandle(BaseObject* op, Int32 i, HandleInfo& info);
	virtual void SetHandle(BaseObject* op, Int32 i, Vector p, const HandleInfo& info);
	virtual void GetDimension(BaseObject *op, Vector *mp, Vector *rad);
	virtual BaseObject* GetVirtualObjects(BaseObject *op, HierarchyHelp *hh);
	virtual DRAWRESULT Draw(BaseObject* op, DRAWPASS type, BaseDraw* bd, BaseDrawHelp* bh);

private:
	//------------------------------------------------------------------------------------------------
	/// Private method to calculate the angle between the x-axis and the line connecting the origin with the vertex referenced by the passed index.
	/// @brief Calculate the angle between the x-axis and the line between origin and i-th vertex.
	/// @param[in] vertexIdx 	The value of the vertex index.
	/// @return 							The value of the angle in radians.
	//------------------------------------------------------------------------------------------------
	const Float GetCurrentAngle(const Int32& vertexIdx);
};

/// @name ObjectData functions
/// @{
const Float VertexHandle::GetCurrentAngle(const Int32& vertexIdx)
{
	// Set the current angle delta to 120deg as on an equilateral triangle
	const Float angleDelta = -2 * PI / 3;
	// Set the angular offset to 90deg in order to align to the Y-axis
	const Float angleOffset = PI / 2;

	// Return the current angle based on the vertex index passed, the delta and the offset.
	return (angleDelta * vertexIdx + angleOffset);
}

Bool VertexHandle::Init(GeListNode* node)
{
	BaseObject* baseObjPtr = (BaseObject*)node;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* bcPtr = baseObjPtr->GetDataInstance();

	// Populate the Object Manager with the default values.
	bcPtr->SetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_A_DIST, 100);
	bcPtr->SetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_B_DIST, 100);
	bcPtr->SetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_C_DIST, 100);

	return true;
}

Int32 VertexHandle::GetHandleCount(BaseObject* op)
{
	// Set the overall number of handles defined for the ObjectData-derived class
	return 3;
}

void VertexHandle::GetHandle(BaseObject* op, Int32 i, HandleInfo& info)
{
	if (nullptr == op)
		return;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* bcPtr = op->GetDataInstance();

	if (nullptr == bcPtr)
		return;

	// Define a vector containing for each of its components the distance of each vertex of the 
	// triangle from the origin.
	const Vector ptsDistance(bcPtr->GetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_A_DIST), bcPtr->GetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_B_DIST), bcPtr->GetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_C_DIST));

	const Float currentAngle = GetCurrentAngle(i);

	// Define the position of the current i-th handle by multiplying its distance from the origin
	// by the corresponding trigonometric functions.
	info.position = Vector(ptsDistance[i] * Cos(currentAngle), 0, ptsDistance[i] * Sin(currentAngle));

	// Define the direction along which each handle should move assuming the they keep moving on a 
	// constant direction
	// NOTE: the handles direction could be also recomputed after every interaction for example 
	//			to align the handle perpendicularly to the direction defined by the opposite vertexes
	info.direction = Vector(Cos(currentAngle), 0, Sin(currentAngle));

	// Set the handle type
	info.type = HANDLECONSTRAINTTYPE_LINEAR;
}

void VertexHandle::SetHandle(BaseObject* op, Int32 i, Vector p, const HandleInfo& info)
{
	if (nullptr == op)
		return;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* bcPtr = op->GetDataInstance();

	if (nullptr == bcPtr)
		return;

	// Project the final position of the handle (after being moved) on the moving direction in 
	// order to calculate the distance occurred.
	const Float moveValue = Dot(p, info.direction);

	// Update the affected value in the Attribute Manager.
	switch (i)
	{
	case 0:
		bcPtr->SetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_A_DIST, moveValue);
		break;
	case 1:
		bcPtr->SetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_B_DIST, moveValue);
		break;
	case 2:
		bcPtr->SetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_C_DIST, moveValue);
		break;
	}
}

void VertexHandle::GetDimension(BaseObject *op, Vector *mp, Vector *rad)
{
	// Set the bbox radius and bbox center to zero.
	mp->SetZero();
	rad->SetZero();

	if (nullptr == op)
		return;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* bcPtr = op->GetDataInstance();

	if (nullptr == bcPtr)
		return;

	// Store vertexes distances in a vector.
	Vector ptsDistance(bcPtr->GetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_A_DIST), bcPtr->GetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_B_DIST), bcPtr->GetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_C_DIST));

	// Assign the maximum vector component to the x and z component of the box radius vector.
	rad->x = ptsDistance.GetMax();
	rad->z = ptsDistance.GetMax();
}

BaseObject* VertexHandle::GetVirtualObjects(BaseObject *op, HierarchyHelp *hh)
{
	if (nullptr == op)
		return nullptr;

	BaseContainer* objectDataPtr = op->GetDataInstance();
	if (nullptr == objectDataPtr)
		return nullptr;

	// Retrieve the distance-from-origin values from the Attribute Manager.
	const Vector ptsDistance(objectDataPtr->GetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_A_DIST), objectDataPtr->GetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_B_DIST), objectDataPtr->GetFloat(SDK_EXAMPLE_VERTEXHANDLE_POINT_C_DIST));

	// Allocate a PolygonObject instance (a simple triangular face).
	PolygonObject* triObjPtr = PolygonObject::Alloc(3, 1);
	if (nullptr == triObjPtr)
		return BaseObject::Alloc(Onull);

	// Get writing access to the vertexes array. 
	Vector* triPntsPtr = triObjPtr->GetPointW();
	if (nullptr == triPntsPtr)
	{
		PolygonObject::Free(triObjPtr);
		return BaseObject::Alloc(Onull);
	}

	// Set vertexes positions for the triangle.
	triPntsPtr[0] = Vector(0, 0, ptsDistance[0]);
	triPntsPtr[1] = Vector(ptsDistance[1] * Cos(-PI / 6), 0, ptsDistance[1] * Sin(-PI / 6));
	triPntsPtr[2] = Vector(ptsDistance[2] * Cos(-PI * 5 / 6), 0, ptsDistance[2] * Sin(-PI * 5 / 6));

	// Get writing access to the vertexes indexes array.
	CPolygon* polysPtr = triObjPtr->GetPolygonW();
	if (nullptr == polysPtr)
	{
		PolygonObject::Free(triObjPtr);
		return BaseObject::Alloc(Onull);
	}

	// Set vertexes indexes for the single triangle.
	polysPtr[0] = CPolygon(0, 2, 1);

	triObjPtr->Message(MSG_UPDATE);

	return triObjPtr;
}

DRAWRESULT VertexHandle::Draw(BaseObject* op, DRAWPASS drawpass, BaseDraw* bd, BaseDrawHelp* bh)
{
	// Draw override to add some more visual appeal to the handles and show during mouse-hovering
	// on the handles the allowed moving direction. 
	// This override beside drawing small segments from the location of the handle identifying the 
	// moving direction requires also to redraw the handles by explicitly calling DrawHandle.

	if (nullptr == op || nullptr == bd || nullptr == bh)
		return DRAWRESULT_ERROR;

	if (drawpass != DRAWPASS_HANDLES)
		return DRAWRESULT_SKIP;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* bcPtr = op->GetDataInstance();

	if (nullptr == bcPtr)
		return DRAWRESULT_SKIP;


	HandleInfo handleInfo;
	const Int32 currentHandleId = op->GetHighlightHandle(bd);
	const Int32 handlesCount = GetHandleCount(op);

	// Set the transformation matrix to transform the coordinates passed to the drawing function 
	// in global coordinates.
	bd->SetMatrix_Matrix(op, bh->GetMg());

	// Loop over all the handles to.
	for (Int32 i = 0; i < handlesCount; ++i)
	{
		// Retrieve the handle information for each handle. 
		GetHandle(op, i, handleInfo);
		const Float currentAngle = GetCurrentAngle(i);
		const Float handleLength = 50;

		if (i == currentHandleId)
		{
			// The currently highlighted handled is found.
			bd->SetPen(GetViewColor(VIEWCOLOR_SELECTION_PREVIEW));
			// Calculate the position of the handle's end 
			Vector finalPos = handleInfo.position;
			finalPos += Vector(handleLength * Cos(currentAngle), 0, handleLength * Sin(currentAngle));
			// Draw the allowed direction by showing a small segment.
			bd->DrawLine(handleInfo.position, finalPos, NOCLIP_D);
			// Draw the handle at the current position.
			bd->DrawHandle(handleInfo.position, DRAWHANDLE_BIG, 0);
		}
		else
		{
			// Just draw the handle.
			bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
			bd->DrawHandle(handleInfo.position, DRAWHANDLE_BIG, 0);
		}
	}

	return SUPER::Draw(op, drawpass, bd, bh);
}
/// @}

Bool RegisterVertexHandle()
{
	String registeredName = GeLoadString(IDS_OBJECTDATA_VERTEXHANDLE);
	if (!registeredName.Content() || registeredName == "StrNotFound")
		registeredName = "C++ SDK - Vertex Handle Example";

	return RegisterObjectPlugin(ID_SDKEXAMPLE_OBJECTDATA_VERTEXHANDLE, registeredName, OBJECT_GENERATOR, VertexHandle::Alloc, "overtexhandle", AutoBitmap("vertexhandle.tif"), 0);
}