// example code for creating a tag that can use the Hair API

#include "c4d.h"
#include "c4d_symbols.h"
#include "lib_hair.h"
#include "main.h"

#include "thairsdkrendering.h"

//////////////////////////////////////////////////////////////////////////

class HairRenderingTag : public TagData
{
	INSTANCEOF(HairRenderingTag, TagData)

public:
	virtual Bool Init(GeListNode* node);
	virtual void Free(GeListNode* node);

	virtual Bool Message(GeListNode* node, Int32 type, void* data);

	static NodeData* Alloc(void) { return NewObjClear(HairRenderingTag); }

	//////////////////////////////////////////////////////////////////////////

	HairPluginObjectData m_FnTable;

	Float								 m_Shadow;
	Float								 m_Trans;
	Int32								 m_Depth;
};

//////////////////////////////////////////////////////////////////////////

static Bool _InitRenderFn(HairVideoPost* vp, VolumeData* vd, BaseDocument* doc, BaseList2D* bl, HairObject* op, HairGuides* guides, Int32 oindex, Int32 pass)
{
	//GePrint("Init Render");

	BaseContainer*		bc	= bl->GetDataInstance();
	HairRenderingTag* hrt = (HairRenderingTag*)bl->GetNodeData();

	if (!bc || !hrt)
		return false;

	hrt->m_Shadow = bc->GetFloat(HAIR_RENDERING_SHADOW);
	hrt->m_Trans	= bc->GetFloat(HAIR_RENDERING_TRANSPARENCY);
	hrt->m_Depth	= bc->GetInt32(HAIR_RENDERING_DEPTH);

	return true;
}

static void _HrFreeRenderFn(HairVideoPost* vp, BaseList2D* bl)
{
	//GePrint("Free Render");
}

static Float _ModifyHairShadowTransparencyFn(HairVideoPost* vp, Int32 oindex, HairMaterialData* mat, RayObject* ro, HairObject* op, HairGuides* guides, BaseList2D* bl, Float32* thk, VolumeData* vd, Int32 cpu, Int32 lid, Int32 seg, Int32 p, Float lined, const Vector& linep, const Vector& n, const Vector& lp, const Vector& huv, const RayHitID& ply_id, RayLight* light, Float trans)
{
	HairRenderingTag* hrt = (HairRenderingTag*)bl->GetNodeData();

	if (light)	// shadow call
		trans = (Float) 1.0 - Clamp01(((Float) 1.0 - Clamp01(trans)) * hrt->m_Shadow);
	else
		trans = (Float) 1.0 - Clamp01(((Float) 1.0 - Clamp01(trans)) * hrt->m_Trans);

	if (hrt->m_Depth > 1)
	{
		Float depth = Float(1 + (seg % hrt->m_Depth)) / Float(hrt->m_Depth);
		trans = 1.0 - ((1.0 - trans) * depth);
	}

	return trans;
}

static Vector _GenerateColorFn(HairVideoPost* vp, Int32 oindex, HairMaterialData* mat, RayObject* ro, HairObject* op, HairGuides* guides, BaseList2D* bl, Float32* thk, VolumeData* vd, Int32 cpu, Int32 lid, Int32 seg, Int32 p, Float lined, const Vector& linep, const Vector& v, const Vector& n, const Vector& lp, const Vector& t, const Vector& r, const Vector& huv, const RayHitID& ply_id)
{
	HairRenderingTag* hrt = (HairRenderingTag*)bl->GetNodeData();

	Vector col;

	vp->Sample(oindex, vd, cpu, lid, seg, p, lined, linep, v, col, n, lp, t, r, huv, HAIR_VP_FLAG_NOHOOKS);

	if (hrt->m_Depth > 1)
	{
		Float depth = Float(1 + (seg % hrt->m_Depth)) / Float(hrt->m_Depth);
		col = col * depth;
	}

	return col;
}

//////////////////////////////////////////////////////////////////////////

Bool HairRenderingTag::Init(GeListNode* node)
{
	BaseContainer* bc = ((BaseList2D*)node)->GetDataInstance();

	m_FnTable.init_render = _InitRenderFn;
	m_FnTable.free_render = _HrFreeRenderFn;
	m_FnTable.calc_shad = _ModifyHairShadowTransparencyFn;
	m_FnTable.calc_col	= _GenerateColorFn;

	bc->SetFloat(HAIR_RENDERING_SHADOW, 1.0);
	bc->SetFloat(HAIR_RENDERING_TRANSPARENCY, 1.0);
	bc->SetInt32(HAIR_RENDERING_DEPTH, 1);

	return true;
}

void HairRenderingTag::Free(GeListNode* node)
{
}

Bool HairRenderingTag::Message(GeListNode* node, Int32 type, void* data)
{
	if (type == MSG_HAIR_GET_OBJECT_TYPE && data)
	{
		HairPluginMessageData* mdata = (HairPluginMessageData*)data;
		mdata->data = &m_FnTable;
		return true;
	}

	return SUPER::Message(node, type, data);
}

//////////////////////////////////////////////////////////////////////////

#define ID_HAIR_RENDERING_EXAMPLE 1018984

Bool RegisterRenderingTag()
{
	return RegisterTagPlugin(ID_HAIR_RENDERING_EXAMPLE, GeLoadString(IDS_HAIR_RENDERING_EXAMPLE), TAG_MULTIPLE | TAG_VISIBLE, HairRenderingTag::Alloc, "Thairsdkrendering", AutoBitmap("hairrendering.tif"), 0);
}
