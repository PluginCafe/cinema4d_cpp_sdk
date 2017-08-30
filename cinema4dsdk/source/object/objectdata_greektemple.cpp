#include "c4d_baseobject.h"
#include "c4d_general.h"
#include "c4d_includes.h"
#include "c4d_objectdata.h"
#include "c4d_thread.h"

#include "lib_description.h"

// Includes from cinema4dsdk
#include "c4d_symbols.h"
#include "main.h"

// Local resources
#include "ogreektemple.h"

/**A unique plugin ID. You must obtain this from http://www.plugincafe.com. Use this ID to create new instances of this object.*/
static const Int32 ID_SDKEXAMPLE_OBJECTDATA_GREEKTEMPLE = 1038235;

namespace GreekTempleHelpers
{
	//------------------------------------------------------------------------------------------------
	/// Global helper function returning the scaling factor of the value passed based on modulo and
	/// base unit.
	/// @brief Global helper function to compute the scale factor upon a modulo, a base unit and a scaling value
	/// @param[in] modulo					Multiplier of the base unit.
	/// @param[in] baseUnit				Base value of the scaling.
	/// @param[in] value					Original value to scale.
	/// @return										Scaled value.
	//------------------------------------------------------------------------------------------------
	const Float ComputeScaleFactor(const Float& modulo, const Float& baseUnit, const Float& value);
	const Float ComputeScaleFactor(const Float& modulo, const Float& baseUnit, const Float& value)
	{
		if (0 != modulo && 0 != value)
			return (value - modulo * (baseUnit * 2)) / value;
		else
			return 1.0f;
	}
	
	//------------------------------------------------------------------------------------------------
	/// Global function responsible to create the base of the temple using slabs.
	/// @brief Global function to create the temple stairs.
	/// @param[out] parentObj			Object provided to store created geometry. @callerOwnsPointed{base object}
	/// @param[in] baseUnit				Base value for geometry construction.
	/// @param[in] objSize				Bounding box radius vector.
	/// @param[in] objSegsPtr			Segmentation array.
	/// @return										True if building process succeeds.
	//------------------------------------------------------------------------------------------------
	const Bool CreateTempleBase(BaseObject* &parentObj, const Float& baseUnit, const Vector& objSize, const Int32* objSegsPtr = nullptr, const Int32 stairsCount = 3);
	const Bool CreateTempleBase(BaseObject* &parentObj, const Float& baseUnit, const Vector& objSize, const Int32* objSegsPtr /*= nullptr*/, const Int32 stairsCount /*=3*/)
	{

		//	Create the rectangle profile used to extrude the base temple.
		AutoAlloc<BaseObject>	slabPtr(Ocube);
		AutoAlloc<BaseObject> baseNullObjPtr(Onull);

		if (!slabPtr || !baseNullObjPtr)
			return false;

		//	Set object names accordingly.
		slabPtr->SetName("TempleBaseStair_0");
		baseNullObjPtr->SetName("TempleBase");

		//	Compute the stair thickness assuming that the minimum number of stairs is
		//	three and the total thickness of the stairs should be 3 base units.
		const Float stairUnit = baseUnit * 3 / stairsCount;

		//	Create a slab sizes vector formed by two components taken
		//	from the objSize and the y-component set to baseUnit.
		const Vector slabSizes(objSize.x, stairUnit, objSize.z);
		slabPtr->SetParameter(PRIM_CUBE_LEN, slabSizes, DESCFLAGS_SET_0);

		//	Assign the segmentation data to the slab object to allow
		//	proper deforming operators to run.
		if (nullptr != objSegsPtr)
		{
			slabPtr->SetParameter(PRIM_CUBE_SUBX, (Int32)objSegsPtr[0], DESCFLAGS_SET_0);
			slabPtr->SetParameter(PRIM_CUBE_SUBY, (Int32)objSegsPtr[1], DESCFLAGS_SET_0);
			slabPtr->SetParameter(PRIM_CUBE_SUBZ, (Int32)objSegsPtr[2], DESCFLAGS_SET_0);
		}

		//	Position the slab to match the bottom face of the calling object
		//	or of the ideal bounding box sized by the objSize vector.
		slabPtr->SetRelPos(Vector(0, (-objSize.y + stairUnit) * 0.5, 0));

		// Allocate object needed to be used with instance generators.
		AutoAlloc<BaseLink> pBL;
		GeData baseLinkData;

		if (!pBL)
			return false;

		//	Create a baselink to populate the instance generators object_link parameter and
		//	set the parameter accordingly.
		pBL->SetLink(slabPtr);
		baseLinkData.SetBaseLink(*pBL);

		for (Int32 i = 1; i < stairsCount; i++)
		{
			AutoAlloc<BaseObject> stairInstancePtr(Oinstance);

			if (nullptr == stairInstancePtr)
				return false;

			stairInstancePtr->SetName("TempleBaseStair_" + String::IntToString(i));

			//	Create scaling components and accordingly scale the instance to guarantee
			//	that the new width and height are modules of the unit base.
			const Float xScaleFactor = ComputeScaleFactor(i, stairUnit, objSize.x);
			const Float zScaleFactor = ComputeScaleFactor(i, stairUnit, objSize.z);
			stairInstancePtr->SetRelScale(Vector(xScaleFactor, 1, zScaleFactor));

			//	Position the instance on top of the previous instance.
			stairInstancePtr->SetRelPos(Vector(0, -objSize.y * 0.5 + stairUnit * (i + 0.5), 0));

			//	Set the link in the first instance object.
			stairInstancePtr->SetParameter(INSTANCEOBJECT_LINK, baseLinkData, DESCFLAGS_SET_0);

			//	Note that the inserting process of "released" object should
			//	follow a child->parent direction (before child than parent)
			//	otherwise released pointers will be no more accessible.
			stairInstancePtr.Release()->InsertUnder(baseNullObjPtr);
		}

		//	Insert the instance reference under the null "base" object.
		slabPtr.Release()->InsertUnder(baseNullObjPtr);

		//	Insert the final assembled node under the parent object passed.
		baseNullObjPtr.Release()->InsertUnder(parentObj);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	/// Global function responsible to create the roof temple booleans decorations
	/// @param[out] roofNullObjPtr			Object provided to store created geometry. @callerOwnsPointed{base object}
	/// @param[in] extrudeRoofGenRelPtr	Extruded roof object. @callerOwnsPointed{base object}
	/// @param[in] baseUnit							Base value for geometry construction.
	/// @param[in] objSize							Bounding box radius vector.
	/// @return													True if building process succeeds.
	//------------------------------------------------------------------------------------------------
	const Bool PerformBooleanSubrtraction(BaseObject* roofNullObjPtr, BaseObject* extrudeRoofGenRelPtr, const Float& baseUnit, const Vector& objSize);
	const Bool PerformBooleanSubrtraction(BaseObject* roofNullObjPtr, BaseObject* extrudeRoofGenRelPtr, const Float& baseUnit, const Vector& objSize)
	{
		//	Create some instance of the roof to detail the temple roof using booleans.
		AutoAlloc<BaseObject> roofBooleanSub01Ptr(Oinstance), roofBooleanSub02Ptr(Oinstance);
		if (!roofBooleanSub01Ptr || !roofBooleanSub02Ptr)
			return false;

		//	Set instance names accordingly.
		roofBooleanSub01Ptr->SetName("RoofSubtractedInstance01");
		roofBooleanSub02Ptr->SetName("RoofSubtractedInstance02");

		//	Set the base link to point to the extruded roof and set the value of the link of the
		//	two instances accordingly.
		AutoAlloc<BaseLink> pBL;
		GeData baseLinkData;

		if (!pBL)
			return false;

		pBL->SetLink(extrudeRoofGenRelPtr);
		baseLinkData.SetBaseLink(pBL);
		roofBooleanSub01Ptr->SetParameter(INSTANCEOBJECT_LINK, baseLinkData, DESCFLAGS_SET_0);
		roofBooleanSub02Ptr->SetParameter(INSTANCEOBJECT_LINK, baseLinkData, DESCFLAGS_SET_0);

		//	Compute the scale factors in order to leave a reasonable space running around the
		//	roof shape and enough length to perform the boolean subtraction.
		if (objSize.z == 0)
			return false;
		const Float xScaleComponent = ComputeScaleFactor(2, baseUnit, objSize.x);
		const Float yScaleComponent = ComputeScaleFactor(2, baseUnit, objSize.y);
		const Float zScaleComponent = baseUnit / objSize.z;

		//	Scale the instances accordingly to the components computed.
		roofBooleanSub01Ptr->SetAbsScale(Vector(xScaleComponent, yScaleComponent, zScaleComponent));
		roofBooleanSub02Ptr->SetAbsScale(Vector(xScaleComponent, yScaleComponent, zScaleComponent));

		//	Set the instances position of the first instance to overlap with the start of the roof.
		const Float xOffsetComponent = 0;
		const Float yOffsetComponent = 0;
		Float zOffsetComponent = objSize.z / 2 - 0.5 * baseUnit;
		roofBooleanSub01Ptr->SetRelPos(Vector(xOffsetComponent, yOffsetComponent, zOffsetComponent));

		//	Set the instances position of the second instance to overlap with the end of the roof.
		zOffsetComponent = -zOffsetComponent - baseUnit;
		roofBooleanSub02Ptr->SetRelPos(Vector(xOffsetComponent, yOffsetComponent, zOffsetComponent));

		//	Create Boolean generators and set the parameter accordingly to subtract the two
		//	instances from the main extruded roof body.
		AutoAlloc<BaseObject> boolFrontPtr(Oboole), boolRearPtr(Oboole);
		if (!boolFrontPtr || !boolRearPtr)
			return false;

		//	Set boolean generators' name accordingly.
		boolRearPtr->SetName("RoofRearBooleanSubOperation");
		boolFrontPtr->SetName("RoofFrontBooleanSubOperation");

		//	Set the subtract operation for both boolean operators.
		boolFrontPtr->SetParameter(BOOLEOBJECT_TYPE, BOOLEOBJECT_TYPE_SUBTRACT, DESCFLAGS_SET_0);
		boolRearPtr->SetParameter(BOOLEOBJECT_TYPE, BOOLEOBJECT_TYPE_SUBTRACT, DESCFLAGS_SET_0);

		//	Make the first boolean operation subtracting the first instance from the roof extrusion.
		extrudeRoofGenRelPtr->InsertUnder(boolFrontPtr);
		roofBooleanSub01Ptr.Release()->InsertUnderLast(boolFrontPtr);

		//	Make the second boolean operation subtracting the second instance from the first boolean.
		boolFrontPtr.Release()->InsertUnder(boolRearPtr);
		roofBooleanSub02Ptr.Release()->InsertUnderLast(boolRearPtr);

		//	Insert the last boolean operand under the roof null object.
		boolRearPtr.Release()->InsertUnderLast(roofNullObjPtr);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/// Global function responsible to create the roof of the temple using a basic
	/// triangular shape and an extrusion operator.
	/// @brief Global function to create the temple roof.
	/// @param[out] parentObj			Object provided to store created geometry. @callerOwnsPointed{base object}
	/// @param[in] baseUnit				Base value for geometry construction.
	/// @param[in] objSize				Bounding box radius vector.
	/// @param[in] objSegsPtr			Segmentation array.
	/// @return										True if building process succeeds.
	//------------------------------------------------------------------------------------------------
	const Bool CreateTempleRoof(BaseObject* &parentObj, const Float& baseUnit, const Vector& objSize, const Int32* objSegsPtr = nullptr);
	const Bool CreateTempleRoof(BaseObject* &parentObj, const Float& baseUnit, const Vector& objSize, const Int32* objSegsPtr /*= nullptr*/)
	{

		AutoAlloc<BaseObject> triangleShapePtr(Osplinenside);
		AutoAlloc<BaseObject> extrudeRoofGenPtr(Oextrude);
		AutoAlloc<BaseObject> roofNullObjPtr(Onull);
		if (!triangleShapePtr || !extrudeRoofGenPtr || !roofNullObjPtr)
			return false;

		//	Set object names accordingly.
		triangleShapePtr->SetName("RoofShape");
		extrudeRoofGenPtr->SetName("RoofExtrusion");
		roofNullObjPtr->SetName("TempleRoof");

		//	Compute some useful value to create the temple roof starting
		//	from the baseUnit value and considering that a triangular shape
		//	will be used as extrusion section for the roof.

		const Float triCircumCircleRadius = baseUnit * 3;
		triangleShapePtr->SetParameter(PRIM_NSIDE_SIDES, 3, DESCFLAGS_SET_0);
		triangleShapePtr->SetParameter(PRIM_NSIDE_RADIUS, triCircumCircleRadius, DESCFLAGS_SET_0);

		//	Assign the segmentation data to the spline object to allow
		//	proper deforming operators to run.
		if (nullptr != objSegsPtr)
		{
			triangleShapePtr->SetParameter(SPLINEOBJECT_INTERPOLATION, SPLINEOBJECT_INTERPOLATION_UNIFORM, DESCFLAGS_SET_0);
			triangleShapePtr->SetParameter(SPLINEOBJECT_SUB, objSegsPtr[1], DESCFLAGS_SET_0);
		}

		// Calculate the length of the triangle side starting from the circle radius.
		const Float triSideSize = 2 * (Cos(PI / 6) * triCircumCircleRadius);
		if (triSideSize == 0)
			return false;

		const Float yScaleComponent = ComputeScaleFactor(2, baseUnit, objSize.x) * objSize.x / triSideSize;
		triangleShapePtr->SetAbsScale(Vector(1, yScaleComponent, 1));
		triangleShapePtr.Release()->InsertUnder(extrudeRoofGenPtr);

		//	Create a translate vector to set the extrusion length of the roof.
		const Vector extrusionVector = Vector(0, 0, objSize.z);
		extrudeRoofGenPtr->SetParameter(EXTRUDEOBJECT_MOVE, extrusionVector, DESCFLAGS_SET_0);

		//	Assign the segmentation data to the extruded object to allow
		//	proper deforming operators to run.
		if (nullptr != objSegsPtr)
			extrudeRoofGenPtr->SetParameter(EXTRUDEOBJECT_SUB, (Int32)objSegsPtr[2], DESCFLAGS_SET_0);

		//	Translate the roof.
		extrudeRoofGenPtr->SetRelPos(Vector(0, 0, -objSize.z / 2));

		BaseObject* extrudeRoofGenRelPtr = extrudeRoofGenPtr.Release();
		if (!extrudeRoofGenRelPtr)
			return false;

		// Perform the boolean subs operation in order to decorate the temple roof;
		PerformBooleanSubrtraction(roofNullObjPtr, extrudeRoofGenRelPtr, baseUnit, objSize);

		//	Rotate and reposition the roof null object to properly match position.
		roofNullObjPtr->SetRotationOrder(ROTATIONORDER_XYZLOCAL);
		roofNullObjPtr->SetAbsRot(Vector(0, 0, -PI * 0.5));
		roofNullObjPtr->SetRelPos(Vector(0, objSize.y / 2 - triCircumCircleRadius, 0));

		roofNullObjPtr.Release()->InsertUnder(parentObj);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	/// Global function responsible to create the capital of the temple column.
	/// @brief Function responsible to create the capital of the temple column.
	/// @param[out] columnCapitalRelPtr		Column capital object pointer. @callerOwnsPointed{base object}
	/// @param[in] capitalHeightRatio			Capital height ratio.
	/// @param[in] capitalSideHeightRatio	Capital side/height ratio.
	/// @param[in] objSegsPtr							Segmentation array.
	/// @return														True if building process succeeds.
	//------------------------------------------------------------------------------------------------
	const Bool CreateColumnCapital(BaseObject* &columnCapitalRelPtr, const Float &capitalHeightRatio, const Float &capitalSideHeightRatio, const Int32* objSegsPtr = nullptr);
	const Bool CreateColumnCapital(BaseObject* &columnCapitalRelPtr, const Float &capitalHeightRatio, const Float &capitalSideHeightRatio, const Int32* objSegsPtr /*= nullptr*/)
	{
		AutoAlloc<BaseObject> columnCapitalPtr(Ocube);
		if (!columnCapitalPtr)
			return false;

		//	Set object names accordingly to their function.
		columnCapitalPtr->SetName("ColumnCapital");
		
		const Float capitalSide = capitalSideHeightRatio * capitalHeightRatio;
		const Vector capitalSizes = Vector(capitalSide, capitalHeightRatio, capitalSide);
		columnCapitalPtr->SetParameter(PRIM_CUBE_LEN, capitalSizes, DESCFLAGS_SET_0);
		columnCapitalPtr->SetParameter(PRIM_CUBE_SUBY, 8, DESCFLAGS_SET_0);

		//	Assign the segmentation data to the steam, base and capital
		//	to allow proper deforming operators to run
		if (nullptr != objSegsPtr)
			columnCapitalPtr->SetParameter(PRIM_CUBE_SUBY, 8 * objSegsPtr[1], DESCFLAGS_SET_0);

		//	Create two taper modifiers to get the typical look of the column
		AutoAlloc<BaseObject> capitalTaper(Otaper);
		if (!capitalTaper)
			return false;

		//	Set modifiers name accordingly to their function
		capitalTaper->SetName("CapitalTaper");

		capitalTaper->SetParameter(DEFORMOBJECT_SIZE, capitalSizes, DESCFLAGS_SET_0);
		capitalTaper->SetParameter(DEFORMOBJECT_STRENGTH, -.5, DESCFLAGS_SET_0);
		capitalTaper->SetParameter(DEFORMOBJECT_CURVATURE, 1, DESCFLAGS_SET_0);
		capitalTaper->SetParameter(DEFORMOBJECT_FILLET, true, DESCFLAGS_SET_0);

		//	Hide the deformer cage from the viewport
		capitalTaper->SetParameter(ID_BASEOBJECT_VISIBILITY_EDITOR, OBJECT_OFF, DESCFLAGS_SET_0);
		capitalTaper.Release()->InsertUnder(columnCapitalPtr);

		columnCapitalRelPtr = columnCapitalPtr.Release();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	/// Global function responsible to create the base of the temple column.
	/// @brief Function responsible to create the base of the temple column.
	/// @param[out] columnBaseRelPtr			Column base object pointer. @callerOwnsPointed{base object}
	/// @param[in] baseHeightRatio				Base height ratio.
	/// @param[in] baseRadiusHeightRatio	Base side/height ratio.
	/// @param[in] objSegsPtr							Segmentation array.
	/// @return														True if building process succeeds.
	//------------------------------------------------------------------------------------------------
	const Bool CreateColumnBase(BaseObject* &columnBaseRelPtr, const Float &baseHeightRatio, const Float &baseRadiusHeightRatio, const Int32* objSegsPtr = nullptr);
	const Bool CreateColumnBase(BaseObject* &columnBaseRelPtr, const Float &baseHeightRatio, const Float &baseRadiusHeightRatio, const Int32* objSegsPtr /*= nullptr*/)
	{

		AutoAlloc<BaseObject> columnBasePtr(Ocylinder);
		if (!columnBasePtr)
			return false;

		//	Set object names accordingly to their function.
		columnBasePtr->SetName("ColumnBase");

		//	Create a smooth Phong appearance and assign it to the column base.
		columnBasePtr->MakeTag(Tphong);

		//	Define the column base sizes to stick with the height ratio above
		//	and with a proper base radius.
		const Float baseRadius = baseHeightRatio * baseRadiusHeightRatio;
		columnBasePtr->SetParameter(PRIM_CYLINDER_HEIGHT, baseHeightRatio, DESCFLAGS_SET_0);
		columnBasePtr->SetParameter(PRIM_CYLINDER_RADIUS, baseRadius, DESCFLAGS_SET_0);


		//	Assign the segmentation data to the steam, base and capital
		//	to allow proper deforming operators to run
		if (nullptr != objSegsPtr)
			columnBasePtr->SetParameter(PRIM_CYLINDER_HSUB, objSegsPtr[1], DESCFLAGS_SET_0);

		columnBaseRelPtr = columnBasePtr.Release();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	/// Global function responsible to create the stem of the temple column.
	/// @brief Function responsible to create the stem of the temple column.
	/// @param[out] columnStemRelPtr			Column stem object pointer. @callerOwnsPointed{base object}
	/// @param[in] stemHeightRatio				Stem height ratio.
	/// @param[in] stemRadiusHeightRatio	Stem side/height ratio.
	/// @param[in] objSegsPtr							Segmentation array.
	/// @return														True if building process succeeds.
	//------------------------------------------------------------------------------------------------
	const Bool CreateColumnStem(BaseObject* &columnStemRelPtr, const Float &stemHeightRatio, const Float &stemRadiusHeightRatio, const Int32* objSegsPtr = nullptr);
	const Bool CreateColumnStem(BaseObject* &columnStemRelPtr, const Float &stemHeightRatio, const Float &stemRadiusHeightRatio, const Int32* objSegsPtr /*= nullptr*/)
	{
		AutoAlloc<BaseObject> columnStemPtr(Ocylinder);
		if (!columnStemPtr)
			return false;

		//	Set object names accordingly to their function.
		columnStemPtr->SetName("ColumnStem");

		const Float stemRadius = stemHeightRatio * stemRadiusHeightRatio;
		columnStemPtr->SetParameter(PRIM_CYLINDER_HEIGHT, stemHeightRatio, DESCFLAGS_SET_0);
		columnStemPtr->SetParameter(PRIM_CYLINDER_RADIUS, stemRadius, DESCFLAGS_SET_0);
		columnStemPtr->SetParameter(PRIM_CYLINDER_SEG, 16, DESCFLAGS_SET_0);

		//	Assign the segmentation data to the steam, base and capital
		//	to allow proper deforming operators to run
		if (nullptr != objSegsPtr)
		{
			columnStemPtr->SetParameter(PRIM_CYLINDER_HSUB, objSegsPtr[1], DESCFLAGS_SET_0);
		}

		//	Create two taper modifiers to get the typical look of the column
		AutoAlloc<BaseObject> stemTaper(Otaper);
		if (!stemTaper)
			return false;

		//	Set modifiers name accordingly to their function
		stemTaper->SetName("StemTaper");

		const Vector stemTaperSizes = Vector(stemRadius * 2, stemHeightRatio, stemRadius * 2);
		stemTaper->SetParameter(DEFORMOBJECT_SIZE, stemTaperSizes, DESCFLAGS_SET_0);
		stemTaper->SetParameter(DEFORMOBJECT_STRENGTH, .15, DESCFLAGS_SET_0);
		stemTaper->SetParameter(DEFORMOBJECT_CURVATURE, 0, DESCFLAGS_SET_0);

		//	Hide the deformer cage from the viewport
		stemTaper->SetParameter(ID_BASEOBJECT_VISIBILITY_EDITOR, OBJECT_OFF, DESCFLAGS_SET_0);
		stemTaper.Release()->InsertUnder(columnStemPtr);

		columnStemRelPtr = columnStemPtr.Release();

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/// Global function responsible to create the colonnade of the temple using cylinders and bent
	/// cubes and responsible for managing the columns grid arrangement.
	/// @brief Global function to create the temple colonnade managing columns grid arrangement.
	/// @param[out] colonnadeNullRelPtr	Colonnade object. @callerOwnsPointed{base object}
	/// @param[in] columnNullRelPtr			Column object to be instanced.
	/// @param[in] topStepWidth					Width of the top-most step.
	/// @param[in] topStepHeight				Height of the top-most step.
	/// @param[in] topStepBLVertexPos		Position of the bottom-left vertex of the top-most step.
	/// @param[in] columnRadius					Column bounding-box sizes.
	/// @param[in] columnScaleVector		Scaling factor based on the overall temple sizes.
	/// @param[in] xSpace								Space between columns along the x-axis.
	/// @param[in] zSpace								Space between columns along the z-axis.
	/// @param[in] hhPtr								A hierarchy helper for the operation. @callerOwnsPointed{hierarchy helper}
	/// @return													True if instancing process succeeds.
	//------------------------------------------------------------------------------------------------
	const Bool PerformeColumnsInstancing(BaseObject* &colonnadeNullRelPtr, BaseObject* columnNullRelPtr, const Float& topStepWidth, const Float& topStepHeight, const Vector& topStepBLVertexPos, const Vector& columnRadius, const Vector& columnScaleVector, const Float xSpace = 0, const Float zSpace = 0, HierarchyHelp* hhPtr = nullptr);
	const Bool PerformeColumnsInstancing(BaseObject* &colonnadeNullRelPtr, BaseObject* columnNullRelPtr, const Float& topStepWidth, const Float& topStepHeight, const Vector& topStepBLVertexPos, const Vector& columnRadius, const Vector& columnScaleVector, const Float xSpace /*= 0*/, const Float zSpace /*= 0*/, HierarchyHelp* hhPtr /*= nullptr*/)
	{
		//	Create a null object acting as parent of the column parts
		AutoAlloc<BaseObject> colonnadeNullPtr(Onull);
		if (!colonnadeNullPtr)
			return false;

		//	Set null object names accordingly to their function
		colonnadeNullPtr->SetName("Colonnade");

		//	Temporary center the bottom/left column with the
		//	bottom/left vertex of the top step
		Vector colPosBL = topStepBLVertexPos;

		AutoAlloc<BaseLink> baseLinkPtr;
		GeData baseLinkData;

		if (!baseLinkPtr)
			return false;

		//	Set the baselink data to point to the columnNull object
		baseLinkPtr->SetLink(columnNullRelPtr);
		baseLinkData.SetBaseLink(baseLinkPtr);

		//	Compute the number of columns in x and z based on the space available,
		//	the column radius and the offset between two adjacent columns
		Float columnCopiesOffsetX, columnCopiesOffsetZ;

		if (xSpace == 0)
			columnCopiesOffsetX = 3 * columnRadius.x;
		else
			columnCopiesOffsetX = xSpace;

		if (zSpace == 0)
			columnCopiesOffsetZ = 3 * columnRadius.z;
		else
			columnCopiesOffsetZ = zSpace;

		const Int32 columnCopiesX = SAFEINT32((topStepWidth - columnRadius.x * 2) / columnCopiesOffsetX);
		const Int32 columnCopiesZ = SAFEINT32((topStepHeight - columnRadius.z * 2) / columnCopiesOffsetZ);

		//	Compute the remaining space from the rounding occurred
		//	by casting float to int
		const Float remainingSpaceX = topStepWidth - columnCopiesX * columnCopiesOffsetX - columnRadius.x * 2;
		const Float remainingSpaceZ = topStepHeight - columnCopiesZ  * columnCopiesOffsetZ - columnRadius.z * 2;

		//	Correct the position of the first bottom/left column
		//	to have all the instances centered in the origin
		//	by adding for the x and z axis both the column radius
		//	and the empty margin available (divided by two)
		colPosBL.x += columnRadius.x + remainingSpaceX * 0.5;
		colPosBL.z += columnRadius.z + remainingSpaceZ * 0.5;
		columnNullRelPtr->SetRelPos(colPosBL);

		for (Int32 i = 0; i <= columnCopiesX; ++i)
		{
			for (Int32 j = 0; j <= columnCopiesZ; ++j)
			{
				if (nullptr != hhPtr && nullptr != hhPtr->GetThread())
				{
					BaseThread* runningThreadPtr = hhPtr->GetThread();
					if (runningThreadPtr->TestBreak())
					{
						GePrint("Object generation halted by user interaction.");
						return false;
					}
				}

				//	Create a instance object to fill up the rectangular pattern with the
				//	columns instances
				AutoAlloc<BaseObject> columnInstPtr(Oinstance);
				if (!columnInstPtr)
					return false;

				// Set column instance name accordingly to the i,j indexes
				columnInstPtr->SetName("Column_" + String::IntToString(i) + "_" + String::IntToString(j));

				//	Connect the baseLinkData
				columnInstPtr->SetParameter(INSTANCEOBJECT_LINK, baseLinkData, DESCFLAGS_SET_0);

				//	Set the position of the different instances based on a rectangular pattern
				const Vector currentInstancePos = colPosBL + Vector(i*columnCopiesOffsetX, 0, j*columnCopiesOffsetZ);
				columnInstPtr->SetRelPos(currentInstancePos);

				//	Scale the instance to match the parent column
				columnInstPtr->SetAbsScale(columnScaleVector);

				//	Avoid inserting an instance at the same location of the parent column
				if (!(i == 0 && j == 0))
					columnInstPtr.Release()->InsertUnderLast(colonnadeNullPtr);
			}
		}

		columnNullRelPtr->InsertUnder(colonnadeNullPtr);

		colonnadeNullRelPtr = colonnadeNullPtr.Release();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	/// Global function responsible to create the colonnade of the temple using cylinders and bent
	/// cubes and responsible for managing the columns grid arrangement.
	/// @brief Global function to create the temple colonnade managing columns grid arrangement.
	/// @param[out] parentObj			Object provided to store created geometry. @callerOwnsPointed{base object}
	/// @param[in] baseUnit				Base value for geometry construction.
	/// @param[in] objSize				Bounding box radius vector.
	/// @param[in] objSegsPtr			Segmentation array.
	/// @param[in] xSpace					Space between columns along the x-axis.
	/// @param[in] zSpace					Space between columns along the z-axis.
	/// @param[in] hhPtr					A hierarchy helper for the operation. @callerOwnsPointed{hierarchy helper}
	/// @return										True if building process succeeds.
	//------------------------------------------------------------------------------------------------
	const Bool CreateTempleColonnade(BaseObject* &parentObj, const Float& baseUnit, const Vector& objSize, const Int32* objSegsPtr = nullptr, const Float xSpace = 0, const Float zSpace = 0, HierarchyHelp* hhPtr = nullptr);
	const Bool CreateTempleColonnade(BaseObject* &parentObj, const Float& baseUnit, const Vector& objSize, const Int32* objSegsPtr /*= nullptr*/, const Float xSpace /*= 0*/, const Float zSpace /*= 0*/, HierarchyHelp* hhPtr /*= nullptr*/)
	{
		//	Create a column composed by three element the base, the stem and the
		//	capital with sizes in the following ratio (.1 - .75 - .15).
		const Float baseHeightRatio = (Float)0.1;
		const Float baseRadiusHeightRatio = (Float)1.666;
		const Float stemHeightRatio = (Float)0.75;
		const Float stemRadiusHeightRatio = (Float)0.15;
		const Float capitalHeightRatio = (Float).15;
		const Float capitalSideHeightRatio = (Float)1.5;

		BaseObject *columnBasePtr = nullptr, *columnStemPtr = nullptr, *columnCapitalPtr = nullptr;

		CreateColumnBase(columnBasePtr, baseHeightRatio, baseRadiusHeightRatio, objSegsPtr);
		if (!columnBasePtr)
			return false;

		CreateColumnStem(columnStemPtr, stemHeightRatio, stemRadiusHeightRatio, objSegsPtr);
		if (!columnStemPtr)
		{
			BaseObject::Free(columnBasePtr);
			return false;
		}
		CreateColumnCapital(columnCapitalPtr, capitalHeightRatio, capitalSideHeightRatio, objSegsPtr);
		if (!columnCapitalPtr)
		{
			BaseObject::Free(columnBasePtr);
			BaseObject::Free(columnStemPtr);
			return false;
		}

		AutoAlloc<BaseObject> columnNullPtr(Onull);
		if (!columnNullPtr)
		{
			BaseObject::Free(columnBasePtr);
			BaseObject::Free(columnStemPtr);
			BaseObject::Free(columnCapitalPtr);
			return false;
		}
		//	Set null object names accordingly to their function
		columnNullPtr->SetName("Column_0_0");

		columnCapitalPtr->SetRelPos(Vector(0, stemHeightRatio * 0.5 + capitalHeightRatio * 0.5, 0));
		columnCapitalPtr->InsertUnder(columnStemPtr);
		columnStemPtr->SetRelPos(Vector(0, baseHeightRatio * 0.5 + stemHeightRatio * 0.5, 0));
		columnStemPtr->InsertUnder(columnBasePtr);
		columnBasePtr->SetRelPos(Vector(0, baseHeightRatio * 0.5, 0));
		columnBasePtr->InsertUnder(columnNullPtr);

		//	Calculate the height as the difference between the thickness of the
		//	three steps and the height of the roof
		const Float triCircumCircleRadius = baseUnit * 3;
		const Float triHeight = triCircumCircleRadius * 1.5;
		const Float heightScaleComponent = objSize.y - (3 * baseUnit + triHeight);
		const Vector columnScaleVector(heightScaleComponent, heightScaleComponent, heightScaleComponent);
		columnNullPtr->SetAbsScale(columnScaleVector);

		//	Calculate the distance of the highest of the three floor and
		//	reposition the column accordingly to the bottom-left vertex
		//	shifted of the base radius
		const Float topStepWidth = objSize.x - 4 * baseUnit;
		const Float topStepHeight = objSize.z - 4 * baseUnit;
		const Vector topStepBLVertexPos(-topStepWidth * .5, -objSize.y * .5 + baseUnit * 3, -topStepHeight * .5);

		//	Compute the columnRadius bounding box
		const Float columnRadiusX = baseHeightRatio * baseRadiusHeightRatio * heightScaleComponent;
		const Float columnRadiusZ = baseHeightRatio * baseRadiusHeightRatio * heightScaleComponent;

		const Vector columnRadius(columnRadiusX, heightScaleComponent, columnRadiusZ);
		
		BaseObject* colonnadeNullPtr = nullptr;
		BaseObject* columnNullRelPtr = columnNullPtr.Release();

		if (!columnNullRelPtr)
			return false;

		// Perform the columnade instancing through a sub-procedure;
		PerformeColumnsInstancing(colonnadeNullPtr, columnNullRelPtr, topStepWidth, topStepHeight, topStepBLVertexPos, columnRadius, columnScaleVector, xSpace, zSpace, hhPtr);

		if (!colonnadeNullPtr)
		{
			BaseObject::Free(columnNullRelPtr);
			return false;
		}

		colonnadeNullPtr->InsertUnder(parentObj);

		return true;
	}
}

//------------------------------------------------------------------------------------------------
/// ObjectData implementation generating a parameter-based Greek-stylized temple. 
///
/// The example makes extended use of different shapes, basic generators, deformers and advanced 
/// generators nested properly to deliver the final temple shape. Exposed parameters permit to 
/// change the number of columns, steps, column-distances and overall temple sizes in order to 
/// create customized temples. 
//------------------------------------------------------------------------------------------------
class GreekTemple : public ObjectData
{
	INSTANCEOF(GreekTemple, ObjectData)

public:
	static NodeData* Alloc(void){ return NewObjClear(GreekTemple); }
	virtual Bool Init(GeListNode* node);
	virtual void GetDimension(BaseObject* op, Vector* mp, Vector* rad);
	virtual BaseObject* GetVirtualObjects(BaseObject* op, HierarchyHelp* hh);
};

/// @name ObjectData functions
/// @{
Bool GreekTemple::Init(GeListNode* node)
{
	//	Retrieve the BaseContainer object belonging to the generator.
	BaseObject* baseObjectPtr = (BaseObject*)node;
	BaseContainer* objectDataPtr = baseObjectPtr->GetDataInstance();

	//	Fill the retrieve BaseContainer object with initial values.
	objectDataPtr->SetFloat(SDK_EXAMPLE_GREEKTEMPLE_WIDTH, 300);
	objectDataPtr->SetFloat(SDK_EXAMPLE_GREEKTEMPLE_HEIGHT, 200);
	objectDataPtr->SetFloat(SDK_EXAMPLE_GREEKTEMPLE_LENGTH, 400);
	objectDataPtr->SetInt32(SDK_EXAMPLE_GREEKTEMPLE_X_SEGMENTS, 1);
	objectDataPtr->SetInt32(SDK_EXAMPLE_GREEKTEMPLE_Y_SEGMENTS, 1);
	objectDataPtr->SetInt32(SDK_EXAMPLE_GREEKTEMPLE_Z_SEGMENTS, 1);
	objectDataPtr->SetInt32(SDK_EXAMPLE_GREEKTEMPLE_STAIRS, 3);
	objectDataPtr->SetFloat(SDK_EXAMPLE_GREEKTEMPLE_COLS_SPACEX, 50);
	objectDataPtr->SetFloat(SDK_EXAMPLE_GREEKTEMPLE_COLS_SPACEZ, 50);

	return true;
}

void GreekTemple::GetDimension(BaseObject* op, Vector* mp, Vector* rad)
{
	//	Reset the barycenter position and the bbox radius vector.
	mp->SetZero();
	rad->SetZero();

	//	Check the passed pointer.
	if (nullptr == op)
		return;

	//	Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* objectDataPtr = op->GetDataInstance();
	if (nullptr == objectDataPtr)
		return;

	//	Set radius values accordingly to the bbox values stored during the init.
	rad->x = objectDataPtr->GetFloat(SDK_EXAMPLE_GREEKTEMPLE_WIDTH) * 0.5;
	rad->y = objectDataPtr->GetFloat(SDK_EXAMPLE_GREEKTEMPLE_HEIGHT) * 0.5;
	rad->z = objectDataPtr->GetFloat(SDK_EXAMPLE_GREEKTEMPLE_LENGTH) * 0.5;

	mp->y = objectDataPtr->GetFloat(SDK_EXAMPLE_GREEKTEMPLE_HEIGHT) * 0.5;
}

BaseObject* GreekTemple::GetVirtualObjects(BaseObject* op, HierarchyHelp* hh)
{
	//	Check the passed pointer.
	if (nullptr == op)
		return BaseObject::Alloc(Onull);

	//	Verify if object cache already exist and check its status.
	Bool bIsDirty = op->CheckCache(hh) || op->IsDirty(DIRTYFLAGS_DATA);

	//	In case it's not dirty return the cache data without doing any calculation.
	if (!bIsDirty)
		return op->GetCache(hh);

	//	Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* objectDataPtr = op->GetDataInstance();
	if (nullptr == objectDataPtr)
		return BaseObject::Alloc(Onull);

	//	Retrieve the bbox values.
	Vector bbox;
	bbox[0] = objectDataPtr->GetFloat(SDK_EXAMPLE_GREEKTEMPLE_WIDTH);
	bbox[1] = objectDataPtr->GetFloat(SDK_EXAMPLE_GREEKTEMPLE_HEIGHT);
	bbox[2] = objectDataPtr->GetFloat(SDK_EXAMPLE_GREEKTEMPLE_LENGTH);

	if (bbox.x == 0 || bbox.y == 0 || bbox.z == 0)
		return BaseObject::Alloc(Onull);

	//	Retrieve the segmentation values.
	Int32 segsPtr[3];
	segsPtr[0] = objectDataPtr->GetInt32(SDK_EXAMPLE_GREEKTEMPLE_X_SEGMENTS);
	segsPtr[1] = objectDataPtr->GetInt32(SDK_EXAMPLE_GREEKTEMPLE_Y_SEGMENTS);
	segsPtr[2] = objectDataPtr->GetInt32(SDK_EXAMPLE_GREEKTEMPLE_Z_SEGMENTS);

	//	Retrieve the stairs count.
	const Int32 stairsCount = objectDataPtr->GetInt32(SDK_EXAMPLE_GREEKTEMPLE_STAIRS);

	//	Retrieve the space between columns on x and z axis.
	const Float colSpaceAlongX = objectDataPtr->GetFloat(SDK_EXAMPLE_GREEKTEMPLE_COLS_SPACEX);
	const Float colSpaceAlongZ = objectDataPtr->GetFloat(SDK_EXAMPLE_GREEKTEMPLE_COLS_SPACEZ);

	// Create a null object to be used as dummy object for the temple.
	BaseObject* templeObjPtr = BaseObject::Alloc(Onull);
	if (nullptr == templeObjPtr)
		return BaseObject::Alloc(Onull);

	//	Establish the temple base unit.
	const Float baseUnit = bbox.GetMin() * .05;

	//	Create the base.
	Bool res = GreekTempleHelpers::CreateTempleBase(templeObjPtr, baseUnit, bbox, segsPtr, stairsCount);
	if (!res)
		return templeObjPtr;

	// Create the roof.
	res = GreekTempleHelpers::CreateTempleRoof(templeObjPtr, baseUnit, bbox, segsPtr);
	if (!res)
		return templeObjPtr;

	// Create the colonnade.
	res = GreekTempleHelpers::CreateTempleColonnade(templeObjPtr, baseUnit, bbox, segsPtr, colSpaceAlongX, colSpaceAlongZ);
	if (!res)
		return templeObjPtr;
	
	templeObjPtr->SetAbsPos(Vector(0, bbox.y * 0.5, 0));

	return templeObjPtr;
}
/// @}

Bool RegisterGreekTemple()
{
	String registeredName = GeLoadString(IDS_OBJECTDATA_GREEKTEMPLE);
	if (!registeredName.Content() || registeredName == "StrNotFound")
		registeredName = "C++ SDK - Greek Temple Generator Example";

	return RegisterObjectPlugin(ID_SDKEXAMPLE_OBJECTDATA_GREEKTEMPLE, registeredName, OBJECT_GENERATOR, GreekTemple::Alloc, "ogreektemple", AutoBitmap("greektemple.tif"), 0);
}