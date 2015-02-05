#include "c4d.h"
#include "c4d_symbols.h"
#include "lib_layershader.h"
#include "layershaderbrowser.h"
#include "main.h"

#define LAYER_SHADER_BROWSER_ID	450000054
#define ICON_SIZE								20

class MyTreeViewFunctions : public TreeViewFunctions
{
	void*		GetFirst(void* root, void* userdata)
	{
		LinkBoxGui*	 b = (LinkBoxGui*)root;
		LayerShader* s = (LayerShader*)(b->GetLink(GetActiveDocument(), Xlayer));
		if (!s)
			return nullptr;
		return s->GetFirstLayer();
	}

	void*		GetDown(void* root, void* userdata, void* obj)
	{
		LayerShaderLayer* l = (LayerShaderLayer*)obj;
		if (l->GetType() == TypeFolder)
		{
			GeData d;
			if (l->GetParameter(LAYER_S_PARAM_FOLDER_FIRSTCHILD, d))
				return d.GetVoid();
			return nullptr;
		}
		return nullptr;
	}

	void*		GetNext(void* root, void* userdata, void* obj)
	{
		return ((LayerShaderLayer*)obj)->GetNext();
	}

	Bool IsSelected(void* root, void* userdata, void* obj)
	{
		LayerShaderLayer* l = (LayerShaderLayer*)obj;
		GeData d;
		if (l->GetParameter(LAYER_S_PARAM_ALL_SELECTED, d))
			return d.GetInt32() != 0;
		return false;
	}

	Int32	GetLineHeight(void* root, void* userdata, void* obj, Int32 col, GeUserArea* area)
	{
		return ICON_SIZE + 4;
	}

	Int32 GetColumnWidth(void* root, void* userdata, void* obj, Int32 col, GeUserArea* area)
	{
		return area->DrawGetTextWidth(GetName(root, userdata, obj)) + 5 + ICON_SIZE;
	}

	void DrawCell(void* root, void* userdata, void* obj, Int32 col, DrawInfo* drawinfo, const GeData& bgColor)
	{
		LayerShaderLayer* l = (LayerShaderLayer*)obj;
		if (col == 'tree')
		{
			BaseBitmap* bm = l->GetPreview();
			if (bm)
				drawinfo->frame->DrawBitmap(bm, drawinfo->xpos, drawinfo->ypos + 2, ICON_SIZE, ICON_SIZE, 0, 0, bm->GetBw(), bm->GetBh(), BMP_NORMALSCALED);
			drawinfo->frame->DrawSetTextCol(IsSelected(root, userdata, obj) ? COLOR_TEXT_SELECTED : COLOR_TEXT, COLOR_TRANS);
			drawinfo->frame->DrawText(GetName(root, userdata, obj), drawinfo->xpos + ICON_SIZE + 2,
				drawinfo->ypos + (drawinfo->height - drawinfo->frame->DrawGetFontHeight()) / 2 + 2);
		}
	}

	Bool IsOpened(void* root, void* userdata, void* obj)
	{
		LayerShaderLayer* l = (LayerShaderLayer*)obj;
		if (l->GetType() == TypeFolder)
		{
			GeData d;
			if (l->GetParameter(LAYER_S_PARAM_FOLDER_OPEN, d))
				return d.GetInt32();
			return false;
		}
		return false;
	}

	String GetName(void* root, void* userdata, void* obj)
	{
		LayerShaderLayer* l = (LayerShaderLayer*)obj;
		return l->GetName(GetActiveDocument());
	}

	Int	GetId(void* root, void* userdata, void* obj)
	{
		return 0;
	}

	Int32	GetDragType(void* root, void* userdata, void* obj)
	{
		return NOTOK;
	}

	void Open(void* root, void* userdata, void* obj, Bool onoff)
	{
		LayerShaderLayer* l = (LayerShaderLayer*)obj;
		l->SetParameter(LAYER_S_PARAM_FOLDER_OPEN, GeData((Int32)onoff));
		((LayerShaderBrowser*)userdata)->UpdateAll(true);
	}

	void Select(void* root, void* userdata, void* obj, Int32 mode)
	{
		((LayerShaderBrowser*)userdata)->ShowInfo((LayerShaderLayer*)obj);
	}

} tvf;

/************************************************************************/
/* LayerShaderBrowser                                                   */
/************************************************************************/
LayerShaderBrowser::LayerShaderBrowser()
{
	lastselected = nullptr;
	lastdirty = -1;
}

LayerShaderBrowser::~LayerShaderBrowser()
{
}

Bool LayerShaderBrowser::CreateLayout(void)
{
	if (!GeDialog::CreateLayout())
		return false;
	if (!LoadDialogResource(IDD_SHADER_BROWSER, nullptr, 0))
		return false;

	linkbox = (LinkBoxGui*)FindCustomGui(IDC_LAYER_BROWSER_LINK, CUSTOMGUI_LINKBOX);
	tree = (TreeViewCustomGui*)FindCustomGui(IDC_LAYER_BROWSER_TREE, CUSTOMGUI_TREEVIEW);
	if (!linkbox || !tree)
		return false;

	BaseContainer layout;
	layout.SetInt32('tree', LV_USERTREE);
	tree->SetLayout(1, layout);
	tree->SetRoot(linkbox, &tvf, this);

	return true;
}

Bool LayerShaderBrowser::InitValues(void)
{
	return true;
}

Bool LayerShaderBrowser::Command(Int32 id, const BaseContainer& msg)
{
	switch (id)
	{
		case IDC_LAYER_BROWSER_LINK:
			tree->Refresh();
			break;

		default:
			break;
	}

	return true;
}

Int32 LayerShaderBrowser::Message(const BaseContainer& msg, BaseContainer& result)
{
	switch (msg.GetId())
	{
		case MSG_DESCRIPTION_CHECKDRAGANDDROP:
		{
			Bool*				accept = (Bool*)msg.GetVoid(LINKBOX_ACCEPT_MESSAGE_ACCEPT, nullptr);
			BaseObject* op = (BaseObject*)msg.GetVoid(LINKBOX_ACCEPT_MESSAGE_ELEMENT);
			if (accept && op)
				*accept = op->IsInstanceOf(Xlayer);
			break;
		}
	}
	return GeDialog::Message(msg, result);
}

Bool LayerShaderBrowser::CoreMessage(Int32 id, const BaseContainer& msg)
{
	if (id == EVMSG_CHANGE)
	{
		Int32				l;
		BaseObject* op = (BaseObject*)linkbox->GetLink(GetActiveDocument(), Xlayer);
		if (op)
		{
			l = op->GetDirty(DIRTYFLAGS_DATA);
			if (lastdirty != l)
			{
				lastdirty = l;
				tree->Refresh();
			}
		}
	}
	return true;
}

void LayerShaderBrowser::UpdateAll(Bool msg)
{
	if (msg)
	{
		BaseObject* op = (BaseObject*)linkbox->GetLink(GetActiveDocument(), Xlayer);
		if (op)
			op->Message(MSG_UPDATE);
	}
	tree->Refresh();
}

#define ADD_PARAMETER_B(expr) if (l->GetParameter(expr, d)) str = str + String(# expr) + "    " + String(d.GetInt32() ? "Yes" : "No") + "\n";
#define ADD_PARAMETER_L(expr) if (l->GetParameter(expr, d)) str = str + String(# expr) + "    " + String::IntToString(d.GetInt32()) + "\n";
#define ADD_PARAMETER_R(expr) if (l->GetParameter(expr, d)) str = str + String(# expr) + "    " + String::FloatToString(d.GetFloat(), -1, 3) + "\n";
#define ADD_PARAMETER_V(expr)	\
	if (l->GetParameter(expr, d))	\
	{	\
		Vector v = d.GetVector();	\
		str = str + String(# expr) + "    " + String::FloatToString(v.x, -1, 3) + ", " + String::FloatToString(v.y, -1, 3) + ", " + String::FloatToString(v.z, -1, 3) + "\n";	\
	}

void LayerShaderBrowser::ShowInfo(LayerShaderLayer* l)
{
	String str;
	GeData d;
	if (l)
	{
		ADD_PARAMETER_B(LAYER_S_PARAM_ALL_ACTIVE);
		ADD_PARAMETER_B(LAYER_S_PARAM_ALL_SELECTED);
		ADD_PARAMETER_L(LAYER_S_PARAM_ALL_FLAGS);

		switch (l->GetType())
		{
			case TypeFolder:
				ADD_PARAMETER_L(LAYER_S_PARAM_FOLDER_MODE);
				ADD_PARAMETER_R(LAYER_S_PARAM_FOLDER_BLEND);
				break;

			case TypeShader:
				ADD_PARAMETER_L(LAYER_S_PARAM_SHADER_MODE);
				ADD_PARAMETER_R(LAYER_S_PARAM_SHADER_BLEND);
				if (l->GetParameter(LAYER_S_PARAM_SHADER_LINK, d))
				{
					BaseShader* s = (BaseShader*)((BaseLink*)(d.GetVoid()))->GetLink(GetActiveDocument(), Xbase);
					str += "LAYER_S_PARAM_SHADER_LINK    ";
					if (s)
						str += s->GetName();
					else
						str += "[none]";
					str += "\n";
				}
				break;

			case TypeBrightnessContrast:
				ADD_PARAMETER_R(LAYER_S_PARAM_BC_BRIGHTNESS);
				ADD_PARAMETER_R(LAYER_S_PARAM_BC_CONTRAST);
				ADD_PARAMETER_R(LAYER_S_PARAM_BC_GAMMA);
				break;

			case TypeHSL:
				ADD_PARAMETER_R(LAYER_S_PARAM_HSL_HUE);
				ADD_PARAMETER_R(LAYER_S_PARAM_HSL_SATURATION);
				ADD_PARAMETER_R(LAYER_S_PARAM_HSL_LIGHTNESS);
				ADD_PARAMETER_B(LAYER_S_PARAM_HSL_COLORIZE);
				break;

			case TypePosterize:
				ADD_PARAMETER_L(LAYER_S_PARAM_POSTER_LEVELS);
				ADD_PARAMETER_R(LAYER_S_PARAM_POSTER_WIDTH);
				break;

			case TypeColorize:
				ADD_PARAMETER_L(LAYER_S_PARAM_COLORIZE_INPUT);
				ADD_PARAMETER_B(LAYER_S_PARAM_COLORIZE_OPEN);
				ADD_PARAMETER_B(LAYER_S_PARAM_COLORIZE_CYCLE);
				break;

			case TypeClamp:
				ADD_PARAMETER_R(LAYER_S_PARAM_CLAMP_LOW_CLIP);
				ADD_PARAMETER_R(LAYER_S_PARAM_CLAMP_HIGH_CLIP);
				break;

			case TypeClip:
				ADD_PARAMETER_R(LAYER_S_PARAM_CLIP_LOW_CLIP);
				ADD_PARAMETER_R(LAYER_S_PARAM_CLIP_HIGH_CLIP);
				break;

			case TypeDistorter:
				ADD_PARAMETER_L(LAYER_S_PARAM_DISTORT_NOISE);
				ADD_PARAMETER_R(LAYER_S_PARAM_DISTORT_STRENGTH);
				ADD_PARAMETER_R(LAYER_S_PARAM_DISTORT_OCTACES);
				ADD_PARAMETER_R(LAYER_S_PARAM_DISTORT_TIME_SCALE);
				ADD_PARAMETER_R(LAYER_S_PARAM_DISTORT_NOISE_SCALE);
				ADD_PARAMETER_B(LAYER_S_PARAM_DISTORT_3D_NOISE);
				ADD_PARAMETER_L(LAYER_S_PARAM_DISTORT_WRAP);
				break;

			case TypeTransform:
				ADD_PARAMETER_R(LAYER_S_PARAM_TRANS_ANGLE);
				ADD_PARAMETER_B(LAYER_S_PARAM_TRANS_MIRROR);
				ADD_PARAMETER_B(LAYER_S_PARAM_TRANS_FLIP);
				ADD_PARAMETER_V(LAYER_S_PARAM_TRANS_SCALE);
				ADD_PARAMETER_V(LAYER_S_PARAM_TRANS_MOVE);
				break;

		}
	}
	SetString(IDC_LAYER_BROWSER_PROPS, str);
}

/************************************************************************/
/* LayerShaderBrowseCommand                                             */
/************************************************************************/
class LayerShaderBrowseCommand : public CommandData
{
public:
	LayerShaderBrowseCommand()
	{
		dlg = nullptr;
	}
	virtual ~LayerShaderBrowseCommand()
	{
		DeleteObj(dlg);
	}
	virtual Bool Execute(BaseDocument* doc)
	{
		if (!dlg)
			dlg = NewObjClear(LayerShaderBrowser);
		if (!dlg)
			return false;
		dlg->Open(DLG_TYPE_ASYNC, LAYER_SHADER_BROWSER_ID);
		return true;
	}
	virtual Int32 GetState(BaseDocument* doc)
	{
		return CMD_ENABLED;
	}
	virtual Bool RestoreLayout(void* secret)
	{
		if (!dlg)
			dlg = NewObjClear(LayerShaderBrowser);
		if (!dlg)
			return false;
		dlg->RestoreLayout(LAYER_SHADER_BROWSER_ID, 0, secret);
		return true;
	}

private:
	LayerShaderBrowser* dlg;
};

Bool RegisterLayerShaderBrowser()
{
	return RegisterCommandPlugin(LAYER_SHADER_BROWSER_ID, GeLoadString(IDS_LAYER_SHADER_BROWSER), 0, nullptr, String(), NewObjClear(LayerShaderBrowseCommand));
}
