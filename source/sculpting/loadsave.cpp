// Sculpt Data import and export

#include <stdio.h>
#include <string.h>
#include "c4d.h"
#include "c4d_symbols.h"
#include "lib_sculpt.h"
#include "main.h"

#define FILE_VERSION 0

class SculptLoaderData : public SceneLoaderData
{
public:
	virtual Bool Init(GeListNode* node);
	virtual Bool Identify(BaseSceneLoader* node, const Filename& name, UChar* probe, Int32 size);
	virtual FILEERROR Load(BaseSceneLoader* node, const Filename& name, BaseDocument* doc, SCENEFILTER filterflags, String* error, BaseThread* bt);

	static NodeData* Alloc() { return NewObjClear(SculptLoaderData); }
};

Bool SculptLoaderData::Init(GeListNode* node)
{
	return true;
}

class SculptSaverData : public SceneSaverData
{
public:
	virtual Bool Init(GeListNode* node);
	virtual FILEERROR Save(BaseSceneSaver* node, const Filename& name, BaseDocument* doc, SCENEFILTER filterflags);

	static NodeData* Alloc() { return NewObjClear(SculptSaverData); }
};

Bool SculptSaverData::Init(GeListNode* node)
{
	return true;
}

Bool SculptLoaderData::Identify(BaseSceneLoader* node, const Filename& name, UChar* probe, Int32 size)
{
	if (!name.CheckSuffix("scp"))
		return false;
	return true;
}

FILEERROR SculptLoaderData::Load(BaseSceneLoader* node, const Filename& name, BaseDocument* doc, SCENEFILTER flags, String* error, BaseThread* thread)
{
	if (!(flags & SCENEFILTER_OBJECTS))
		return FILEERROR_NONE;

	AutoAlloc<BaseFile> file;
	if (!file)
		return FILEERROR_OUTOFMEMORY;
	if (!file->Open(name, FILEOPEN_READ, FILEDIALOG_NONE, BYTEORDER_INTEL))
		return file->GetError();

	Int32 version = 0;
	Int32 polyCount	 = 0;
	Int32 pointCount = 0;
	if (!file->ReadInt32(&version))
		return file->GetError();

	if (version < FILE_VERSION)
		return FILEERROR_READ;

	if (!file->ReadInt32(&polyCount))
		return file->GetError();
	if (!file->ReadInt32(&pointCount))
		return file->GetError();

	PolygonObject* pPoly = PolygonObject::Alloc(pointCount, polyCount);
	if (!pPoly)
		return FILEERROR_OUTOFMEMORY;

	Int32			a;
	CPolygon* pPolys = pPoly->GetPolygonW();
	for (a = 0; a < polyCount; a++)
	{
		CPolygon& p = pPolys[a];
		if (!file->ReadInt32(&p.a))
			return file->GetError();
		if (!file->ReadInt32(&p.b))
			return file->GetError();
		if (!file->ReadInt32(&p.c))
			return file->GetError();
		if (!file->ReadInt32(&p.d))
			return file->GetError();
	}

	Vector* pPoints = pPoly->GetPointW();
	for (a = 0; a < pointCount; a++)
	{
		// Using real instead of vector to match the Melange library
		if (!file->ReadFloat64(&pPoints[a].x))
			return file->GetError();
		if (!file->ReadFloat64(&pPoints[a].y))
			return file->GetError();
		if (!file->ReadFloat64(&pPoints[a].z))
			return file->GetError();
	}

	doc->InsertObject(pPoly, nullptr, nullptr);

	SculptObject* pSculptObject = MakeSculptObject(pPoly, doc);
	if (!pSculptObject)
		return FILEERROR_INVALID;

	Int32 subdivisionCount = 0;
	if (!file->ReadInt32(&subdivisionCount))
		return file->GetError();

	for (a = 0; a <= subdivisionCount; a++)
	{
		SculptLayer* pBaseLayer = pSculptObject->GetBaseLayer();
		if (!pBaseLayer)
			return FILEERROR_OUTOFMEMORY;
		{
			Int32 level, layerPointCount;
			if (!file->ReadInt32(&level))
				return file->GetError();
			if (!file->ReadInt32(&layerPointCount))
				return file->GetError();

			if (level != a)
				return FILEERROR_INVALID;
			if (layerPointCount != pBaseLayer->GetPointCount())
				return FILEERROR_INVALID;

			// First read in the baselayer data for this level
			Int32 b;
			for (b = 0; b < layerPointCount; b++)
			{
				Vector p;
				// Using Float instead of Vector to match the Melange Library
				if (!file->ReadFloat64(&p.x))
					return file->GetError();
				if (!file->ReadFloat64(&p.y))
					return file->GetError();
				if (!file->ReadFloat64(&p.z))
					return file->GetError();
				pBaseLayer->SetOffset(b, p);
			}

			Int32 hasMask = false;
			if (!file->ReadInt32(&hasMask))
				return file->GetError();
			if (hasMask)
			{
				for (b = 0; b < layerPointCount; b++)
				{
					Float32 mask = 0;
					if (!file->ReadFloat32(&mask))
						return file->GetError();
					pBaseLayer->SetMask(b, mask);
				}
			}
		}

		Int32 layerCount = 0;
		if (!file->ReadInt32(&layerCount))
			return file->GetError();

		Int32 lay = 0;
		for (lay = 0; lay < layerCount; lay++)
		{
			Int32 type;
			if (!file->ReadInt32(&type))
				return file->GetError();
			if (type == SCULPT_FOLDER_ID)
			{

			}
			else if (type == SCULPT_LAYER_ID)
			{
				Int32 level;
				Int32 layerPointCount = 0;
				if (!file->ReadInt32(&level))
					return file->GetError();
				if (!file->ReadInt32(&layerPointCount))
					return file->GetError();

				String layerName;
				if (!file->ReadString(&layerName))
					return file->GetError();

				Int32 visible = true;
				if (!file->ReadInt32(&visible))
					return file->GetError();

				Float strength = 1.0;
				if (!file->ReadFloat64(&strength))
					return file->GetError();

				if (level != pSculptObject->GetCurrentLevel())
				{
					DebugAssert(false);
					return FILEERROR_INVALID;
				}

				SculptLayer* pLayer = pSculptObject->AddLayer();
				if (!pLayer)
					return FILEERROR_OUTOFMEMORY;

				pLayer->SetName(layerName);
				pLayer->SetStrength(strength);
				pLayer->SetVisible(visible);

				Int32 actualLayerPC = pLayer->GetPointCount();
				if (actualLayerPC != layerPointCount)
				{
					DebugAssert(false);
					return FILEERROR_INVALID;
				}

				Int32 b;
				for (b = 0; b < layerPointCount; b++)
				{
					Vector p;
					// Using Float instead of Vector to match the Melange Library
					if (!file->ReadFloat64(&p.x))
						return file->GetError();
					if (!file->ReadFloat64(&p.y))
						return file->GetError();
					if (!file->ReadFloat64(&p.z))
						return file->GetError();
					pLayer->SetOffset(b, p);
				}

				Int32 hasMask = false;
				if (!file->ReadInt32(&hasMask))
					return file->GetError();
				if (hasMask)
				{
					for (b = 0; b < layerPointCount; b++)
					{
						Float32 mask = 0;
						if (!file->ReadFloat32(&mask))
							return file->GetError();
						pLayer->SetMask(b, mask);
					}

					Int32 maskEnabled = false;
					if (!file->ReadInt32(&maskEnabled))
						return file->GetError();
					pLayer->SetMaskEnabled(maskEnabled);
				}
			}
		}

		pSculptObject->Update();

		if (a != subdivisionCount)
			pSculptObject->Subdivide();
	}

	file->Close();
	return file->GetError();
}

FILEERROR SculptSaverData::Save(BaseSceneSaver* node, const Filename& name, BaseDocument* doc, SCENEFILTER flags)
{
	if (!(flags & SCENEFILTER_OBJECTS))
		return FILEERROR_NONE;

	AutoAlloc<BaseFile> file;
	if (!file)
		return FILEERROR_OUTOFMEMORY;
	if (!file->Open(name, FILEOPEN_WRITE, FILEDIALOG_NONE, BYTEORDER_INTEL))
		return file->GetError();

	if (!file->WriteInt32(FILE_VERSION))
		return file->GetError();

	SculptObject* pSculpt = GetSelectedSculptObject(doc);
	if (pSculpt)
	{
		PolygonObject* pBaseMesh = pSculpt->GetOriginalObject();
		if (pBaseMesh)
		{
			Int32 polyCount	 = pBaseMesh->GetPolygonCount();
			Int32 pointCount = pBaseMesh->GetPointCount();
			if (!file->WriteInt32(polyCount))
				return file->GetError();
			if (!file->WriteInt32(pointCount))
				return file->GetError();

			if (polyCount > 0)
			{
				const CPolygon* pPolygons = pBaseMesh->GetPolygonR();
				Int32 a;
				for (a = 0; a < polyCount; a++)
				{
					if (!file->WriteInt32(pPolygons[a].a))
						return file->GetError();
					if (!file->WriteInt32(pPolygons[a].b))
						return file->GetError();
					if (!file->WriteInt32(pPolygons[a].c))
						return file->GetError();
					if (!file->WriteInt32(pPolygons[a].d))
						return file->GetError();
				}
			}

			if (pointCount > 0)
			{
				const Vector* pPoints = pBaseMesh->GetPointR();
				Int32					a;
				for (a = 0; a < pointCount; a++)
				{
					// Using Float instead of Vector to match the Melange Library
					if (!file->WriteFloat64(pPoints[a].x))
						return file->GetError();
					if (!file->WriteFloat64(pPoints[a].y))
						return file->GetError();
					if (!file->WriteFloat64(pPoints[a].z))
						return file->GetError();
				}
			}
		}

		Int32 subdivisionCount = pSculpt->GetSubdivisionCount();
		if (!file->WriteInt32(subdivisionCount))
			return file->GetError();

		Int32 a;
		for (a = 0; a <= subdivisionCount; a++)
		{
			SculptLayer* pBaseLayer = pSculpt->GetBaseLayer();
			if (pBaseLayer)
			{
				SculptLayerData* pLayerData = pBaseLayer->GetFirstSculptLayerData();
				while (pLayerData && pLayerData->GetSubdivisionLevel() != a)
				{
					pLayerData = (SculptLayerData*)pLayerData->GetNext();
				}
				if (pLayerData)
				{
					Int32 pointCount = pLayerData->GetPointCount();
					if (!file->WriteInt32(a))
						return file->GetError();
					if (!file->WriteInt32(pointCount))
						return file->GetError();
					Int32 b;
					for (b = 0; b < pointCount; b++)
					{
						Vector offset;
						pLayerData->GetOffset(b, offset);

						// Using real instead of vector to match the melange library
						if (!file->WriteFloat64(offset.x))
							return file->GetError();
						if (!file->WriteFloat64(offset.y))
							return file->GetError();
						if (!file->WriteFloat64(offset.z))
							return file->GetError();
					}

					Int32 hasMask = pLayerData->HasMask();
					if (!file->WriteInt32(hasMask))
						return file->GetError();
					if (hasMask)
					{
						for (b = 0; b < pointCount; b++)
						{
							Float32 mask = 0;
							pLayerData->GetMask(b, mask);
							if (!file->WriteFloat32(mask))
								return file->GetError();
						}
					}
				}
			}

			// Save data for all other layers that have information at this level
			// First count how many layers we have at this level (currently not recursing to children but this should be done).
			Int32 layerCount = 0;
			SculptLayerBase* pLayer = pSculpt->GetFirstLayer();
			while (pLayer)
			{
				Int32 type = pLayer->GetType();
				if (type == SCULPT_LAYER_ID)
				{
					SculptLayer*		 pRealLayer	 = (SculptLayer*)pLayer;
					Bool						 isBaseLayer = pRealLayer->IsBaseLayer();
					SculptLayerData* pData = pRealLayer->GetFirstSculptLayerData();
					if (!isBaseLayer && pData && pData->GetSubdivisionLevel() == a)
					{
						layerCount++;
					}
				}
				pLayer = (SculptLayerBase*)pLayer->GetNext();
			}

			// Write out how many layers we have at this level
			if (!file->WriteInt32(layerCount))
				return file->GetError();

			// Write out the layer data
			pLayer = pSculpt->GetFirstLayer();
			while (pLayer)
			{
				Int32 type = pLayer->GetType();
				if (type == SCULPT_LAYER_ID)
				{
					SculptLayer*		 pRealLayer	 = (SculptLayer*)pLayer;
					Bool						 isBaseLayer = pRealLayer->IsBaseLayer();
					SculptLayerData* pData = pRealLayer->GetFirstSculptLayerData();
					if (!isBaseLayer && pData && pData->GetSubdivisionLevel() == a)
					{
						Int32 level = pData->GetSubdivisionLevel();
						Int32 pointCount = pData->GetPointCount();

						if (!file->WriteInt32(type))
							return file->GetError();
						if (!file->WriteInt32(level))
							return file->GetError();
						if (!file->WriteInt32(pointCount))
							return file->GetError();
						if (!file->WriteString(pRealLayer->GetName()))
							return file->GetError();

						Int32 visible = pRealLayer->IsVisible();
						if (!file->WriteInt32(visible))
							return file->GetError();

						Float strength = pRealLayer->GetStrength();
						if (!file->WriteFloat64(strength))
							return file->GetError();

						Int32 b;
						for (b = 0; b < pointCount; b++)
						{
							Vector offset;
							pData->GetOffset(b, offset);

							// Using real instead of vector to match the melange library
							if (!file->WriteFloat64(offset.x))
								return file->GetError();
							if (!file->WriteFloat64(offset.y))
								return file->GetError();
							if (!file->WriteFloat64(offset.z))
								return file->GetError();
						}

						Int32 hasMask = pData->HasMask();
						if (!file->WriteInt32(hasMask))
							return file->GetError();

						if (hasMask)
						{
							for (b = 0; b < pointCount; b++)
							{
								Float32 mask = 0;
								pData->GetMask(b, mask);
								if (!file->WriteFloat32(mask))
									return file->GetError();
							}
							Int32 maskEnabled = pRealLayer->IsMaskEnabled();
							if (!file->WriteInt32(maskEnabled))
								return file->GetError();
						}
					}
				}
				pLayer = (SculptLayerBase*)pLayer->GetNext();
			}
		}
	}
	file->Close();
	return file->GetError();
}

Bool RegisterSculpt()
{
	String name = GeLoadString(IDS_SCULPT);
	if (!RegisterSceneLoaderPlugin(1027977, name, 0, SculptLoaderData::Alloc, "", nullptr))
		return false;
	if (!RegisterSceneSaverPlugin(1027978, name, 0, SculptSaverData::Alloc, "", "scp"))
		return false;
	return true;
}
