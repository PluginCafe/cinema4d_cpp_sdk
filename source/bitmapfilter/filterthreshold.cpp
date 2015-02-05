// example threshold filter

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
static FILTER_PARAM	filter_new_settings(FI_REF fi, const BaseContainer* bc);
static Int32 filter_delete_settings(FI_REF fi, FILTER_PARAM settings);
static FLAT_FILTER_SETTINGS* filter_flatten_settings(FI_REF fi, FILTER_PARAM settings, void*(*flat_malloc)(Int32 size));
static FILTER_PARAM	filter_unflatten_settings(FI_REF fi, FLAT_FILTER_SETTINGS* flat_settings);

// filter actions
static Int32 filter_start(FI_REF fi, BM_REF bm, FILTER_PARAM settings);
static Int32 filter_end(FI_REF fi, BM_REF bm, FILTER_PARAM settings);
static Int32 filter_run(FI_REF fi, BM_REF bm, FILTER_PARAM settings, Bool use_gui);

static BITMAP_FILTER threshold_filter =
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
#define BM_FILTER_THRESHOLD_LEVELS 1

#define	MIN_LEVELS	2
#define	MAX_LEVELS	256
#define	STEP_LEVELS	1
#define	DFLT_LEVELS	16

#define	PRIVATE_SETTINGS			 THRESHOLD_SETTINGS
#define	PRIVATE_SETTINGS_MAGIC CHAR_CONST32('thrs')
struct PRIVATE_SETTINGS : public BM_FILTER_SETTINGS
{
	UInt32	magic;
	Int32		no_levels;

	Int32		max_value;
	UInt32* scale_table;
};

static Int32 apply_effect(BM_REF bm, const RECT32* src_rect, BM_TILE* dt, BM_FILTER_SETTINGS* settings, Bool update_view, BaseThread* thread);
static Int32 do_effect(BM_REF bm, const RECT32* src_rect, BM_TILE* dst, PRIVATE_SETTINGS* settings);
static Int32 do_effect16(BM_REF bm, const RECT32* src_rect, BM_TILE* dst, PRIVATE_SETTINGS* settings);
static Int32 do_effect32(BM_REF bm, const RECT32* src_rect, BM_TILE* dst, PRIVATE_SETTINGS* settings);
static Bool	change_scale_table(PRIVATE_SETTINGS* settings, Int32 no_levels, Bool always);


//----------------------------------------------------------------------------------------
// filter dialog class (ModalDialog)
//----------------------------------------------------------------------------------------
class ThresholdDialog : public GeModalDialog
{
	PRIVATE_SETTINGS* settings;
	PreviewGroup			preview;
	BM_REF						bm;
	Bool							real_time;
	Bool							document_preview;

public:
	ThresholdDialog(void);
	~ThresholdDialog();

	virtual Bool Command(Int32 id, const BaseContainer& msg);
	virtual Bool CreateLayout(void);
	virtual Bool InitValues(void);

	Bool Init(BM_REF bm, PRIVATE_SETTINGS* settings);
};


//----------------------------------------------------------------------------------------
// Register filter
// Function result:		true/false
//----------------------------------------------------------------------------------------
Bool RegisterThreshold(void)
{
	String name = GeLoadString(IDS_THRESHOLD);
	if (!name.Content())
		return true;

	// be sure to use a unique ID obtained from www.plugincafe.com
	return GeRegisterPlugin(PLUGINTYPE_BITMAPFILTER, 1000691, name, &threshold_filter, sizeof(threshold_filter));
}

//----------------------------------------------------------------------------------------
// Create default filter settings
// Function result:		filter settings oder 0 (not enough memory)
//----------------------------------------------------------------------------------------
static FILTER_PARAM	filter_new_settings(FI_REF fi, const BaseContainer* bc)
{
	PRIVATE_SETTINGS* settings;

	settings = NewMemClear(PRIVATE_SETTINGS, 1);
	if (settings)
	{
		settings->magic = PRIVATE_SETTINGS_MAGIC;
		settings->no_levels = DFLT_LEVELS;
		settings->scale_table = nullptr;

		if (bc)	// read settings from container?
		{
			settings->no_levels = (Int32) bc->GetFloat(BM_FILTER_THRESHOLD_LEVELS);

			if (settings->no_levels < MIN_LEVELS)
				settings->no_levels = MIN_LEVELS;
			if (settings->no_levels > MAX_LEVELS)
				settings->no_levels = MAX_LEVELS;
		}
	}

	return (FILTER_PARAM) settings;
}

//----------------------------------------------------------------------------------------
// Dispose of filter settings
// Function result:		FILTER_OK or an error code
// settings:					private filter settings
//----------------------------------------------------------------------------------------
static Int32 filter_delete_settings(FI_REF fi, FILTER_PARAM settings)
{
	if (settings)
	{
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
		flat_settings->magic	= FLAT_SETTINGS_MAGIC;
		flat_settings->length = len;
		flat_settings->format = 0;
		flat_settings->reserved = 0;

		CopyMem(settings, flat_settings->data, sizeof(PRIVATE_SETTINGS));	// ignore max_value and scale_table
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
		if (p->magic == PRIVATE_SETTINGS_MAGIC)	// do they belong to this filter?
		{
			settings = (PRIVATE_SETTINGS*) filter_new_settings(fi, 0);
			if (settings)
			{
				if ((p->no_levels > MIN_LEVELS) && (p->no_levels < MAX_LEVELS))	// valid range?
					settings->no_levels = p->no_levels;														// set value
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
		Int32	component_size;

		component_size = get_PX_USED(bm->image_px_format) / get_PX_CMPNTS(bm->image_px_format);	// get the number of bits per component
		if (component_size <= 8)
		{
			Int32		max_value;
			UInt32* scale_table;

			max_value = Int32((1L << component_size) - 1);	// maximum pixel component value

			scale_table = NewMemClear(UInt32, (max_value + 1));
			if (scale_table)
			{
				settings->max_value = max_value;
				settings->scale_table = scale_table;
				change_scale_table(settings, settings->no_levels, true);

				return FILTER_OK;			// everything is fine
			}
			return FILTER_MEM_ERR;	// missing memory
		}
		else											// more than 8 bit
		{
			settings->max_value = 0;
			settings->scale_table = nullptr;
			return FILTER_OK;	// everything is fine
		}
	}
	return FILTER_PARAM_ERR;	// invalid parameters
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
		if (settings->scale_table)
		{
			DeleteMem(settings->scale_table);	// free scale table
			settings->scale_table = nullptr;
		}
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
		ThresholdDialog dlg;

		if (dlg.Init(bm, settings))
		{
			if (dlg.Open())	// do the dialog handling
				return FILTER_OK;
		}

		return FILTER_CANCEL;
	}
	else	// might be better to use threading here, too (depending on the filter speed)
	{
		return apply_effect(bm, 0, 0, settings, false, nullptr);
	}
}

//----------------------------------------------------------------------------------------
// Fill 1D scaling table for pixel components (adjusts gamma, contrast, brightness)
// Function result:		true: new table content has been generated false: no change
// settings:					private filter settings
// no_levels:					new no_levels value
// always:						true: force new calculation false: only if input values are different
//----------------------------------------------------------------------------------------
static Bool	change_scale_table(PRIVATE_SETTINGS* settings, Int32 no_levels, Bool always)
{
	if (settings->scale_table == nullptr)
	{
		settings->no_levels = no_levels;
		settings->max_value = 0;
		return true;
	}

	if (always || (no_levels != settings->no_levels))
	{
		Int32		i;
		Int32		max_value;
		UInt32* scale_table;

		settings->no_levels = no_levels;
		scale_table = settings->scale_table;
		max_value = settings->max_value;	// max. pixel component value

		for (i = 0; i <= max_value; i++)	// calculate scaling table
		{
			Int32	value;

			value = i * no_levels;
			if (value)
				value--;
			value /= max_value;
			value *= max_value;
			value /= (no_levels - 1);

			*scale_table++ = (UInt32) value;
		}
		return true;	// settings have been altered
	}
	return false;		// settings remain unchanged
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

	Int32 tile_width	= bm->preferred_tile_width;
	Int32 tile_height = bm->preferred_tile_height;
	Int32 err = FILTER_CANCEL;

	PROGRESS_ID	pid = 0;
	Int32				progress_cnt;
	Int32				progress_max;

	RECT32 rect;
	Int32	 x;
	Int32	 y;
	String apply_filter_str;

	apply_filter_str = GeLoadString(IDS_APPLY_FILTER);

	if (src_rect == 0)
		src_rect = &bm->mask_rect;

	if (dt)
	{
		switch (get_PX_DEPTH(bm->image_px_format))
		{
			case PX_DEPTH_8:	err = do_effect(bm, src_rect, dt, (PRIVATE_SETTINGS*) settings); break;
			case PX_DEPTH_16: err = do_effect16(bm, src_rect, dt, (PRIVATE_SETTINGS*) settings); break;
			case PX_DEPTH_32: err = do_effect32(bm, src_rect, dt, (PRIVATE_SETTINGS*) settings); break;
		}
	}
	else
	{
		rect = bm->mask_rect;

		progress_cnt = 0;
		progress_max = (((rect.y2 + tile_height - 1) / tile_height) - (rect.y1 / tile_height)) *
									 (((rect.x2 + tile_width - 1) / tile_width) - (rect.x1 / tile_width));
		pid = BfProgressNew(bm);

		for (y = rect.y1 - (rect.y1 % tile_height); y < rect.y2; y += tile_height)
		{
			for (x = rect.x1 - (rect.x1 % tile_width); x < rect.x2; x += tile_width)
			{
				RECT32	 tile_rect;
				BM_TILE* dst;

				if (thread && thread->TestBreak())	// force exit?
				{
					err = FILTER_CANCEL;
					update_view = false;
					goto exit_filter;
				}

				BfProgressUpdate(bm, pid, progress_cnt++, progress_max, apply_filter_str);

				tile_rect.x1 = LMax(rect.x1, x);
				tile_rect.y1 = LMax(rect.y1, y);
				tile_rect.x2 = LMin(rect.x2, x + tile_width);
				tile_rect.y2 = LMin(rect.y2, y + tile_height);
				dst = BfBitmapTileGet(bm, &tile_rect, 0, 0, TILE_BM_WRITE, 0);	// allocate destination tile

				if (dst)
				{
					switch (get_PX_DEPTH(bm->image_px_format))
					{
						case PX_DEPTH_8:	err = do_effect(bm, (RECT32*) &dst->xmin, dst, settings);	break;
						case PX_DEPTH_16: err = do_effect16(bm, (RECT32*) &dst->xmin, dst, settings); break;
						case PX_DEPTH_32: err = do_effect32(bm, (RECT32*) &dst->xmin, dst, settings); break;
					}

					BfBitmapTileDetach(bm, dst, err == FILTER_OK);	// return destination tile and apply it (FILTER_OK)
					if (err != FILTER_OK)
						goto exit_filter;

					if (update_view)
						BfUpdateView(bm);		// update view
				}
			}
		}

exit_filter:

		BfProgressDelete(bm, pid);
	}

	return err;
}

static Int32 do_effect(BM_REF bm, const RECT32* src_rect, BM_TILE* dst, PRIVATE_SETTINGS* settings)
{
	BM_TILE* src;

	if ((bm == nullptr) || (dst == nullptr) || (settings == nullptr))
		return FILTER_MEM_ERR;

	src = BfBitmapTileGet(bm, src_rect, dst->xmax - dst->xmin, dst->ymax - dst->ymin, TILE_BM_READ_ONLY, 0);
	if (src)
	{
		UInt32* scale_table;
		UChar*	srcptr;
		UChar*	dstptr;
		Int32		x, y, inc;

		srcptr = (UChar*) src->addr;
		dstptr = (UChar*) dst->addr;
		inc = get_PX_CMPNTS(src->px_format);
		scale_table = settings->scale_table;

		for (y = src->ymin; y < src->ymax; y++)
		{
			for (x = src->xmin; x < src->xmax; x++)
			{
				Int32 i = 0;

				if (bm->image_color_space & CSPACE_ALPHA_FLAG)
				{
					*dstptr++ = *srcptr++;
					i++;
				}

				for (; i < inc; i++)
					*dstptr++ = (UChar)scale_table[*srcptr++];
			}
		}
		BfBitmapTileDetach(bm, src, false);
		return FILTER_OK;
	}
	return FILTER_MEM_ERR;
}

static Int32 do_effect16(BM_REF bm, const RECT32* src_rect, BM_TILE* dst, PRIVATE_SETTINGS* settings)
{
	BM_TILE* src;

	if ((bm == 0) || (dst == 0) || (settings == 0))
		return FILTER_MEM_ERR;

	src = BfBitmapTileGet(bm, src_rect, dst->xmax - dst->xmin, dst->ymax - dst->ymin, TILE_BM_READ_ONLY, 0);
	if (src)
	{
		UInt16* srcptr;
		UInt16* dstptr;
		UInt32	no_levels;
		Int32		x, y, inc;

		srcptr = (UInt16*) src->addr;
		dstptr = (UInt16*) dst->addr;
		inc = get_PX_CMPNTS(src->px_format);
		no_levels = settings->no_levels;

		for (y = src->ymin; y < src->ymax; y++)
		{
			for (x = src->xmin; x < src->xmax; x++)
			{
				Int32 i = 0;

				if (bm->image_color_space & CSPACE_ALPHA_FLAG)
				{
					*dstptr++ = *srcptr++;
					i++;
				}

				for (; i < inc; i++)
				{
					UInt32 value;

					value	 = *srcptr++;
					value *= no_levels;
					if (value)
						value--;

					value /= 65535;
					value *= 65535;
					value /= (no_levels - 1);

					*dstptr++ = (UInt16) value;
				}
			}
		}
		BfBitmapTileDetach(bm, src, false);
		return FILTER_OK;
	}
	return FILTER_MEM_ERR;
}

static Int32 do_effect32(BM_REF bm, const RECT32* src_rect, BM_TILE* dst, PRIVATE_SETTINGS* settings)
{
	BM_TILE* src;

	if ((bm == 0) || (dst == 0) || (settings == 0))
		return FILTER_MEM_ERR;

	src = BfBitmapTileGet(bm, src_rect, dst->xmax - dst->xmin, dst->ymax - dst->ymin, TILE_BM_READ_ONLY, 0);
	if (src)
	{
		PIX_F* srcptr;
		PIX_F* dstptr;
		PIX_F	 no_levels;
		Int32	 x, y, inc;

		srcptr = (PIX_F*) src->addr;
		dstptr = (PIX_F*) dst->addr;
		inc = get_PX_CMPNTS(src->px_format);
		no_levels = (PIX_F)settings->no_levels;

		for (y = src->ymin; y < src->ymax; y++)
		{
			for (x = src->xmin; x < src->xmax; x++)
			{
				Int32 i = 0;

				if (bm->image_color_space & CSPACE_ALPHA_FLAG)
				{
					*dstptr++ = *srcptr++;
					i++;
				}

				for (; i < inc; i++)
				{
					PIX_F	value;

					value	 = *srcptr++;
					value *= no_levels;
					value	 = floor(value);
					value /= no_levels;

					*dstptr++ = value;
				}
			}
		}
		BfBitmapTileDetach(bm, src, false);
		return FILTER_OK;
	}
	return FILTER_MEM_ERR;
}

ThresholdDialog::ThresholdDialog(void)
{
	bm = nullptr;
	real_time = BfpGetRealTimeFlag();
	document_preview = BfpGetDocumentPreviewFlag();
}

ThresholdDialog::~ThresholdDialog()
{
	if (GetResult())									// exit dialog with "OK"?
	{
		preview.FinishDocumentUpdate();	// wait until the effect has been applied
		BfpSetDocumentPreviewFlag(document_preview);
	}
}

Bool ThresholdDialog::Init(BM_REF _bm, PRIVATE_SETTINGS* _settings)
{
	if (_bm)
	{
		bm = _bm;
		settings = _settings;
		return true;
	}
	return false;
}

Bool ThresholdDialog::CreateLayout(void)
{
	LoadDialogResource(DLG_THRESHOLD, nullptr, 0);
	preview.Create(this, GADGET_THRESHOLD_PREVIEWGROUP);
	preview.SetSource(bm);
	preview.SetDestination(bm, apply_effect, settings, true, document_preview);

	return true;
}

Bool ThresholdDialog::InitValues(void)
{
	SetInt32(GADGET_THRESHOLD_DOCUMENT_PREVIEW, document_preview);
	SetFloat(GADGET_THRESHOLD_SLIDER_LEVELS, settings->no_levels, MIN_LEVELS, MAX_LEVELS, STEP_LEVELS, FORMAT_FLOAT, 0.0, 0.0, true);

	return true;
}

Bool ThresholdDialog::Command(Int32 id, const BaseContainer& msg)
{
	Bool	update_settings = false;
	Int32	no_levels = settings->no_levels;

	if (preview.Command(id, msg))
		return true;

	switch (id)
	{
		case	GADGET_THRESHOLD_SLIDER_LEVELS:
		{
			if (real_time || (msg.GetInt32(BFM_ACTION_INDRAG) == false))
			{
				no_levels = msg.GetInt32(BFM_ACTION_VALUE);
				update_settings = true;	// indicates changed effect settings
			}
			break;
		}
		case	GADGET_THRESHOLD_DOCUMENT_PREVIEW:
		{
			document_preview = msg.GetInt32(BFM_ACTION_VALUE);
			preview.SetDocumentPreview(document_preview);
			break;
		}
	}

	if (update_settings)
	{
		if (change_scale_table(settings, no_levels, false))
		{
			preview.ChangedSettings();
			preview.Update();
		}
	}

	return true;
}

