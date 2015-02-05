// video post example file - visualize post data
// operates after the image is completly rendered

#include "c4d.h"
#include "c4d_symbols.h"
#include "main.h"

class VisualizePostData : public VideoPostData
{
public:
	static NodeData* Alloc(void) { return NewObjClear(VisualizePostData); }
	virtual Bool RenderEngineCheck(BaseVideoPost* node, Int32 id);
	virtual RENDERRESULT Execute(BaseVideoPost* node, VideoPostStruct* vps);
	virtual VIDEOPOSTINFO GetRenderInfo(BaseVideoPost* node) { return VIDEOPOSTINFO_STOREFRAGMENTS; }
};

RENDERRESULT VisualizePostData::Execute(BaseVideoPost* node, VideoPostStruct* vps)
{
	if (vps->vp == VIDEOPOSTCALL_RENDER && !vps->open && *vps->error == RENDERRESULT_OK && !vps->thread->TestBreak())
	{
		VPBuffer*			rgba = vps->render->GetBuffer(VPBUFFER_RGBA, NOTOK);
		RayParameter* ray	 = vps->vd->GetRayParameter();	// only in VP_INNER & VIDEOPOSTCALL_RENDER
		if (!ray)
			return RENDERRESULT_OUTOFMEMORY;
		if (!rgba)
			return RENDERRESULT_OUTOFMEMORY;

		Int32 x1, y1, x2, y2, x, y, cnt;

		// example functions
		Int32 cpp = rgba->GetInfo(VPGETINFO_CPP);

		x1	= ray->left;
		y1	= ray->top;
		x2	= ray->right;
		y2	= ray->bottom;
		cnt = x2 - x1 + 1;

		Int			 bufferSize = cpp * cnt;
		Float32* b, *buffer = nullptr;

		if (bufferSize > 0)
			buffer = NewMemClear(Float32, bufferSize);
		if (!buffer)
			return RENDERRESULT_OUTOFMEMORY;

		for (y = y1; y <= y2; y++)
		{
			rgba->GetLine(x1, y, cnt, buffer, 32, true);

			VPFragment** frag = vps->vd->GetFragments(x1, y, cnt, VPGETFRAGMENTS_Z_P | VPGETFRAGMENTS_N), ** ind = frag;
			if (!frag)
				continue;

			for (b = buffer, x = x1; x <= x2; x++, b += cpp, ind++)
			{
				Vector32		col = Vector32(0.0);
				VPFragment* f;
				for (f = (*ind); f; f = f->next)
					col += (Vector32((Float32)Abs(f->n.x), (Float32)Abs(f->n.y), (Float32)Abs(f->n.z)) * f->weight) * f->color;

				col /= (Float32) 256.0;

				b[0] = col.x;
				b[1] = col.y;
				b[2] = col.z;
			}

			DeleteMem(frag);

			rgba->SetLine(x1, y, cnt, buffer, 32, true);
		}
		DeleteMem(buffer);
	}

	return RENDERRESULT_OK;
}

Bool VisualizePostData::RenderEngineCheck(BaseVideoPost* node, Int32 id)
{
	// the following render engines are not supported by this effect
	if (id == RDATA_RENDERENGINE_PREVIEWSOFTWARE ||
			id == RDATA_RENDERENGINE_PREVIEWHARDWARE ||
			id == RDATA_RENDERENGINE_CINEMAN)
		return false;

	return true;
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_VISUALIZENORMALS 1000986

Bool RegisterVPVisualizeNormals(void)
{
	return RegisterVideoPostPlugin(ID_VISUALIZENORMALS, GeLoadString(IDS_VPVISUALIZEPOST), 0, VisualizePostData::Alloc, "", 0, 0);
}
