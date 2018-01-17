#include "c4d_basedraw.h"
#include "c4d_particleobject.h"
#include "c4d_objectdata.h"
#include "c4d_tools.h"

// Includes from cinema4dsdk
#include "c4d_symbols.h"
#include "main.h"

// Local resources
#include "oshufflingparticles.h"

/**A unique plugin ID. You must obtain this from http://www.plugincafe.com. Use this ID to create new instances of this object.*/
static const Int32 ID_SDKEXAMPLE_OBJECTDATA_SHUFFLINGPARTICLES = 1038238;

namespace ShufflingParticlesHelpers
{
	//------------------------------------------------------------------------------------------------
	/// Global helper function to draw a cube.
	/// @brief Global helper function to draw a cube.
	/// @param[in] bd 				The passed BaseDraw instance.
	/// @param[in] cubeSizes 	The vector containing the cube sizes.
	//------------------------------------------------------------------------------------------------
	static void DrawCube(BaseDraw& bd, const Vector& cubeSizes);
	static void DrawCube(BaseDraw& bd, const Vector& cubeSizes)
	{
		maxon::BaseArray<Vector> cubeVertices; 
		cubeVertices.Resize(8);
		
		// Vertexes on the bottom face
		cubeVertices[0] = Vector(-cubeSizes.x, -cubeSizes.y, -cubeSizes.z);
		cubeVertices[1] = Vector(cubeSizes.x, -cubeSizes.y, -cubeSizes.z);
		cubeVertices[2] = Vector(cubeSizes.x, cubeSizes.y, -cubeSizes.z);
		cubeVertices[3] = Vector(-cubeSizes.x, cubeSizes.y, -cubeSizes.z);

		// Vertexes on the top face
		cubeVertices[4] = Vector(-cubeSizes.x, -cubeSizes.y, cubeSizes.z);
		cubeVertices[5] = Vector(cubeSizes.x, -cubeSizes.y, cubeSizes.z);
		cubeVertices[6] = Vector(cubeSizes.x, cubeSizes.y, cubeSizes.z);
		cubeVertices[7] = Vector(-cubeSizes.x, cubeSizes.y, cubeSizes.z);

		// Draw the top face lines
		bd.DrawLine(cubeVertices[0], cubeVertices[1], NOCLIP_D);
		bd.DrawLine(cubeVertices[1], cubeVertices[2], NOCLIP_D);
		bd.DrawLine(cubeVertices[2], cubeVertices[3], NOCLIP_D);
		bd.DrawLine(cubeVertices[3], cubeVertices[0], NOCLIP_D);

		// Draw the bottom face lines
		bd.DrawLine(cubeVertices[4], cubeVertices[5], NOCLIP_D);
		bd.DrawLine(cubeVertices[5], cubeVertices[6], NOCLIP_D);
		bd.DrawLine(cubeVertices[6], cubeVertices[7], NOCLIP_D);
		bd.DrawLine(cubeVertices[7], cubeVertices[4], NOCLIP_D);

		// Draw the four connecting vertical lines
		bd.DrawLine(cubeVertices[0], cubeVertices[4], NOCLIP_D);
		bd.DrawLine(cubeVertices[1], cubeVertices[5], NOCLIP_D);
		bd.DrawLine(cubeVertices[2], cubeVertices[6], NOCLIP_D);
		bd.DrawLine(cubeVertices[3], cubeVertices[7], NOCLIP_D);
	}

	//------------------------------------------------------------------------------------------------
	/// Global helper function to draw a cube.
	/// @brief Global helper function to draw a cube.
	/// @param[in] partModGlobalMatrixInv		The reference to the modifier inverted global matrix.
	/// @param[in] particleGlobalPosition		The vector containing the global particle position.
	/// @param[in] partModSizes							The reference to the vector representing the modifier's size.
	/// @return 														True if particle is contained in the modifier box.
	//------------------------------------------------------------------------------------------------
	static Bool IsParticleInObjectBoundaries(const Matrix& partModGlobalMatrixInv, const Vector& particleGlobalPosition, const Vector& partModSizes);
	static Bool IsParticleInObjectBoundaries(const Matrix& partModGlobalMatrixInv, const Vector& particleGlobalPosition, const Vector& partModSizes)
	{
		// Convert the offset vector of the particle from world space to local (modifier) space
		const Vector particleLocalModPosition = partModGlobalMatrixInv * particleGlobalPosition;

		// Check the position of the particle against the space occupied by the modifier
		// NOTE:using the coordinate space conversion operated above it's not relevant to know the
		// position of the particle in the world space and compare it with the position of the object
		// plus/minus its boundings; using the particles position in the local space of the modifier
		// it's enough to check them against the size of the modifier in the space.
		const Bool IsParticleInsideModifierOnX = ((particleLocalModPosition.x >= -partModSizes.x) && (particleLocalModPosition.x <= partModSizes.x));
		const Bool IsParticleInsideModifierOnY = ((particleLocalModPosition.y >= -partModSizes.y) && (particleLocalModPosition.y <= partModSizes.y));
		const Bool IsParticleInsideModifierOnZ = ((particleLocalModPosition.z >= -partModSizes.z) && (particleLocalModPosition.z <= partModSizes.z));

		return (IsParticleInsideModifierOnX && IsParticleInsideModifierOnY && IsParticleInsideModifierOnZ);
	}
}

//------------------------------------------------------------------------------------------------
/// ObjectData implementation providing particles deformation. It shuffles particle trajectories 
/// based on the seed parameter and on the trajectory deformation strength. 
//------------------------------------------------------------------------------------------------
class ShufflingParticles : public ObjectData
{
	INSTANCEOF(ShufflingParticles, ObjectData)

public:
	static NodeData* Alloc(void){ return NewObjClear(ShufflingParticles); }
	virtual Bool Init(GeListNode* node);
	virtual void GetDimension(BaseObject* op, Vector* mp, Vector* rad);
	virtual void ModifyParticles(BaseObject* op, Particle* pp, BaseParticle* ss, Int32 pcnt, Float diff);
	virtual DRAWRESULT Draw(BaseObject* op, DRAWPASS drawpass, BaseDraw* bd, BaseDrawHelp* bh);
};

/// @name ObjectData functions
/// @{
Bool ShufflingParticles::Init(GeListNode* node)
{
	if (!node)
		return false;

	//	Retrieve the BaseContainer object belonging to the generator.
	BaseObject* baseObjectPtr = static_cast<BaseObject*>(node);

	BaseContainer* bcPtr = baseObjectPtr->GetDataInstance();

	if (!bcPtr)
		return false;

	//	Fill the retrieve BaseContainer object with initial values.
	bcPtr->SetInt32(SDK_EXAMPLE_SHUFFLINGPARTICLES_SEED, 1);
	bcPtr->SetInt32(SDK_EXAMPLE_SHUFFLINGPARTICLES_STRENGTH, 50);
	bcPtr->SetFloat(SDK_EXAMPLE_SHUFFLINGPARTICLES_SIZE_X, 100);
	bcPtr->SetFloat(SDK_EXAMPLE_SHUFFLINGPARTICLES_SIZE_Y, 100);
	bcPtr->SetFloat(SDK_EXAMPLE_SHUFFLINGPARTICLES_SIZE_Z, 100);

	return true;
}

DRAWRESULT ShufflingParticles::Draw(BaseObject* op, DRAWPASS drawpass, BaseDraw* bd, BaseDrawHelp* bh)
{
	if (!op || !bd || !bh)
		return DRAWRESULT_SKIP;

	// Retrieve the object world transformation matrix
	const Matrix modifierWorldMatrix = op->GetMg();

	// Set the BaseDraw instance coordinates transformation 
	bd->SetMatrix_Matrix(op, modifierWorldMatrix);

	// Set the BaseDraw instance color to match the object color
	bd->SetPen(bd->GetObjectColor(bh, op));

	Vector rad = op->GetRad();
	ShufflingParticlesHelpers::DrawCube(*bd, rad);

	return SUPER::Draw(op, drawpass, bd, bh);
}

void ShufflingParticles::GetDimension(BaseObject* op, Vector* mp, Vector* rad)
{
	// Check the passed pointers.
	if (!op || !mp || !rad)
		return;

	//	Reset the barycenter position and the bounding box radius vector.
	mp->SetZero();
	rad->SetZero();

	// Set the barycenter position to match the generator center.
	const Vector objGlobalOffset = op->GetMg().off;
	mp->x = objGlobalOffset.x;
	mp->y = objGlobalOffset.y;
	mp->z = objGlobalOffset.z;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* bcPtr = op->GetDataInstance();
	if (!bcPtr)
		return;

	// Set radius values accordingly to arbitrary default value (they won't be used).
	rad->x = bcPtr->GetFloat(SDK_EXAMPLE_SHUFFLINGPARTICLES_SIZE_X);
	rad->y = bcPtr->GetFloat(SDK_EXAMPLE_SHUFFLINGPARTICLES_SIZE_Y);
	rad->z = bcPtr->GetFloat(SDK_EXAMPLE_SHUFFLINGPARTICLES_SIZE_Z);
}

void ShufflingParticles::ModifyParticles(BaseObject* op, Particle* pp, BaseParticle* ss, Int32 pcnt, Float diff)
{
	// Check the passed pointers
	if (!op || !pp || !ss)
		return;

	BaseContainer* bcPtr = op->GetDataInstance();

	// Check the retrieved BaseContainer instance belonging to the particle modifier.
	if (!bcPtr)
		return;

	// Retrieve the particles modifiers parameters value
	const Int32 seedValue = bcPtr->GetInt32(SDK_EXAMPLE_SHUFFLINGPARTICLES_SEED);
	const Int32 strengthValue = bcPtr->GetInt32(SDK_EXAMPLE_SHUFFLINGPARTICLES_STRENGTH);

	// Retrieve the particles modifier sizes.
	const Float sizeX = bcPtr->GetFloat(SDK_EXAMPLE_SHUFFLINGPARTICLES_SIZE_X);
	const Float sizeY = bcPtr->GetFloat(SDK_EXAMPLE_SHUFFLINGPARTICLES_SIZE_Y);
	const Float sizeZ = bcPtr->GetFloat(SDK_EXAMPLE_SHUFFLINGPARTICLES_SIZE_Z);
	const Vector partModSizes(sizeX, sizeY, sizeZ);

	// Retrieve the particles modifier world transformation matrix and its inverse.
	const Matrix partModGlobalMatrix(op->GetMg());
	const Matrix partModGlobalMatrixInv(~partModGlobalMatrix);

	// Instantiate a random generator
	Random randomGen;

	// Init the generator based on the modifier's SDK_EXAMPLE_SHUFFLINGPARTICLES_SEED parameter
	randomGen.Init(seedValue);

	for (Int32 i = 0; i < pcnt; ++i)
	{
		// Retrieve meaningful data of the current particle.
		const Vector particleGlobalPos = pp[i].off;
		const PARTICLEFLAGS particleFlag = pp[i].bits;

		// Skip those particles which are no more alive
		if (!(particleFlag & PARTICLEFLAGS_ALIVE))
			continue;

		// If particle is inside the modifier occupied space then modify the particle somehow.
		if (ShufflingParticlesHelpers::IsParticleInObjectBoundaries(partModGlobalMatrixInv, particleGlobalPos, partModSizes))
		{
			// Generate a random direction vector
			const Vector randomDir(randomGen.Get11(), randomGen.Get11(), randomGen.Get11());
			// Update the velocity vector of the i-th particle accordingly to the random dir.
			ss[i].v += pp[i].v3 + randomDir * strengthValue;
			// Increase the velocity sum counter.
			ss[i].count += 1;
		}
	}
}
/// @}

Bool RegisterParticlesShuffling()
{
	String registeredName = GeLoadString(IDS_OBJECTDATA_PARTICLESSHUFFLING);
	if (!registeredName.Content() || registeredName == "StrNotFound")
		registeredName = "C++ SDK - Particles Shuffling Modifier Example";

	return RegisterObjectPlugin(ID_SDKEXAMPLE_OBJECTDATA_SHUFFLINGPARTICLES, registeredName, OBJECT_PARTICLEMODIFIER, ShufflingParticles::Alloc, "oshufflingparticles", AutoBitmap("particlesshuffling.tif"), 0);
}