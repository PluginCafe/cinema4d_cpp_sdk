/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example matrix filter

#include "c4d.h"
#include "c4d_symbols.h"
#include "filterthread.h"
#include "filterpreview.h"
#include "filter_prefs.h"
#include "matrixutil.h"
#include "matrixwrapper.h"
#include "main.h"

//----------------------------------------------------------------------------------------
// filter interface functions
//----------------------------------------------------------------------------------------

// filter settings
static FILTER_PARAM	filter_new_settings(FI_REF fi, const BaseContainer* t_bc);
static Int32 filter_delete_settings(FI_REF fi, FILTER_PARAM settings);
static FLAT_FILTER_SETTINGS* filter_flatten_settings(FI_REF fi, FILTER_PARAM settings, void*(*flat_malloc)(Int32 size));
static FILTER_PARAM	filter_unflatten_settings(FI_REF fi, FLAT_FILTER_SETTINGS* flat_settings);

// filter actions
static Int32 filter_start(FI_REF fi, BM_REF bm, FILTER_PARAM settings);
static Int32 filter_end(FI_REF fi, BM_REF bm, FILTER_PARAM settings);
static Int32 filter_run(FI_REF fi, BM_REF bm, FILTER_PARAM settings, Bool use_gui);

static BITMAP_FILTER smpl_matrix_filter =
{
	BITMAPFILTER_MAGIC_V3,

	// filter settings
	filter_new_settings,
	filter_delete_settings,
	filter_flatten_settings,
	filter_unflatten_settings,

	// filter actions
	filter_start,
	filter_end,
	filter_run,

	BMFI_SUPPORT_8BIT | BMFI_SUPPORT_16BIT | BMFI_SUPPORT_32BIT

};

//----------------------------------------------------------------------------------------
// private functions
//----------------------------------------------------------------------------------------
#define	MIN_ANGLE	 0.0
#define	MAX_ANGLE	 360.0
#define	STEP_ANGLE 0.1
#define	DFLT_ANGLE 0.0

#define	PRIVATE_SETTINGS			 MATRIX_SAMPLE_SETTINGS
#define	PRIVATE_SETTINGS_MAGIC CHAR_CONST32('smtr')
struct PRIVATE_SETTINGS : public BM_FILTER_SETTINGS
{
	UInt32		 magic;
	Float32		 matrix_opacity;	// opacity level of the matrix (0.0 ... 1.0)
	Float32		 angle;
	Bool			 scale_matrix;
	Int32			 tile_flags;

	Int32			 type;
	MATRIX_REF emr;				// reference to the effect matrix settings
	Semaphore* emr_lock;	// is used to enable safe creation of the effect matrix
};

static Bool	change_effect_matrix(PRIVATE_SETTINGS* settings, BM_REF bm);
static Int32 apply_effect(BM_REF bm, const RECT32* src_rect, BM_TILE* dt, BM_FILTER_SETTINGS* settings, Bool update_view, BaseThread* thread);

static WEIGHT_FMATRIX_5x5	Astrange_triangle_matrix_5x5 =
{
	5,
	5,
	2,
	2,
	{
		{	-1.0,	2.0, -1.0, 2.0,	-1.0 },
		{	0.0, -1.0, 3.0,	-1.0,	0.0	},
		{	0.0, 0.0,	-1.0,	0.0, 0.0 },
		{	0.0, 0.0,	0.0, 0.0,	0.0	},
		{	0.0, 0.0,	0.0, 0.0,	0.0	}
	}
};

static WEIGHT_FMATRIX_5x5	Bstrange_triangle_matrix_5x5 =
{
	5,
	5,
	2,
	2,
	{
		{	1.0, 1.0,	1.0, 1.0,	1.0	},
		{	0.0, 1.0,	1.0, 1.0,	0.0	},
		{	0.0, 0.0,	1.0, 0.0,	0.0	},
		{	0.0, 0.0,	0.0, 0.0,	0.0	},
		{	0.0, 0.0,	0.0, 0.0,	0.0	}
	}
};

static WEIGHT_FMATRIX* matrix_tab[] =
{
	(WEIGHT_FMATRIX*) &Astrange_triangle_matrix_5x5,
	(WEIGHT_FMATRIX*) &Bstrange_triangle_matrix_5x5,

	0
};

//----------------------------------------------------------------------------------------
// filter dialog class (ModalDialog)
//----------------------------------------------------------------------------------------
class SmplMatrixDialog : public GeModalDialog
{
	PRIVATE_SETTINGS* settings;
	PreviewGroup			preview;
	BM_REF						bm;
	Bool							real_time;	// true: apply effect in preview while dragging
	Bool							document_preview;

public:
	SmplMatrixDialog(void);
	~SmplMatrixDialog();

	virtual Bool Command(Int32 id, const BaseContainer& msg);
	virtual Bool CreateLayout(void);
	virtual Bool InitValues(void);

	Bool Init(BM_REF bm, PRIVATE_SETTINGS* settings);
};

//----------------------------------------------------------------------------------------
// Register filter
// Function result:		true/false
//----------------------------------------------------------------------------------------
Bool RegisterSampleMatrix(void)
{
	String name = GeLoadString(IDS_MATRIX);
	if (!name.Content())
		return true;

	// be sure to use a unique ID obtained from www.plugincafe.com
	return GeRegisterPlugin(PLUGINTYPE_BITMAPFILTER, 1000690, name, &smpl_matrix_filter, sizeof(smpl_matrix_filter));
}


//----------------------------------------------------------------------------------------
// Create default filter settings
// Function result:		filter settings oder 0 (not enough memory)
//----------------------------------------------------------------------------------------
static FILTER_PARAM	filter_new_settings(FI_REF fi, const BaseContainer* t_bc)
{
	PRIVATE_SETTINGS* settings;

	settings = NewMemClear(PRIVATE_SETTINGS, 1);
	if (settings)
	{
		settings->magic = PRIVATE_SETTINGS_MAGIC;
		settings->matrix_opacity = 1.0;
		settings->angle = DFLT_ANGLE;
		settings->scale_matrix = true;
		settings->type = 0;
		settings->tile_flags = TILE_REPEAT_BORDER;

		settings->emr = 0;												// no effect matrix so far
		settings->emr_lock = Semaphore::Alloc();	// create a semaphore for the matrix object

		if (settings->emr_lock == 0)							// failed to allocate the semaphore?
		{
			DeleteMem(settings);
			settings = 0;
		}
	}

	return (FILTER_PARAM)settings;
}

//----------------------------------------------------------------------------------------
// Dispose filter settings
// Function result:		FILTER_OK or an error code
// settings:					private filter settings
//----------------------------------------------------------------------------------------
static Int32 filter_delete_settings(FI_REF fi, FILTER_PARAM _settings)
{
	PRIVATE_SETTINGS* settings;

	settings = (PRIVATE_SETTINGS*) _settings;
	if (settings)
	{
		if (settings->emr)
			delete_effect_matrix(settings->emr);	// free the matrix object

		Semaphore::Free(settings->emr_lock);		// free the semaphore

		DeleteMem(settings);
		settings = 0;
	}
	return FILTER_OK;
}

//----------------------------------------------------------------------------------------
// Convert private settings in format that can be saved (no pointers, ...)
// Function result:		pointer to the "flat" filter settings or 0
// settings:					private filter settings
// flat_malloc:				must be used to allocate the memory for flat_settings
//----------------------------------------------------------------------------------------
static FLAT_FILTER_SETTINGS* filter_flatten_settings(FI_REF fi, FILTER_PARAM settings, void*(*flat_malloc)(Int32 size))
{
	FLAT_FILTER_SETTINGS* flat_settings;
	Int32	len;

	len = sizeof_FLAT_FILTER_SETTINGS + sizeof(PRIVATE_SETTINGS);
	flat_settings = (FLAT_FILTER_SETTINGS*) flat_malloc(len);

	if (flat_settings)
	{
		PRIVATE_SETTINGS* tmp;

		flat_settings->magic	= FLAT_SETTINGS_MAGIC;
		flat_settings->length = len;
		flat_settings->format = 0;
		flat_settings->reserved = 0;

		tmp = (PRIVATE_SETTINGS*) flat_settings->data;
		CopyMem(settings, tmp, sizeof(PRIVATE_SETTINGS));
		tmp->emr = 0;
		tmp->emr_lock = 0;
	}
	return flat_settings;
}

//----------------------------------------------------------------------------------------
// Validate flat settings and convert into private format
// Function result:		pointer to the private filter settings or 0
// flat_settings:			saved settings
//----------------------------------------------------------------------------------------
static FILTER_PARAM	filter_unflatten_settings(FI_REF fi, FLAT_FILTER_SETTINGS* flat_settings)
{
	PRIVATE_SETTINGS* settings;

	settings = 0;

	if (flat_settings->magic == FLAT_SETTINGS_MAGIC)
	{
		PRIVATE_SETTINGS* p;

		p = (PRIVATE_SETTINGS*) flat_settings->data;
		if (p->magic == PRIVATE_SETTINGS_MAGIC)
		{
			settings = (PRIVATE_SETTINGS*) filter_new_settings(fi, 0);
			if (settings)
			{
				if ((p->matrix_opacity >= 0.0) && (p->matrix_opacity <= 1.0))
					settings->matrix_opacity = p->matrix_opacity;

				if ((p->angle >= MIN_ANGLE) && (p->angle <= MAX_ANGLE))
					settings->angle = p->angle;
			}
		}
	}

	return settings;
}

//----------------------------------------------------------------------------------------
// Start of a filter action
// Function result:		FILTER_OK or an error code (e.g. FILTER_MEM_ERR)
// bm:								bitmap reference
// settings:					private filter settings
//----------------------------------------------------------------------------------------
static Int32 filter_start(FI_REF fi, BM_REF bm, FILTER_PARAM _settings)
{
	PRIVATE_SETTINGS* settings;

	settings = (PRIVATE_SETTINGS*) _settings;
	if (settings && (settings->magic == PRIVATE_SETTINGS_MAGIC))
	{
		return FILTER_OK;
	}
	return FILTER_PARAM_ERR;	// invalid settings
}

//----------------------------------------------------------------------------------------
// End of a filter action
// Function result:		FILTER_OK or an error code
// bm:								bitmap reference
// settings:					private filter settings
//----------------------------------------------------------------------------------------
static Int32 filter_end(FI_REF fi, BM_REF bm, FILTER_PARAM _settings)
{
	PRIVATE_SETTINGS* settings;

	settings = (PRIVATE_SETTINGS*) _settings;

	if (settings && (settings->magic == PRIVATE_SETTINGS_MAGIC))
	{
		return FILTER_OK;
	}
	return FILTER_PARAM_ERR;
}

//----------------------------------------------------------------------------------------
// Run the filter action
// Function result:		FILTER_OK, FILTER_CANCEL, FILTER_ABORT or an error code
// bm:								bitmap reference
// settings:					private filter settings
// use_gui:						true: show dialog false: use current settings (silent mode)
//----------------------------------------------------------------------------------------
static Int32 filter_run(FI_REF fi, BM_REF bm, FILTER_PARAM _settings, Bool use_gui)
{
	PRIVATE_SETTINGS* settings;

	settings = (PRIVATE_SETTINGS*) _settings;
	if (use_gui)
	{
		SmplMatrixDialog dlg;

		if (dlg.Init(bm, settings))
		{
			if (dlg.Open())	// do the dialog handling
				return FILTER_OK;
		}

		return FILTER_CANCEL;
	}
	else
	{
		return apply_effect(bm, 0, 0, settings, false, 0);
	}
}

static Bool	change_effect_matrix(PRIVATE_SETTINGS* settings, BM_REF bm)
{
	Float64	r;

	if (settings->emr)
		delete_effect_matrix(settings->emr);

	r	 = settings->angle;
	r /= 180.0;
	r *= PI;

	settings->emr = new_effect_matrix(matrix_tab[settings->type], bm->image_color_space, bm->image_px_format, settings->matrix_opacity, settings->scale_matrix, r);
	if (settings->emr)
		return true;
	else
		return false;
}
//----------------------------------------------------------------------------------------
// Apply the effect to the whole bitmap and update the document view (if indicated)
// Function result:		FILTER_OK or error code
// bm:								bitmap reference
// src_rect:					requested source rectangle
// dt:								pointer of the preview destination tile or 0 (apply effect to the mask_rect area)
// settings:					private filter settings
// update_view:				true: call BfUpdateView() for processed tiles
// thread:						pointer to thread class (0: no thread)
//----------------------------------------------------------------------------------------
static Int32 apply_effect(BM_REF bm, const RECT32* src_rect, BM_TILE* dt, BM_FILTER_SETTINGS* t_settings, Bool update_view, BaseThread* thread)
{
	PRIVATE_SETTINGS* settings = (PRIVATE_SETTINGS*)t_settings;
	Int32	err;

	err = FILTER_OK;

	if (src_rect == 0)
		src_rect = &bm->mask_rect;

	if (settings->emr_lock->Lock(thread))	// lock the semaphore for the effect matrix
	{
		if (settings->emr == 0)							// effect matrix not initialized?
		{
			if (change_effect_matrix(settings, bm) == false)
				err = FILTER_MEM_ERR;
		}
		else
		{
			update_effect_matrix(settings->emr, bm->image_color_space, bm->image_px_format);	// update settings in case bitmap has changed

		}
		settings->emr_lock->Unlock();
	}
	else	// stop thread
	{
		err = FILTER_ABORT;
	}

	if (err == FILTER_OK)
	{
		if (dt)
		{
			BM_TILE	 tmp;
			BM_TILE* scratch;
			Bool		 scale;
			Int32		 width;
			Int32		 height;

			width	 = dt->xmax - dt->xmin;
			height = dt->ymax - dt->ymin;

			if ((width != src_rect->x2 - src_rect->x1) && (height != src_rect->y2 - src_rect->y1))
			{
				scale = true;	// in this case we have to post-scale the result of the matrix
				scratch = BfBitmapTileGet(bm, src_rect, 0, 0, TILE_BM_SCRATCH, 0);
				if (scratch == 0)
					return FILTER_MEM_ERR;
			}
			else
			{
				scale = false;
				tmp = *dt;
				tmp.xmin = src_rect->x1;	// make sure the effect gets the same source and tile rect
				tmp.ymin = src_rect->y1;
				tmp.xmax = src_rect->x2;
				tmp.ymax = src_rect->y2;
				scratch	 = &tmp;
			}

			err = do_matrix_effect_1dt(bm, scratch, settings->emr, settings->tile_flags);
			if (scale)
			{
				if (err == FILTER_OK)
				{
					if (BfBitmapTileScale(bm, scratch, dt, (RECT32*) &scratch->xmin, (RECT32*) &dt->xmin) == false)
						err = FILTER_MEM_ERR;
				}
				BfBitmapTileDetach(bm, scratch, false);
			}
		}
		else
		{
			String apply_filter_str;

			apply_filter_str = GeLoadString(IDS_APPLY_FILTER);
			err = do_matrix_effect(bm, settings->emr, thread, settings->tile_flags, update_view, &apply_filter_str);
		}
	}
	return err;
}

SmplMatrixDialog::SmplMatrixDialog(void)
{
	bm = nullptr;
	real_time = BfpGetRealTimeFlag();
	document_preview = BfpGetDocumentPreviewFlag();
}

SmplMatrixDialog::~SmplMatrixDialog()
{
	if (GetResult())									// exit dialog with "OK"?
	{
		preview.FinishDocumentUpdate();	// wait until the effect has been applied
		BfpSetDocumentPreviewFlag(document_preview);
		BfpSetTextureModeFlag(settings->tile_flags == TILE_REPEAT_TILING);
	}
}

Bool SmplMatrixDialog::Init(BM_REF _bm, PRIVATE_SETTINGS* _settings)
{
	if (_bm)
	{
		bm = _bm;
		settings = _settings;
		settings->tile_flags = BfpGetTextureModeFlag() ? TILE_REPEAT_TILING : TILE_REPEAT_BORDER;
		return true;
	}
	return false;
}

Bool SmplMatrixDialog::CreateLayout(void)
{
	LoadDialogResource(DLG_SMPL_MATRIX, nullptr, 0);
	preview.Create(this, GADGET_SMPL_MATRIX_PREVIEWGROUP);
	preview.SetSource(bm);
	preview.SetDestination(bm, apply_effect, settings, true, document_preview);

	return true;
}

Bool SmplMatrixDialog::InitValues(void)
{
	SetInt32(GADGET_SMPL_MATRIX_DOCUMENT_PREVIEW, document_preview);
	SetInt32(GADGET_SMPL_MATRIX_TEXTURE_MODE, settings->tile_flags == TILE_REPEAT_BORDER ? 0 : 1);

	SetPercent(GADGET_SMPL_MATRIX_SLIDER_MIX, settings->matrix_opacity, 0.0, 100.0, 0.1);
	SetFloat(GADGET_SMPL_MATRIX_SLIDER_ANGLE, DegToRad(settings->angle), DegToRad(MIN_ANGLE), DegToRad(MAX_ANGLE), DegToRad(STEP_ANGLE), FORMAT_DEGREE);
	SetInt32(GADGET_SMPL_MATRIX_TYPE_POPUP, settings->type);

	return true;
}

Bool SmplMatrixDialog::Command(Int32 id, const BaseContainer& msg)
{
	Bool update_settings = false;

	if (preview.Command(id, msg))
		return true;

	switch (id)
	{
		case	GADGET_SMPL_MATRIX_TYPE_POPUP:
		{
			Int32	type;

			preview.ChangedSettings();	// stop running threads before settings are changed
			type = msg.GetInt32(BFM_ACTION_VALUE);
			if (settings->emr)
			{
				delete_effect_matrix(settings->emr);
				settings->emr = 0;
			}

			settings->type	= type;
			update_settings = true;	// indicates changed effect settings

			break;
		}
		case	GADGET_SMPL_MATRIX_SLIDER_ANGLE:
		{
			Float32	slider_value;

			slider_value = (Float32)RadToDeg(msg.GetFloat(BFM_ACTION_VALUE));
			if (real_time || (msg.GetInt32(BFM_ACTION_INDRAG) == false))
			{
				if (settings->angle != slider_value)
				{
					preview.ChangedSettings();	// stop running threads before settings are changed
					settings->angle = slider_value;
					update_settings = true;			// indicates changed effect settings
				}
			}
			break;
		}
		case	GADGET_SMPL_MATRIX_SLIDER_MIX:
		{
			Float32	slider_value;

			slider_value = (Float32)msg.GetFloat(BFM_ACTION_VALUE);
			if (real_time || msg.GetInt32(BFM_ACTION_INDRAG) == false)
			{
				if (settings->matrix_opacity != slider_value)
				{
					preview.ChangedSettings();	// stop running threads before settings are changed
					settings->matrix_opacity = slider_value;
					update_settings = true;			// indicates changed effect settings
				}
			}
			break;
		}
		case	GADGET_SMPL_MATRIX_TEXTURE_MODE:
		{
			preview.ChangedSettings();	// stop running threads before settings are changed
			settings->tile_flags = msg.GetInt32(BFM_ACTION_VALUE) ? TILE_REPEAT_TILING : TILE_REPEAT_BORDER;
			update_settings = true;
			break;
		}
		case	GADGET_SMPL_MATRIX_DOCUMENT_PREVIEW:
		{
			document_preview = msg.GetInt32(BFM_ACTION_VALUE);
			preview.SetDocumentPreview(document_preview);
			break;
		}
	}

	if (update_settings)
	{
		change_effect_matrix(settings, bm);
		preview.Update();
	}

	return true;
}

