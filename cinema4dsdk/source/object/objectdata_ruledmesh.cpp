#include "c4d_basetag.h"
#include "c4d_objectdata.h"
#include "c4d_thread.h"

// Includes from cinema4dsdk
#include "c4d_symbols.h"
#include "main.h"

// Local resources
#include "oruledmesh.h"

/**A unique plugin ID. You must obtain this from http://www.plugincafe.com. Use this ID to create new instances of this object.*/
static const Int32 ID_SDKEXAMPLE_OBJECTDATA_RULEDMESH = 1038251;

namespace RuledMeshHelpers
{
	//------------------------------------------------------------------------------------------------
	/// Global helper function to populate PolygonObject vertices array and polygon indexes array 
	/// with the data contained in a 2-dim array of vector listing the vertices space position.
	/// @brief Global helper function to populate PolygonObject with data from 2-dim array of positions.
	/// @param[out] polyObj						The reference to the PolygonObject instance.
	/// @param[in] verticesAlongS			The reference to the number of vertices along the S-direction.
	/// @param[in] verticesAlongT			The reference to the number of vertices along the T-direction.
	/// @param[in] hh									The pointer to the HierarchyHelp isntance. @callerOwnsPointed{hierarchy helper}.
	/// @param[in] verticesBA					The reference to the 2-dim BaseArray instance containing the vertices's position.
	/// @param[in] closedS						The reference to the boolean closure status along S-direction.
	/// @param[in] closedT						The reference to the boolean closure status along T-direction.
	/// @return												True if operation completes successfully, false otherwise.
	//------------------------------------------------------------------------------------------------
	const Bool FillPolygonObjectData(PolygonObject &polyObj, const Int32 &verticesAlongS, const Int32& verticesAlongT, HierarchyHelp* hh, const maxon::BaseArray<maxon::BaseArray<Vector>> &verticesBA, const Bool& closedS = false, const Bool& closedT = false);
	const Bool FillPolygonObjectData(PolygonObject &polyObj, const Int32 &verticesAlongS, const Int32& verticesAlongT, HierarchyHelp* hh, const maxon::BaseArray<maxon::BaseArray<Vector>> &verticesBA, const Bool& closedS /*= false*/, const Bool& closedT /*= false*/)
	{
		Vector* verticesArrayW = polyObj.GetPointW();
		if (nullptr == verticesArrayW)
			return false;

		CPolygon* polysIdxArrayW = polyObj.GetPolygonW();
		if (nullptr == polysIdxArrayW)
			return false;

		Int32 vtxIdx, s, t;
		const Int32 polysAlongS = verticesAlongS - 1;
		const Int32 polysAlongT = verticesAlongT - 1;

		Int32 vertsAlongS = verticesAlongS;
		if (closedS)
			vertsAlongS--;

		Int32 vertsAlongT = verticesAlongT;
		if (closedT)
			vertsAlongT--;

		for (s = 0; s < (vertsAlongS); ++s)
		{
			for (t = 0; t < (vertsAlongT); ++t)
			{
				if (hh)
				{
					// Check from time to time (every 64 segments) if a user break has been requested.
					BaseThread* btPtr = hh->GetThread();
					if (btPtr && !(t & 63) && btPtr->TestBreak())
						return false;
				}

				vtxIdx = s * vertsAlongT + t;
				// Fill the vertices data.
				verticesArrayW[vtxIdx] = verticesBA[s][t];

				// Fill the polygons indices.
				if (s < polysAlongS && t < polysAlongT)
				{
					// Define the general polygon index.
					Int32 polyIdx = s * polysAlongT + t;

					Int32 s_pad = 0;
					Int32 t_pad = 0;
					if (closedS && s == polysAlongS - 1)
						s_pad = vertsAlongS;
					if (closedT && t == polysAlongT - 1)
						t_pad = vertsAlongT;

					// Check the type of generated face (triangles or quadrangles).
					if (polyObj.GetPolygonCount() != (polysAlongS * polysAlongT))
					{
						Int32 polyIdxA = 2 * polyIdx;
						Int32 polyIdxB = 2 * polyIdx + 1;

						// Fill polygon (triangle A) by using indexes for the vertexes generated above.
						polysIdxArrayW[polyIdxA] = CPolygon(
							t + s * vertsAlongT,
							t + (s + 1 - s_pad) * vertsAlongT,
							t + (s + 1 - s_pad) * vertsAlongT + 1 - t_pad
							);

						//	Fill polygon (triangle B) by using indexes for the vertexes generated above.
						polysIdxArrayW[polyIdxB] = CPolygon(
							t + s * vertsAlongT,
							t + (s + 1 - s_pad) * vertsAlongT + 1 - t_pad,
							t + s * vertsAlongT + 1 - t_pad
							);
					}
					else
					{
						//	Fill polygon [quad] by using indexes for the vertexes generated above.
						polysIdxArrayW[polyIdx] = CPolygon(
							t + s * vertsAlongT,
							t + (s + 1 - s_pad) * vertsAlongT,
							t + (s + 1 - s_pad) * vertsAlongT + 1 - t_pad,
							t + s * vertsAlongT + 1 - t_pad
							);
					}
				}
			}
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/// Global helper function to resize the BaseArray of BaseArray of vector for storing points pos.
	/// @brief Global helper function to create, init and return a 2-dim BaseArray of vector.
	/// @param[in] verticesBA					The 2-dim BaseArray storing the vertices position.
	/// @param[in] sizeA							The size of the first dimension.
	/// @param[in] sizeB							The size of the second dimension.
	/// @param[in] closedA						The reference to the boolean closure status along S-direction.
	/// @param[in] closedB						The reference to the boolean closure status along T-direction.
	/// @return												True if resize successfully occurs, false otherwise.
	//------------------------------------------------------------------------------------------------	
	Bool ResizeVerticesBaseArray(maxon::BaseArray<maxon::BaseArray<Vector>> &verticesBA, const Int32 &sizeA, const Int32 &sizeB, const Bool& closedA = false, const Bool& closedB = false);
	Bool ResizeVerticesBaseArray(maxon::BaseArray<maxon::BaseArray<Vector>> &verticesBA, const Int32 &sizeA, const Int32 &sizeB, const Bool& closedA /*= false*/, const Bool& closedB /*= false*/)
	{
		Int32 sizeA_ = sizeA;
		if (closedA)
			sizeA_--;
		Int32 sizeB_ = sizeB;
		if (closedB)
			sizeB_--;

		if (verticesBA.Resize(sizeA_) != true)
		{
			return false;
		}

		for (Int32 i = 0; i < verticesBA.GetCount(); i++)
		{
			if (verticesBA[i].Resize(sizeB_) != true)
			{
				return false;
			}
		}

		return true;
	}
}

//------------------------------------------------------------------------------------------------
/// ObjectData implementation responsible for generating a ruled mesh using two curves as input 
/// objects and connecting via linear interpolation (https://en.wikipedia.org/wiki/Ruled_surface). 
/// Mesh creation is controlled by specifying S/T curves segmentations and curves parametrization.
//------------------------------------------------------------------------------------------------

/*! \brief A simple object generator creating a ruled mesh from two curves.
*/
class RuledMesh : public ObjectData
{
	INSTANCEOF(RuledMesh, ObjectData)

public:
	static NodeData* Alloc(void){ return NewObjClear(RuledMesh); }
	virtual Bool Init(GeListNode *node);
	virtual void GetDimension(BaseObject *op, Vector *mp, Vector *rad);
	virtual BaseObject* GetVirtualObjects(BaseObject *op, HierarchyHelp *hh);
	virtual Bool Message(GeListNode* node, Int32 type, void* data);

private:
	//------------------------------------------------------------------------------------------------
	/// Private method to check the dirty status and clone (eventually) the input objects.
	/// @brief Method to check the dirty status and clone (eventually) the input objects.
	/// @param[in] op							The pointer to the BaseObject instance. @callerOwnsPointed{hierarchy helper}.
	/// @param[in] hh							The pointer to the HierarchyHelp isntance. @callerOwnsPointed{object}.
	/// @param[in] dirtyFlag			The reference to the Bool dirty flag.
	/// @param[in] firstChild			The pointer to the first input object to start the clone from. @callerOwnsPointed{object}.
	/// @return										True if successful, false otherwise.
	//------------------------------------------------------------------------------------------------
	const Bool GetFirstAndSecondClonedCurves(BaseObject* op, HierarchyHelp* hh, Bool &dirtyFlag, BaseObject* firstChild);
	
	//------------------------------------------------------------------------------------------------
	/// Private method to populate the 2-dim BAseArray responsible for storing the vertices position.
	/// @brief Method to populate the 2-dim BAseArray responsible for storing the vertices position.
	/// @param[in] verticesData		The reference to the 2-dim BaseArray instance.
	/// @param[in] stepsS					The reference to the number of segments on S.
	/// @param[in] stepsT					The reference to the number of segments on T.
	/// @param[in] firstFlip			The reference to the invert direction flag on the first curve.
	/// @param[in] secondFlip			The reference to the invert direction flag on the second curve.
	/// @param[in] paramFirst			The reference to the parametrization type on the first curve.
	/// @param[in] paramSecond		The reference to the parametrization type on the second curve.
	/// @return										True if successful, false otherwise.
	//------------------------------------------------------------------------------------------------
	const Bool FillVerticesPosition(maxon::BaseArray<maxon::BaseArray<Vector>> &verticesData, const Int32 &stepsS, const Int32 &stepsT, const Bool &firstFlip, const Bool &secondFlip, const Int32 &paramFirst, const Int32 &paramSecond);

	//------------------------------------------------------------------------------------------------
	/// Private method to check and set the Phong tag for the returned PolygonObject.
	/// @brief Method to check and set the Phong tag for the returned PolygonObject.
	/// @param[in] op							The pointer to the BaseObject instance. @callerOwnsPointed{hierarchy helper}.
	/// @param[in] polyObj				The pointer to the PolygonOjbect isntance. @callerOwnsPointed{polygon object}.
	/// @return										True if successful, false otherwise.
	//------------------------------------------------------------------------------------------------
	const Bool CheckAndSetPhongTag(BaseObject *op, PolygonObject *polyObj);

	//------------------------------------------------------------------------------------------------
	/// Private method to allocate the SplingLengthHelper() instances.
	/// @brief Method to allocate the SplingLengthHelper() instances.
	/// @return										True if successful, false otherwise.
	//------------------------------------------------------------------------------------------------
	const Bool AllocateSplineHelpers();

	//------------------------------------------------------------------------------------------------
	/// Private method used before returning from GVO to release all the allocated resources.
	/// @brief Method used before returning from GVO to release all the allocated resources.
	/// @return										True if successful, false otherwise.
	//------------------------------------------------------------------------------------------------
	const Bool FreeResources();

private:
	BaseObject *_clonedObjs;										/// Pointer to the BaseObject resulting from GetAndCheckHierarchyClone()
	SplineObject *_firstCrv, *_secondCrv;				/// Pointers to the SplineObject(s) found in the cloned null object
	SplineLengthData *_firstCurveLengthHelper,	/// Pointers to the SplineLengthHelpers used to calculate the spline point position
		*_secondCurveLengthHelper;
};

/// @name ObjectData functions
/// @{
Bool RuledMesh::Init(GeListNode* node)
{
	_clonedObjs = nullptr;
	_firstCrv = nullptr;
	_secondCrv = nullptr;
	_firstCurveLengthHelper = nullptr;
	_secondCurveLengthHelper = nullptr;

	// Check the provided input pointer.
	if (nullptr == node)
		return false;

	// Cast the node to the BasObject class.
	BaseObject* baseObjPtr = static_cast<BaseObject*>(node);

	// Retrieve the BaseContainer instance binded to the BaseObject instance.
	BaseContainer* bcPtr = baseObjPtr->GetDataInstance();

	// Check the BaseContainer instance pointer.
	if (nullptr == bcPtr)
		return false;

	// Set the values for the different parameters of the generator.
	bcPtr->SetInt32(SDK_EXAMPLE_RULEDMESH_S_STEPS, 5);
	bcPtr->SetInt32(SDK_EXAMPLE_RULEDMESH_T_STEPS, 5);
	bcPtr->SetBool(SDK_EXAMPLE_RULEDMESH_FLIP_FIRST, false);
	bcPtr->SetBool(SDK_EXAMPLE_RULEDMESH_FLIP_SECOND, false);
	bcPtr->SetInt32(SDK_EXAMPLE_RULEDMESH_PARAM_FIRST, SDK_EXAMPLE_RULEDMESH_PARAM_NATURAL);
	bcPtr->SetInt32(SDK_EXAMPLE_RULEDMESH_PARAM_SECOND, SDK_EXAMPLE_RULEDMESH_PARAM_NATURAL);

	return true;
}

Bool RuledMesh::Message(GeListNode* node, Int32 type, void* data)
{
	if (type == MSG_MENUPREPARE)
		((BaseObject*)node)->SetPhong(true, true, DegToRad(40.0));

	return SUPER::Message(node, type, data);
}

void RuledMesh::GetDimension(BaseObject* op, Vector* mp, Vector* rad)
{
	// Check the provided pointers.
	if (nullptr == op || nullptr == rad || nullptr == mp)
		return;

	// Reset the radius and center vectors.
	mp->SetZero();
	rad->SetZero();
}

const Bool RuledMesh::FreeResources()
{
	if (_clonedObjs)
	{
		BaseObject::Free(_clonedObjs);
		_clonedObjs = nullptr;
	}

	if (_firstCurveLengthHelper)
	{
		SplineLengthData::Free(_firstCurveLengthHelper);
		_firstCurveLengthHelper = nullptr;
	}

	if (_secondCurveLengthHelper)
	{
		SplineLengthData::Free(_secondCurveLengthHelper);
		_secondCurveLengthHelper = nullptr;
	}

	return true;
}

const Bool RuledMesh::GetFirstAndSecondClonedCurves(BaseObject* op, HierarchyHelp* hh, Bool &dirtyFlag, BaseObject* firstChild)
{
	_clonedObjs = op->GetAndCheckHierarchyClone(hh, firstChild, HIERARCHYCLONEFLAGS_ASIS, &dirtyFlag, nullptr, true);
	if (!dirtyFlag)
		return true;

	// Something has failed in GetAndCheckHierarchyClone()
	if (!_clonedObjs)
		return false;

	// NOTE: The cloned children are placed under the clonedObjs (which is a null object)
	_firstCrv = static_cast<SplineObject*>(_clonedObjs->GetDown());
	if (!_firstCrv)
		return false;
	_firstCrv = _firstCrv->GetRealSpline();

	_secondCrv = static_cast<SplineObject*>(_clonedObjs->GetDown()->GetNext());
	if (!_secondCrv)
		return false;
	_secondCrv = _secondCrv->GetRealSpline();

	return true;
}

const Bool RuledMesh::FillVerticesPosition(maxon::BaseArray<maxon::BaseArray<Vector>> &verticesData, const Int32 &stepsS, const Int32 &stepsT, const Bool &flipFirst, const Bool &flipSecond, const Int32 &paramFirst, const Int32 &paramSecond)
{
	// Retrieve the local transformation for both cloned children
	Matrix firstLocalMtx = _firstCrv->GetMl();
	Matrix secondLocalMtx = _secondCrv->GetMl();
	
	
	const Float sStep = 1.0f / float(stepsS);
	const Float tStep = 1.0f / float(stepsT);
	
	Int32 s, t;
	for (s = 0; s < (stepsS + 1); ++s)
	{
		Float sParam = (Float)s * sStep;
		Float abscissaA = sParam;
		Float abscissaB = sParam;
		
		// Check if the curve has been flipped and accordingly change the param value.
		if (flipFirst)
			abscissaA = 1.0f - abscissaA;
		if (flipSecond)
			abscissaB = 1.0f - abscissaB;
		
		// Check the parametrization of the curves and modify the abscissa accordingly
		if (paramFirst == SDK_EXAMPLE_RULEDMESH_PARAM_UNIFORM)
			abscissaA = _firstCurveLengthHelper->UniformToNatural(abscissaA);
		if (paramSecond == SDK_EXAMPLE_RULEDMESH_PARAM_UNIFORM)
			abscissaB = _secondCurveLengthHelper->UniformToNatural(abscissaB);
		
		// Retrieve the position of the point at the i-th step on the splines object.
		Vector pointOnFirstCurve = _firstCrv->GetSplinePoint(abscissaA);
		Vector pointOnSecondCurve = _secondCrv->GetSplinePoint(abscissaB);
		
		// Transform the point in global coordinates.
		pointOnFirstCurve = firstLocalMtx * pointOnFirstCurve;
		pointOnSecondCurve = secondLocalMtx * pointOnSecondCurve;
		
		// Fill the vertices's positions of the resulting ruled mesh
		for (t = 0; t < (stepsT + 1); ++t)
		{
			Float tParam = t * tStep;
			verticesData[s][t] = (1 - tParam)*pointOnFirstCurve + tParam*pointOnSecondCurve;
		}
	}
	
	return true;
}

const Bool RuledMesh::CheckAndSetPhongTag(BaseObject *op, PolygonObject *polyObj)
{
	if (!op || !polyObj)
		return false;
	
	// Check the existence of the Phong tag for the generated polygon object
	// cloning the one found in the generator.
	if (op->GetTag(Tphong))
	{
		BaseTag *phongTagPolyObj = polyObj->GetTag(Tphong);
		// If the Phong tag already exists  on the PolygonObject delete it and add the new cloned tag
		// in order to copy the changes occurred on the tag attached to the generator
		if (phongTagPolyObj)
			polyObj->KillTag(Tphong);
			
			// Clone the Phong tag.
			phongTagPolyObj = (BaseTag*)op->GetTag(Tphong)->GetClone(COPYFLAGS_0, nullptr);
			
			// Insert the newly cloned tag.
			if (nullptr != phongTagPolyObj)
				polyObj->InsertTag(phongTagPolyObj);
		}
	
	return true;
}

const Bool RuledMesh::AllocateSplineHelpers()
{
	// Allocate the SplineLength helper objects.
	_firstCurveLengthHelper = SplineLengthData::Alloc();
	if (!_firstCurveLengthHelper)
		return false;
	_firstCurveLengthHelper->Init(_firstCrv);

	_secondCurveLengthHelper = SplineLengthData::Alloc();
	if (!_secondCurveLengthHelper)
		return false;
	_secondCurveLengthHelper->Init(_secondCrv);

	return true;
}

BaseObject* RuledMesh::GetVirtualObjects(BaseObject *op, HierarchyHelp *hh)
{
	// Check the provided pointers.
	if (nullptr == op || nullptr == hh)
		return BaseObject::Alloc(Onull);

	// Check the presence of two children curves needed to run the generator.
	if (!op->GetDown())
		return BaseObject::Alloc(Onull);

	BaseObject* firstLeaf = op->GetDown();
	BaseObject* secondLeaf = firstLeaf->GetNext();
	if (nullptr == secondLeaf)
		return BaseObject::Alloc(Onull);

	Bool cloneIsDirty = false;
	if (!GetFirstAndSecondClonedCurves(op, hh, cloneIsDirty, firstLeaf))
	{
		FreeResources();
		return BaseObject::Alloc(Onull);
	}

	if (!cloneIsDirty)
		return op->GetCache(hh);

	// Retrieve the BaseContainer associated check it 
	BaseContainer* bcPtr = op->GetDataInstance();
	if (nullptr == bcPtr)
	{
		FreeResources();
		return BaseObject::Alloc(Onull);
	}
	// Retrieve the number of subdivisions which every of the two splines will be subdivided in.
	const Int32 stepsS = bcPtr->GetInt32(SDK_EXAMPLE_RULEDMESH_S_STEPS);
	const Int32 stepsT = bcPtr->GetInt32(SDK_EXAMPLE_RULEDMESH_T_STEPS);

	// Retrieve the flip flags for the two curves
	const Bool flipFirst = bcPtr->GetBool(SDK_EXAMPLE_RULEDMESH_FLIP_FIRST);
	const Bool flipSecond = bcPtr->GetBool(SDK_EXAMPLE_RULEDMESH_FLIP_SECOND);

	const Int32 paramFirst = bcPtr->GetInt32(SDK_EXAMPLE_RULEDMESH_PARAM_FIRST);
	const Int32 paramSecond = bcPtr->GetInt32(SDK_EXAMPLE_RULEDMESH_PARAM_SECOND);
	
	if (!AllocateSplineHelpers())
	{
		FreeResources();
		return BaseObject::Alloc(Onull);
	}

	// Define the resulting polygon mesh parameters and allocate the object.
	const Int32 vertexesCount = (stepsS + 1) * (stepsT + 1);
	const Int32 polysCount = stepsS * stepsT;
	
	// Allocate the PolygonObject used to return the geometry of the generator
	PolygonObject* polyObj = PolygonObject::Alloc(vertexesCount, polysCount);
	// check the allocated PolygonObject
	if (!polyObj)
	{
		FreeResources();
		return BaseObject::Alloc(Onull);
	}

	// Allocate a 2-dim BaseArray to store vertices position.
	maxon::BaseArray<maxon::BaseArray<Vector>> verticesBA;

	// Resize the 2-dim BaseArray to store the computed position for the vertices.
	if (!RuledMeshHelpers::ResizeVerticesBaseArray(verticesBA, stepsS + 1, stepsT + 1))
	{
		FreeResources();
		return BaseObject::Alloc(Onull);
	}
	
	if (!FillVerticesPosition(verticesBA, stepsS, stepsT, flipFirst, flipSecond, paramFirst, paramSecond))
	{
		FreeResources();
		return BaseObject::Alloc(Onull);
	}
	
	// Populate the PolygonObject with the computed position of the vertices.
	if (RuledMeshHelpers::FillPolygonObjectData(*polyObj, stepsS + 1, stepsT + 1, hh, verticesBA))
		polyObj->Message(MSG_UPDATE);

	CheckAndSetPhongTag(op, polyObj);
	
	// Deallocate all the local objects.
	FreeResources();
	return polyObj;
}
/// @}

Bool RegisterRuledMesh()
{
	String registeredName = GeLoadString(IDS_OBJECTDATA_RULEDMESH);
	if (!registeredName.Content() || registeredName == "StrNotFound")
		registeredName = "C++ SDK - Ruled Mesh Generator Example";

	return RegisterObjectPlugin(ID_SDKEXAMPLE_OBJECTDATA_RULEDMESH, registeredName, OBJECT_GENERATOR | OBJECT_INPUT, RuledMesh::Alloc, "oruledmesh", AutoBitmap("ruledmesh.tif"), 0);
}