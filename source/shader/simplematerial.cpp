// example for an easy implementation of a volume (surface) shader
// It modifies the preview scenes and adds its own scene

#include "c4d.h"
#include "c4d_symbols.h"
#include "msimplematerial.h"
#include "customgui_matpreview.h"
#include "main.h"

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_SIMPLEMAT 1001164

class SimpleMaterial : public MaterialData
{
	INSTANCEOF(SimpleMaterial, MaterialData)

private:
	Vector color;

public:
	virtual Bool Init		(GeListNode* node);
	virtual	void CalcSurface			(BaseMaterial* mat, VolumeData* vd);
	virtual	INITRENDERRESULT InitRender(BaseMaterial* mat, const InitRenderStruct& irs);
	virtual Bool GetDParameter(GeListNode* node, const DescID& id, GeData& t_data, DESCFLAGS_GET& flags);
	virtual Bool SetDParameter(GeListNode* node, const DescID& id, const GeData& t_data, DESCFLAGS_SET& flags);
	virtual Bool Message(GeListNode* node, Int32 type, void* data);
	virtual Bool CopyTo(NodeData* dest, GeListNode* snode, GeListNode* dnode, COPYFLAGS flags, AliasTrans* trn);

	static NodeData* Alloc(void) { return NewObjClear(SimpleMaterial); }

	Int32 updatecount;
};

Bool SimpleMaterial::Init(GeListNode* node)
{
	BaseContainer* data = ((BaseMaterial*)node)->GetDataInstance();
	data->SetVector(SIMPLEMATERIAL_COLOR, Vector(1.0));
	updatecount = 0;

	GeData previewData(CUSTOMDATATYPE_MATPREVIEW, DEFAULTVALUE);
	MaterialPreviewData* preview = (MaterialPreviewData*)previewData.GetCustomDataType(CUSTOMDATATYPE_MATPREVIEW);
	if (!preview)
		return false;
	data->SetData(SIMPLEMATERIAL_MAT_PREVIEW, previewData);

	return true;
}

INITRENDERRESULT SimpleMaterial::InitRender(BaseMaterial* mat, const InitRenderStruct& irs)
{
	BaseContainer* data = mat->GetDataInstance();
	color = data->GetVector(SIMPLEMATERIAL_COLOR);
	return INITRENDERRESULT_OK;
}

static void SimpleIllumModel(VolumeData* sd, RayLightCache* rlc, void* dat)
{
	Bool	nodif, nospec;
	Int32	i;
	Float	cosa, cosb, exponent = 5.0;
	const Vector64& v = sd->ray->v;

	rlc->diffuse = rlc->specular = Vector(0.0);

	for (i = 0; i < rlc->cnt; i++)
	{
		RayLightComponent* lc = rlc->comp[i];
		if (lc->lv == 0.0)
			continue;		// light invisible

		RayLight* ls = lc->light;

		nodif = nospec = false;
		if (ls->lr.object)
			CalcRestrictionInc(&ls->lr, sd->op, nodif, nospec);

		lc->rdiffuse = lc->rspecular = Vector(0.0);
		if (ls->ambient)
		{
			lc->rdiffuse = Vector(1.0);
		}
		else if (ls->arealight)
		{
			sd->CalcArea(ls, nodif, nospec, exponent, v, sd->p, sd->bumpn, sd->orign, sd->raybits, &lc->rdiffuse, &lc->rspecular);
		}
		else
		{
			cosa = Dot(sd->bumpn, lc->lv);
			if (!(ls->nodiffuse || nodif) && sd->cosc * cosa >= 0.0)
			{
				Float trn = ls->trn;
				if (trn != 1.0)
					lc->rdiffuse = Vector(Pow(Abs(cosa), trn));
				else
					lc->rdiffuse = Vector(Abs(cosa));
			}

			if (!(ls->nospecular || nospec))
			{
				cosb = Dot(v, lc->lv - sd->bumpn * (2.0 * cosa));

				if (cosb > 0.0)
					lc->rspecular = Vector(Pow(cosb, exponent));
			}
		}

		rlc->diffuse	+= lc->rdiffuse * lc->col;
		rlc->specular += lc->rspecular * lc->col;
	}
}

void SimpleMaterial::CalcSurface(BaseMaterial* mat, VolumeData* vd)
{
	Vector diff, spec, att_spc, att_dif;
	Int32	 i;

	//sd->Illuminance1(&diff,&spec,5.0);
	vd->IlluminanceSimple(&diff, &spec, 0.0, SimpleIllumModel, this);	// replace standard model by custom model

	att_spc = Vector(0.5 + 0.5 * Turbulence(vd->uvw * 2.5, 4.0, true));
	att_dif = att_spc * color;

	vd->col = (att_dif * (diff + vd->ambient)) + (att_spc * spec);

	// process multipass data
	Multipass* buf = vd->multipass;
	if (!buf)
		return;

	*buf->vp_mat_color = att_dif;
	*buf->vp_mat_specularcolor = att_spc;
	*buf->vp_mat_specular	= 0.4;	// 2.0/exponent (or similar value)

	// values have only to be filled if != 0.0
	// *buf->vp_mat_luminance			= 0.0;
	// *buf->vp_mat_environment		= 0.0;
	// *buf->vp_mat_transparency		= 0.0;
	// *buf->vp_mat_reflection			= 0.0;
	// *buf->vp_mat_diffusion			= 0.0;

	// calculate ambient component
	*buf->vp_ambient = att_dif * vd->ambient;

	// attenuate diffuse components
	for (i = 0; i < buf->diffuse_cnt; i++)
		*buf->diffuse[i] = att_dif * (*buf->diffuse[i]);

	// attenuate specular components
	for (i = 0; i < buf->specular_cnt; i++)
		*buf->specular[i] = att_spc * (*buf->specular[i]);
}


Bool SimpleMaterial::GetDParameter(GeListNode* node, const DescID& id, GeData& t_data, DESCFLAGS_GET& flags)
{
	BaseContainer* data = ((BaseMaterial*)node)->GetDataInstance();


	switch (id[0].id)
	{
		case SIMPLEMATERIAL_MAT_PREVIEW:
			return GetDParameterPreview(data, (GeData*)&t_data, flags, SIMPLEMATERIAL_MAT_PREVIEW, updatecount, (BaseMaterial*)node);
			break;
	}

	return MaterialData::GetDParameter(node, id, t_data, flags);
}

Bool SimpleMaterial::SetDParameter(GeListNode* node, const DescID& id, const GeData& t_data, DESCFLAGS_SET& flags)
{
	BaseContainer* data = ((BaseMaterial*)node)->GetDataInstance();

	updatecount++;

	switch (id[0].id)
	{
		case SIMPLEMATERIAL_MAT_PREVIEW:
			return SetDParameterPreview(data, &t_data, flags, SIMPLEMATERIAL_MAT_PREVIEW);
			break;
	}

	return MaterialData::SetDParameter(node, id, t_data, flags);
}

Bool SimpleMaterial::Message(GeListNode* node, Int32 type, void* data)
{
	if (type == MSG_UPDATE)
		updatecount++;

	switch (type)
	{
		case MATPREVIEW_GET_OBJECT_INFO:
		{
			MatPreviewObjectInfo* info = (MatPreviewObjectInfo*)data;
			info->bHandlePreview = true;		// own preview handling
			info->bNeedsOwnScene = true;		// we need our own entry in the preview scene cache
			info->bNoStandardScene = false;	// we modify the standard scene
			info->lFlags = 0;
			return true;
			break;
		}
		case MATPREVIEW_MODIFY_CACHE_SCENE:
			// modify the preview scene here. We have a pointer to a scene inside the preview scene cache.
			// our scene contains the object
		{
			MatPreviewModifyCacheScene* scene = (MatPreviewModifyCacheScene*)data;
			// get the striped plane from the preview
			BaseObject* plane = scene->pDoc->SearchObject("Polygon");
			if (plane)
				plane->SetRelScale(Vector(0.1));	// scale it a bit
			return true;
			break;
		}
		case MATPREVIEW_PREPARE_SCENE:
			// let the preview handle the rest...
			return true;
			break;
		case MATPREVIEW_GENERATE_IMAGE:
		{
			MatPreviewGenerateImage* image = (MatPreviewGenerateImage*)data;
			if (image->pDoc)
			{
				Int32					w = image->pDest->GetBw();
				Int32					h = image->pDest->GetBh();
				BaseContainer bcRender = image->pDoc->GetActiveRenderData()->GetData();
				bcRender.SetFloat(RDATA_XRES, w);
				bcRender.SetFloat(RDATA_YRES, h);
				bcRender.SetInt32(RDATA_ANTIALIASING, ANTI_GEOMETRY);
				if (image->bLowQuality)
					bcRender.SetBool(RDATA_RENDERENGINE, RDATA_RENDERENGINE_PREVIEWSOFTWARE);
				image->pDest->Clear(0, 0, 0);
				image->lResult = RenderDocument(image->pDoc, bcRender, nullptr, nullptr, image->pDest,
													 RENDERFLAGS_EXTERNAL | RENDERFLAGS_PREVIEWRENDER, image->pThread);
			}
			return true;
			break;
		}

		case MATPREVIEW_GET_PREVIEW_ID:
			*((Int32*)data) = SIMPLEMATERIAL_MAT_PREVIEW;
			return true;
	}

	return true;
}

Bool SimpleMaterial::CopyTo(NodeData* dest, GeListNode* snode, GeListNode* dnode, COPYFLAGS flags, AliasTrans* trn)
{
	((SimpleMaterial*)dest)->updatecount = updatecount;
	return NodeData::CopyTo(dest, snode, dnode, flags, trn);
}


Bool RegisterSimpleMaterial(void)
{
	String name = GeGetDefaultFilename(DEFAULTFILENAME_SHADER_VOLUME) + GeLoadString(IDS_SIMPLEMATERIAL);	// place in default Shader section

	// add a preview scene that can only be selected in the Simple Material's preview
	AddUserPreviewScene(GeGetPluginResourcePath() + String("scene") + String("Stairs.c4d"), ID_SIMPLEMAT, nullptr);

	return RegisterMaterialPlugin(ID_SIMPLEMAT, name, 0, SimpleMaterial::Alloc, "Msimplematerial", 0);
}
