// MakeDistriDialog.cpp
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "globals.h"
#include "MakeDistriDialog.h"
#include <stdio.h>
#include "DistriCompressProgr.h"
#include "customgui_datetime.h"

#define RES_EDIT_DISTRI_FILTER  1011155
#define RES_EDIT_DISTRI_PRESET  450000074

void SetModalResizeFlag(void* pHandle);

PresetArray g_Presets;
#define ID_PRESET_CLONE      ID_TREEVIEW_FIRST_NEW_ID
Bool CompileCoffeeFile(const Filename &file,Filename *res);

void CopyPreset(PresetElement* pSrc, PresetElement* pDest)
{
	pDest->strOrigin = pSrc->strOrigin;
	pDest->strDestination = pSrc->strDestination;
	pDest->strName = pSrc->strName;
	pDest->bCreateZipFile = pSrc->bCreateZipFile;
	pDest->lCreateZipCompressionLevel = pSrc->lCreateZipCompressionLevel;
	pDest->bCheckVersion = pSrc->bCheckVersion;
	pDest->bParseSymbols = pSrc->bParseSymbols;
	pDest->bWriteBuildInfo = pSrc->bWriteBuildInfo;
	pDest->bRemoveSCC = pSrc->bRemoveSCC;
	Int32 lFilterElements = pSrc->arFilters.GetElementCount();
	FilterElement** ppFilter = pSrc->arFilters.GetArray()->GetArray();
	Int32 a;
	for (a = 0; a < lFilterElements; a++)
	{
		if (!ppFilter[a])
			return;
		FilterElement* pNewFilter = pDest->arFilters.GetNextObject();
		if (!pNewFilter)
			return;
		pNewFilter->lCondition = ppFilter[a]->lCondition;
		pNewFilter->lAction = ppFilter[a]->lAction;
		pNewFilter->str = ppFilter[a]->str;
		pNewFilter->strRename = ppFilter[a]->strRename;
		pNewFilter->bSetXBit = ppFilter[a]->bSetXBit;
	}
}

class PresetListFunctions : public TreeViewFunctions
{
public:
	void*	GetFirst(void *root,void *userdata)
	{
		PresetArray* pArray = (PresetArray*)root;
		if (!pArray)
			return nullptr;
		return pArray->GetObjectAt(0);
	}
	void* GetDown(void *root,void *userdata,void *obj)
	{
		return nullptr;
	}
	void* GetNext(void *root,void *userdata,void *obj)
	{
		Int32 l;
		PresetArray* pArray = (PresetArray*)root;
		Int32 lElements = pArray->GetElementCount();
		PresetElement** ppElements = pArray->GetArray()->GetArray();
		for (l = 0; l < lElements; l++)
		{
			if (ppElements[l] == obj)
			{
				if (l < lElements - 1)
					return ppElements[l + 1];
				return nullptr;
			}
		}
		return nullptr;
	}
	Bool IsSelected(void *root,void *userdata,void *obj)
	{
		PresetElement* pElement = (PresetElement*)obj;
		return ((MakeDistriDialog*)userdata)->IsSelected(pElement);
	}

	void Select(void *root,void *userdata,void *obj,Int32 mode)
	{
		PresetElement* pElement = (PresetElement*)obj;
		if (mode == SELECTION_SUB)
			((MakeDistriDialog*)userdata)->SelectPreset(nullptr);
		else
			((MakeDistriDialog*)userdata)->SelectPreset(pElement);
	}

	Bool IsOpened(void *root,void *userdata,void *obj)
	{
		return false;
	}

	String GetName(void *root,void *userdata,void *obj)
	{
		PresetElement* pElement = (PresetElement*)obj;
		return pElement->strName;
	}

	void SetName(void *root,void *userdata,void *obj, const String &str)
	{
		PresetElement* pElement = (PresetElement*)obj;
		pElement->strName = str;
		((MakeDistriDialog*)userdata)->RefreshPresets();
	}

	void DeletePressed(void *root, void *userdata)
	{
		((MakeDistriDialog*)userdata)->DeleteSelected();
	}

	Int GetId(void *root,void *userdata,void *obj)
	{
		return 0;
	}

	Int32 GetDragType(void *root,void *userdata,void *obj)
	{
		return RES_EDIT_DISTRI_PRESET;
	}

	Int32 AcceptDragObject(void *root,void *userdata,void *obj,Int32 dragtype,void *dragobject, Bool &bAllowCopy)
	{
		bAllowCopy = false;
		if (dragtype == RES_EDIT_DISTRI_PRESET)
			return INSERT_BEFORE | INSERT_AFTER;
		return 0;
	}

	void InsertObject(void *root,void *userdata,void *obj,Int32 dragtype,void *dragobject,Int32 insertmode, Bool bCopy)
	{
		if (dragtype != RES_EDIT_DISTRI_PRESET)
			return;
		if (insertmode != INSERT_BEFORE && insertmode != INSERT_AFTER)
			return;

		PresetArray* pArray = (PresetArray*)root;
		Int32 lElements = pArray->GetElementCount();
		PresetElement **ppElements = pArray->GetArray()->GetArray();
		Int32 a, b;
		Bool bExisted = false;
		for (a = 0, b = 0; a < lElements; a++)
		{
			if (ppElements[a] == dragobject)
			{
				bExisted = true;
				continue;
			}
			ppElements[b++] = ppElements[a];
		}
		if (!bExisted)
			return;
		lElements--;
		DebugAssert(b == lElements);
		if (insertmode == INSERT_BEFORE)
		{
			b = lElements - 1;
			for (a = lElements; a >= 0; a--)
			{
				ppElements[a] = ppElements[b--];
				if (obj == ppElements[a])
				{
					ppElements[--a] = (PresetElement*)dragobject;
				}
			}
		}
		else if (insertmode == INSERT_AFTER)
		{
			b = lElements - 1;
			for (a = lElements; a >= 0; a--)
			{
				if (obj == ppElements[b])
				{
					ppElements[a--] = (PresetElement*)dragobject;
				}
				ppElements[a] = ppElements[b--];
			}
		}
	}

	Int32 GetColumnWidth(void *root, void *userdata, void *obj, Int32 col, GeUserArea* area)
	{
		GeUserArea* pArea = ((MakeDistriDialog*)userdata)->GetPresetListArea();
		PresetElement* pElement = (PresetElement*)obj;
		if (!pArea)
			return 50;
		if (col == 'srcp')
			return pArea->DrawGetTextWidth(pElement->strOrigin);
		else if (col == 'dstp')
			return pArea->DrawGetTextWidth(pElement->strDestination);
		return 0;
	}

	void DrawCell(void *root,void *userdata,void *obj,Int32 col,DrawInfo *drawinfo, const GeData& bgColor)
	{
		PresetElement* pElement = (PresetElement*)obj;
		drawinfo->frame->DrawSetPen(bgColor);
		drawinfo->frame->DrawRectangle(drawinfo->xpos, drawinfo->ypos, drawinfo->xpos + drawinfo->width, drawinfo->ypos + drawinfo->height);
		if (((MakeDistriDialog*)userdata)->IsSelected(pElement))
			drawinfo->frame->DrawSetTextCol(COLOR_TEXT_SELECTED, bgColor);
		else
			drawinfo->frame->DrawSetTextCol(COLOR_TEXT, bgColor);
		if (col == 'srcp')
			drawinfo->frame->DrawText(pElement->strOrigin, drawinfo->xpos, drawinfo->ypos + (drawinfo->height - drawinfo->frame->DrawGetFontHeight()) / 2);
		else if (col == 'dstp')
			drawinfo->frame->DrawText(pElement->strDestination, drawinfo->xpos, drawinfo->ypos + (drawinfo->height - drawinfo->frame->DrawGetFontHeight()) / 2);
		else if (col == 'batc')
		{
			DebugAssert(false);
		}
	}

	Bool MouseDown(void *root,void *userdata,void *obj,Int32 col,MouseInfo *mouseinfo, Bool rightButton)
	{
		return false;
	}

	Int32 GetLineHeight(void *root,void *userdata,void *obj,Int32 col, GeUserArea* area)
	{
		return LMax(13, area->DrawGetFontHeight());
	}

	void CreateContextMenu(void *root, void *userdata, void *obj, Int32 lColumn, BaseContainer* bc)
	{
		bc->RemoveData(ID_TREEVIEW_CONTEXT_RESET);
		bc->SetString(ID_PRESET_CLONE, GeLoadString(IDS_CLONE_PRESET));
	}

	Bool ContextMenuCall(void *root,void *userdata,void *obj, Int32 lColumn, Int32 lCommand)
	{
		if (lCommand == ID_PRESET_CLONE)
		{
			PresetElement* pElement = (PresetElement*)obj;
			PresetArray* pArray = (PresetArray*)root;
			PresetElement* pNew = pArray->GetNextObject();
			if (!pNew)
				return true;
			CopyPreset(pElement, pNew);
			((MakeDistriDialog*)userdata)->RefreshPresets();
			return true;
		}
		return false;
	}

	Bool DoubleClick(void *root,void *userdata,void *obj,Int32 col,MouseInfo *mouseinfo)
	{
		PresetElement* pElement = (PresetElement*)obj;
		RenameDialog(&pElement->strName);
		return true;
	}

	Int32 IsChecked(void *root,void *userdata,void *obj, Int32 lColumn)
	{
		if (lColumn == 'batc')
		{
			PresetElement* pElement = (PresetElement*)obj;
			return LV_CHECKBOX_ENABLED | (pElement->bBatch ? LV_CHECKBOX_CHECKED : 0);
		}
		DebugAssert(false);
		return false;
	}

	void SetCheck(void *root,void *userdata,void *obj, Int32 lColumn, Bool bCheck, const BaseContainer& bcMsg)
	{
		if (lColumn == 'batc')
		{
			PresetElement* pElement = (PresetElement*)obj;
			pElement->bBatch = bCheck;
			return;
		}
		DebugAssert(false);
	}
};

PresetListFunctions g_PresetFuncs;


class FilterListFunctions : public TreeViewFunctions
{
public:
	void*	GetFirst(void *root,void *userdata)
	{
		FilterArray* pArray = (FilterArray*)root;
		if (!pArray)
			return nullptr;
		return pArray->GetObjectAt(0);
	}
	void* GetDown(void *root,void *userdata,void *obj)
	{
		return nullptr;
	}
	void* GetNext(void *root,void *userdata,void *obj)
	{
		Int32 l;
		FilterArray* pArray = (FilterArray*)root;
		Int32 lElements = pArray->GetElementCount();
		FilterElement** ppElements = pArray->GetArray()->GetArray();
		for (l = 0; l < lElements; l++)
		{
			if (ppElements[l] == obj)
			{
				if (l < lElements - 1)
					return ppElements[l + 1];
				return nullptr;
			}
		}
		return nullptr;
	}
	Bool IsSelected(void *root,void *userdata,void *obj)
	{
		FilterElement* pElement = (FilterElement*)obj;
		return ((MakeDistriDialog*)userdata)->IsSelected(pElement);
	}

	void Select(void *root,void *userdata,void *obj,Int32 mode)
	{
		FilterElement* pElement = (FilterElement*)obj;
		if (mode == SELECTION_SUB)
			((MakeDistriDialog*)userdata)->SelectFilter(nullptr);
		else
			((MakeDistriDialog*)userdata)->SelectFilter(pElement);
	}

	Bool IsOpened(void *root,void *userdata,void *obj)
	{
		return false;
	}

	String GetName(void *root,void *userdata,void *obj)
	{
		FilterElement* pElement = (FilterElement*)obj;
		return pElement->str;
	}

	void SetName(void *root,void *userdata,void *obj, const String &str)
	{
		FilterElement* pElement = (FilterElement*)obj;
		pElement->str = str;
		((MakeDistriDialog*)userdata)->RefreshFilters();
	}

	void DeletePressed(void *root, void *userdata)
	{
		((MakeDistriDialog*)userdata)->DeleteSelectedFilter();
	}

	Int GetId(void *root,void *userdata,void *obj)
	{
		return 0;
	}

	Int32 GetDragType(void *root,void *userdata,void *obj)
	{
		return RES_EDIT_DISTRI_FILTER;
	}

	Int32 GetColumnWidth(void *root, void *userdata, void *obj, Int32 col, GeUserArea* area)
	{
		GeUserArea* pArea = ((MakeDistriDialog*)userdata)->GetPresetListArea();
		FilterElement* pElement = (FilterElement*)obj;
		if (!pArea)
			return 50;
		return pArea->DrawGetTextWidth(GetColumnText(pElement, col));
	}

	void DrawCell(void *root,void *userdata,void *obj,Int32 col,DrawInfo *drawinfo, const GeData& bgColor)
	{
		FilterElement* pElement = (FilterElement*)obj;
		drawinfo->frame->DrawSetPen(bgColor);
		drawinfo->frame->DrawRectangle(drawinfo->xpos, drawinfo->ypos, drawinfo->xpos + drawinfo->width, drawinfo->ypos + drawinfo->height);
		drawinfo->frame->DrawSetTextCol(COLOR_TEXT, bgColor);
		drawinfo->frame->DrawText(GetColumnText(pElement, col), drawinfo->xpos, drawinfo->ypos + (drawinfo->height - drawinfo->frame->DrawGetFontHeight()) / 2);
	}

	String GetColumnText(FilterElement* pElement, Int32 lCol)
	{
		Int32 lID = -1;
		String strText;
		if (lCol == 'fcnd')
		{
			if (pElement->lCondition == FILTER_COND_FN_IS) lID = IDS_DISTRI_COND_FILENAME_IS;
			else if (pElement->lCondition == FILTER_COND_FN_EXT) lID = IDS_DISTRI_COND_NAME_EXTENSION;
			else if (pElement->lCondition == FILTER_COND_PATH_CONTAINS) lID = IDS_DISTRI_COND_PATH_CONTAINS;
			else if (pElement->lCondition == FILTER_COND_PATH_IS) lID = IDS_DISTRI_COND_PATH_IS;
			else if (pElement->lCondition == FILTER_COND_DIRECTORY_IS) lID = IDS_DISTRI_COND_DIR_IS;
		}
		else if (lCol == 'fact')
		{
			if (pElement->lAction == FILTER_ACTION_INCLUDE) lID = IDS_DISTRI_ACTION_INCLUDE;
			else if (pElement->lAction == FILTER_ACTION_EXCLUDE) lID = IDS_DISTRI_ACTION_EXCLUDE;
			else if (pElement->lAction == FILTER_ACTION_RENAME) lID = IDS_DISTRI_ACTION_RENAME;
			else if (pElement->lAction == FILTER_ACTION_COMPILE_INCL) lID = IDS_DISTRI_ACTION_COMPILE;
			else if (pElement->lAction == FILTER_ACTION_COMPILE_INCL_KILL) lID = IDS_DISTRI_ACTION_COMPILE_KILL;
			else if (pElement->lAction == FILTER_ACTION_MOVE_TO) lID = IDS_DISTRI_ACTION_MOVETO;
			else if (pElement->lAction == FILTER_ACTION_FORCE_COPY) lID = IDS_DISTRI_FORCE_COPY;
		}
		if (lID == IDS_DISTRI_ACTION_RENAME)
			strText = GeLoadString(IDS_DISTRI_ACTION_RENAME) + " " + pElement->strRename;
		else if (lID == IDS_DISTRI_ACTION_MOVETO)
			strText = GeLoadString(IDS_DISTRI_ACTION_MOVETO) + " " + pElement->strRename;
		else
			strText = lID >= 0 ? GeLoadString(lID) : String();
		if (lCol == 'fact')
		{
			if (pElement->bSetXBit)
				strText += ", Set X Bit";
		}
		return strText;
	}

	Int32 AcceptDragObject(void *root,void *userdata,void *obj,Int32 dragtype,void *dragobject, Bool &bAllowCopy)
	{
		bAllowCopy = false;
		if (dragtype == RES_EDIT_DISTRI_FILTER)
			return INSERT_BEFORE | INSERT_AFTER;
		return 0;
	}

	void InsertObject(void *root,void *userdata,void *obj,Int32 dragtype,void *dragobject,Int32 insertmode, Bool bCopy)
	{
		if (dragtype != RES_EDIT_DISTRI_FILTER)
			return;
		if (insertmode != INSERT_BEFORE && insertmode != INSERT_AFTER)
			return;
		FilterArray* pArray = (FilterArray*)root;
		Int32 lElements = pArray->GetElementCount();
		FilterElement **ppElements = pArray->GetArray()->GetArray();
		Int32 a, b;
		Bool bExisted = false;
		for (a = 0, b = 0; a < lElements; a++)
		{
			if (ppElements[a] == dragobject)
			{
				bExisted = true;
				continue;
			}
			ppElements[b++] = ppElements[a];
		}
		if (!bExisted)
			return;
		lElements--;
		DebugAssert(b == lElements);
		if (insertmode == INSERT_BEFORE)
		{
			b = lElements - 1;
			for (a = lElements; a >= 0; a--)
			{
				ppElements[a] = ppElements[b--];
				if (obj == ppElements[a])
				{
					ppElements[--a] = (FilterElement*)dragobject;
				}
			}
		}
		else if (insertmode == INSERT_AFTER)
		{
			b = lElements - 1;
			for (a = lElements; a >= 0; a--)
			{
				if (obj == ppElements[b])
				{
					ppElements[a--] = (FilterElement*)dragobject;
				}
				ppElements[a] = ppElements[b--];
			}
		}
	}

	Bool DoubleClick(void *root,void *userdata,void *obj,Int32 col,MouseInfo *mouseinfo)
	{
		return true;
	}
};

FilterListFunctions g_FilterFuncs;

MakeDistriDialog::MakeDistriDialog()
{
	m_pSelPreset = nullptr;
	m_bCreateZipFile = false;
	m_lZipCompressionLevel = 9;
	m_bCheckVersion = false;
	m_bParseSymbols = false;
	m_bBuildInfo = false;
	m_pszPassword = nullptr;
}

MakeDistriDialog::~MakeDistriDialog()
{
	DeleteMem(m_pszPassword);
}

Bool MakeDistriDialog::CreateLayout()
{
	if (!GeModalDialog::CreateLayout() || !LoadDialogResource(IDD_MAKE_DISTRI_DLG, nullptr, 0))
		return false;

	m_pPresetList = (TreeViewCustomGui*)FindCustomGui(IDC_DISTR_PRESET_TREE, CUSTOMGUI_TREEVIEW);
	m_pFilterList = (TreeViewCustomGui*)FindCustomGui(IDC_DISTR_FILTER_TREE, CUSTOMGUI_TREEVIEW);
	m_pRootDir = (FilenameCustomGui*)FindCustomGui(IDC_DISTR_ROOT_DIRECTORY, CUSTOMGUI_FILENAME);
	m_pDestDir = (FilenameCustomGui*)FindCustomGui(IDC_DISTR_DESTINATION, CUSTOMGUI_FILENAME);

	if (!m_pPresetList || !m_pFilterList || !m_pRootDir || !m_pDestDir)
		return false;

	BaseContainer bcPresetLayout;
	bcPresetLayout.SetInt32('tree', LV_TREE);
	bcPresetLayout.SetInt32('srcp', LV_USER);
	bcPresetLayout.SetInt32('dstp', LV_USER);
	bcPresetLayout.SetInt32('batc', LV_CHECKBOX);
	m_pPresetList->SetLayout(4, bcPresetLayout);
	m_pPresetList->SetRoot(&g_Presets, &g_PresetFuncs, this);
	m_pPresetList->SetHeaderText('tree', GeLoadString(IDS_PRESET));
	m_pPresetList->SetHeaderText('srcp', GeLoadString(IDS_SRC_PATH));
	m_pPresetList->SetHeaderText('batc', GeLoadString(IDS_BATCH));

	BaseContainer bcFilterLayout;
	bcFilterLayout.SetInt32('tree', LV_TREE);
	bcFilterLayout.SetInt32('fcnd', LV_USER);
	bcFilterLayout.SetInt32('fact', LV_USER);
	m_pFilterList->SetLayout(3, bcFilterLayout);
	m_pFilterList->SetHeaderText('tree', GeLoadString(IDS_FILTER));
	m_pFilterList->SetHeaderText('fcnd', GeLoadString(IDS_FILTER_COND));
	m_pFilterList->SetHeaderText('fact', GeLoadString(IDS_FILTER_ACTION));

	SetModalResizeFlag(GetWindowHandle());

	return true;
}

void SaveResEditPrefs();

Bool MakeDistriDialog::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
	case IDC_DISTR_NEW_PRESET_BTN:
		{
			PresetElement* pNew = g_Presets.GetNextObject();
			if (!pNew)
				return false;
			pNew->strName = GeLoadString(IDS_PRESET);
			SelectPreset(pNew);
			m_pPresetList->MakeVisible(pNew);
			RefreshPresets();
			break;
		}
	case IDC_DISTR_DELETE_PRESET_BTN:
		DeleteSelected();
		break;
	case IDC_DISTR_SAVE_PRESETS_BTN:
		SaveResEditPrefs();
		break;
	case IDC_DISTR_CLONE_BTN:
		if (m_pSelPreset)
		{
			PresetElement* pNew = g_Presets.GetNextObject();
			if (!pNew)
				return true;
			CopyPreset(m_pSelPreset, pNew);
			RefreshPresets();
			return true;
		}
		break;
	case IDC_DISTR_START_BATCH_BTN:
		{
			String strQuestion, str;
			Filename fnOrig, fnDest;
			for (Int32 l = 0; l < g_Presets.GetElementCount(); l++)
			{
				if (g_Presets[l]->bBatch)
				{
					if (strQuestion.GetLength() == 0)
						strQuestion = GeLoadString(IDS_START_DISTRI);

					str = g_Presets[l]->strOrigin;
					g_pStringReplace->Replace(str);
					fnOrig = str;

					str = g_Presets[l]->strDestination;
					g_pStringReplace->Replace(str);
					fnDest = str;

					strQuestion += "\n" + g_Presets[l]->strName + " " + GeLoadString(IDS_FROM) + " " + fnOrig.GetString() + " " + GeLoadString(IDS_TO) + " " + fnDest.GetString();
				}
			}
			if (strQuestion.Content())
			{
				strQuestion += "?";
				String strErrors;
				Bool bCancel = false;
				if (QuestionDialog(strQuestion))
				{
					GetString(IDC_DISTR_BUILDINFO_EDIT, m_strBuildVersion);
					for (Int32 l = 0; l < g_Presets.GetElementCount(); l++)
					{
						if (g_Presets[l]->bBatch)
						{
							if (!MakeDistribution(g_Presets[l], g_Presets[l]->strPassword, &strErrors))
							{
								bCancel = true;
								break;
							}
						}
					}
					if (!strErrors.Content())
					{
						if (!bCancel)
							GeOutString(GeLoadString(IDS_DISTRI_SUCCESS), GEMB_ICONASTERISK);
					}
					else
						GeOutString(strErrors, GEMB_ICONASTERISK | GEMB_MULTILINE);
				}
			}
		}
		break;
	default:
		break;
	}
	if (m_pSelPreset)
	{
		switch (lID)
		{
		case IDC_DISTR_ROOT_DIRECTORY:
			m_pSelPreset->strOrigin = m_pRootDir->GetData().GetValue().GetFilename().GetString();
			break;
		case IDC_DISTR_DESTINATION:
			m_pSelPreset->strDestination = m_pDestDir->GetData().GetValue().GetFilename().GetString();
			break;
		case IDC_DISTR_PASSWORD_EDIT:
			GetString(IDC_DISTR_PASSWORD_EDIT, m_pSelPreset->strPassword);
			break;
		case IDC_DISTR_START_BTN:
			GetString(IDC_DISTR_BUILDINFO_EDIT, m_strBuildVersion);
			MakeDistribution(m_pSelPreset, m_pSelPreset->strPassword, nullptr);
			break;
		case IDC_DISTR_GENERATE_ZIP_CHK:
			GetBool(IDC_DISTR_GENERATE_ZIP_CHK, m_pSelPreset->bCreateZipFile);
			break;
		case IDC_DISTR_GENERATE_ZIP_LEVEL:
			GetInt32(IDC_DISTR_GENERATE_ZIP_LEVEL, m_pSelPreset->lCreateZipCompressionLevel);
			break;
		case IDC_DISTR_CHECK_VERSION_CHK:
			GetBool(IDC_DISTR_CHECK_VERSION_CHK, m_pSelPreset->bCheckVersion);
			break;
		case IDC_DISTR_PARSE_SYMBOLS_CHK:
			GetBool(IDC_DISTR_PARSE_SYMBOLS_CHK, m_pSelPreset->bParseSymbols);
			break;
		case IDC_DISTR_CREATE_BUILDINFO_CHK:
			GetBool(IDC_DISTR_CREATE_BUILDINFO_CHK, m_pSelPreset->bWriteBuildInfo);
			break;
		case IDC_DISTR_REMOVE_SCM_CHK:
			GetBool(IDC_DISTR_REMOVE_SCM_CHK, m_pSelPreset->bRemoveSCC);
			break;
		case IDC_DISTR_FILTER_NEW_BTN:
			{
				Int32 lActive;
				lActive = m_pSelPreset->arFilters.GetObjectIndex(m_pSelFilter);
				FilterElement* pNew;
				if (lActive < 0)
					pNew = m_pSelPreset->arFilters.GetNextObject();
				else
					pNew = m_pSelPreset->arFilters.InsertAt(lActive + 1);
				if (!pNew)
					return false;
				pNew->str = GeLoadString(IDS_FILTER);
				SelectFilter(pNew);
				RefreshFilters();
				break;
			}
		}
	}

	if (m_pSelFilter)
	{
		switch (lID)
		{
		case IDC_DISTR_FILTER_STRING:
			GetString(IDC_DISTR_FILTER_STRING, m_pSelFilter->str);
			RefreshFilters();
			break;
		case IDC_DISTR_FILTER_CONDITION_COMBO:
			{
				Int32 lSel, lFilter = -1;
				GetInt32(IDC_DISTR_FILTER_CONDITION_COMBO, lSel);
				switch (lSel)
				{
				case ID_DISTRI_COND_FILENAME_IS: lFilter = FILTER_COND_FN_IS; break;
				case ID_DISTRI_COND_NAME_EXTENSION: lFilter = FILTER_COND_FN_EXT; break;
				case ID_DISTRI_COND_PATH_CONTAINS: lFilter = FILTER_COND_PATH_CONTAINS; break;
				case ID_DISTRI_COND_PATH_IS: lFilter = FILTER_COND_PATH_IS; break;
				case ID_DISTRI_COND_DIRECTORY_IS: lFilter = FILTER_COND_DIRECTORY_IS; break;
				}
				if (lFilter != -1)
					m_pSelFilter->lCondition = lFilter;
			}
			RefreshFilters();
			break;
		case IDC_DISTR_FILTER_ACTION_COMBO:
			{
				Int32 lSel, lFilter = -1;
				GetInt32(IDC_DISTR_FILTER_ACTION_COMBO, lSel);
				switch (lSel)
				{
				case ID_DISTRI_ACTION_INCLUDE: lFilter = FILTER_ACTION_INCLUDE; break;
				case ID_DISTRI_ACTION_EXCLUDE: lFilter = FILTER_ACTION_EXCLUDE; break;
				case ID_DISTRI_ACTION_RENAME: lFilter = FILTER_ACTION_RENAME; break;
				case ID_DISTRI_ACTION_COMPILE: lFilter = FILTER_ACTION_COMPILE_INCL; break;
				case ID_DISTRI_ACTION_COMPILE_KILL: lFilter = FILTER_ACTION_COMPILE_INCL_KILL; break;
				case ID_DISTRI_ACTION_MOVE_TO: lFilter = FILTER_ACTION_MOVE_TO; break;
				case ID_DISTRI_ACTION_FORCE_COPY: lFilter = FILTER_ACTION_FORCE_COPY; break;
				}
				if (lFilter != -1)
					m_pSelFilter->lAction = lFilter;
			}
			RefreshFilters();
			break;
		case IDC_DISTR_FILTER_DEL_BTN:
			DeleteSelectedFilter();
			break;
		case IDC_DISTR_FILTER_RENAME_EDIT:
			GetString(IDC_DISTR_FILTER_RENAME_EDIT, m_pSelFilter->strRename);
			RefreshFilters();
			break;
		case IDC_DISTR_FILTER_FLAG_EXECUTABLE_CHK:
			GetBool(IDC_DISTR_FILTER_FLAG_EXECUTABLE_CHK, m_pSelFilter->bSetXBit);
			break;
		}
	}

	return true;
}

Bool MakeDistriDialog::InitValues()
{
	if (!GeModalDialog::InitValues()) return false;

	SelectPreset(nullptr);
	SelectFilter(nullptr);
	DoEnable();

	DateTime dt;
	GetDateTimeNow(dt);
	m_strBuildVersion = OldFormatString("%04d%02d%02d_%02d%02d%02d", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
	SetString(IDC_DISTR_BUILDINFO_EDIT, m_strBuildVersion);

	return true;
}

Int32 MakeDistriDialog::Message(const BaseContainer &msg, BaseContainer &result)
{
/*
	switch (msg.GetId())
	{
	default:
		break;
	}
*/
	return GeModalDialog::Message(msg, result);
}

Bool MakeDistriDialog::AskClose()
{
	// return true, if the dialog should not be closed
	return false;
}

void MakeDistriDialog::RefreshPresets()
{
	if (!m_pPresetList)
		return;
	m_pPresetList->Refresh();
}

void MakeDistriDialog::RefreshFilters()
{
	if (!m_pFilterList)
		return;
	m_pFilterList->Refresh();
}

void MakeDistriDialog::DeleteSelected()
{
	if (!m_pSelPreset)
		return;

	if (GeOutString(GeLoadString(IDS_DELETE_DISTRI_PRESET) + " " + m_pSelPreset->strName + "?", GEMB_YESNO |GEMB_ICONQUESTION) != GEMB_R_YES)
		return;

	Int32 l;
	PresetElement** ppElements = g_Presets.GetArray()->GetArray();
	Int32 lElements = g_Presets.GetElementCount();
	for (l = lElements - 1; l >= 0; l--)
	{
		if (m_pSelPreset == ppElements[l])
		{
			String strName = m_pSelPreset->strName;
			GeFKill(GeGetC4DPath(C4D_PATH_RESOURCE) + String("distribution") + (strName + ".prf"));
			GeFKill(GeGetC4DPath(C4D_PATH_PREFS) + String("ResEdit") + (strName + ".prf"));
			SelectPreset(nullptr);
			g_Presets.RemoveAt(l);
			RefreshPresets();
			return;
		}
	}
}

void MakeDistriDialog::DeleteSelectedFilter()
{
	if (!m_pSelPreset || !m_pSelFilter)
		return;

	Int32 l;
	FilterElement** ppElements = m_pSelPreset->arFilters.GetArray()->GetArray();
	Int32 lElements = m_pSelPreset->arFilters.GetElementCount();
	for (l = 0; l < lElements; l++)
	{
		if (m_pSelFilter == ppElements[l])
		{
			SelectFilter(nullptr);
			m_pSelPreset->arFilters.RemoveAt(l);
			RefreshFilters();
		}
	}
}

void MakeDistriDialog::SelectPreset(PresetElement* pElement)
{
	if (m_pSelPreset != pElement)
		SelectFilter(nullptr);
	m_pSelPreset = pElement;
	DoEnable();
	if (!m_pSelPreset)
	{
		m_pRootDir->SetData(TriState<GeData>(GeData(Filename(""))));
		m_pDestDir->SetData(TriState<GeData>(GeData(Filename(""))));
		m_pFilterList->SetRoot(nullptr, &g_FilterFuncs, this);
		SetBool(IDC_DISTR_GENERATE_ZIP_CHK, false);
		SetInt32(IDC_DISTR_GENERATE_ZIP_LEVEL, 1, 1, 9);
		SetBool(IDC_DISTR_CHECK_VERSION_CHK, false);
		SetBool(IDC_DISTR_PARSE_SYMBOLS_CHK, false);
		SetBool(IDC_DISTR_CREATE_BUILDINFO_CHK, false);
		SetBool(IDC_DISTR_REMOVE_SCM_CHK, false);
		SetString(IDC_DISTR_PASSWORD_EDIT, "", 0, EDITTEXT_PASSWORD);
	}
	else
	{
		m_pRootDir->SetData(TriState<GeData>(GeData(Filename(m_pSelPreset->strOrigin))));
		m_pDestDir->SetData(TriState<GeData>(GeData(Filename(m_pSelPreset->strDestination))));
		m_pFilterList->SetRoot(&m_pSelPreset->arFilters, &g_FilterFuncs, this);
		SetBool(IDC_DISTR_GENERATE_ZIP_CHK, m_pSelPreset->bCreateZipFile);
		SetInt32(IDC_DISTR_GENERATE_ZIP_LEVEL, m_pSelPreset->lCreateZipCompressionLevel, 1, 9);
		SetBool(IDC_DISTR_CHECK_VERSION_CHK, m_pSelPreset->bCheckVersion);
		SetBool(IDC_DISTR_PARSE_SYMBOLS_CHK, m_pSelPreset->bParseSymbols);
		SetBool(IDC_DISTR_CREATE_BUILDINFO_CHK, m_pSelPreset->bWriteBuildInfo);
		SetBool(IDC_DISTR_REMOVE_SCM_CHK, m_pSelPreset->bRemoveSCC);
		SetString(IDC_DISTR_PASSWORD_EDIT, m_pSelPreset->strPassword, 0, EDITTEXT_PASSWORD);
	}
}

void MakeDistriDialog::SelectFilter(FilterElement* pElement)
{
	m_pSelFilter = pElement;
	DoEnable();
	if (!m_pSelFilter)
	{
		SetString(IDC_DISTR_FILTER_STRING, "");
		SetString(IDC_DISTR_FILTER_RENAME_EDIT, "");
		SetInt32(IDC_DISTR_FILTER_CONDITION_COMBO, ID_DISTRI_COND_FILENAME_IS);
		SetInt32(IDC_DISTR_FILTER_ACTION_COMBO, ID_DISTRI_ACTION_INCLUDE);
		SetBool(IDC_DISTR_FILTER_FLAG_EXECUTABLE_CHK, false);
	}
	else
	{
		SetString(IDC_DISTR_FILTER_STRING, m_pSelFilter->str);
		SetString(IDC_DISTR_FILTER_RENAME_EDIT, m_pSelFilter->strRename);
		SetBool(IDC_DISTR_FILTER_FLAG_EXECUTABLE_CHK, m_pSelFilter->bSetXBit);
		Int32 lSel = -1;

		switch (m_pSelFilter->lCondition)
		{
		case FILTER_COND_FN_IS : lSel = ID_DISTRI_COND_FILENAME_IS; break;
		case FILTER_COND_FN_EXT : lSel = ID_DISTRI_COND_NAME_EXTENSION; break;
		case FILTER_COND_PATH_CONTAINS : lSel = ID_DISTRI_COND_PATH_CONTAINS; break;
		case FILTER_COND_PATH_IS : lSel = ID_DISTRI_COND_PATH_IS; break;
		case FILTER_COND_DIRECTORY_IS : lSel = ID_DISTRI_COND_DIRECTORY_IS; break;
		}
		if (lSel != -1)
			SetInt32(IDC_DISTR_FILTER_CONDITION_COMBO, lSel);

		lSel = -1;
		switch (m_pSelFilter->lAction)
		{
		case FILTER_ACTION_INCLUDE : lSel = ID_DISTRI_ACTION_INCLUDE; break;
		case FILTER_ACTION_EXCLUDE : lSel = ID_DISTRI_ACTION_EXCLUDE; break;
		case FILTER_ACTION_RENAME : lSel = ID_DISTRI_ACTION_RENAME; break;
		case FILTER_ACTION_COMPILE_INCL : lSel = ID_DISTRI_ACTION_COMPILE; break;
		case FILTER_ACTION_COMPILE_INCL_KILL : lSel = ID_DISTRI_ACTION_COMPILE_KILL; break;
		case FILTER_ACTION_MOVE_TO : lSel = ID_DISTRI_ACTION_MOVE_TO; break;
		case FILTER_ACTION_FORCE_COPY : lSel = ID_DISTRI_ACTION_FORCE_COPY; break;
		}
		if (lSel != -1)
			SetInt32(IDC_DISTR_FILTER_ACTION_COMBO, lSel);
	}
}

void MakeDistriDialog::DoEnable()
{
	Enable(IDC_DISTR_DELETE_PRESET_BTN, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_ROOT_DIRECTORY, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_START_BTN, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_DESTINATION, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_PASSWORD_EDIT, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_CLONE_BTN, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_FILTER_TREE, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_FILTER_NEW_BTN, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_GENERATE_ZIP_CHK, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_GENERATE_ZIP_LEVEL, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_CHECK_VERSION_CHK, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_PARSE_SYMBOLS_CHK, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_CREATE_BUILDINFO_CHK, m_pSelPreset != nullptr);
	Enable(IDC_DISTR_REMOVE_SCM_CHK, m_pSelPreset != nullptr);

	Enable(IDC_DISTR_FILTER_STRING, m_pSelPreset != nullptr && m_pSelFilter != nullptr);
	Enable(IDC_DISTR_FILTER_CONDITION_COMBO, m_pSelPreset != nullptr && m_pSelFilter != nullptr);
	Enable(IDC_DISTR_FILTER_ACTION_COMBO, m_pSelPreset != nullptr && m_pSelFilter != nullptr);
	Enable(IDC_DISTR_FILTER_DEL_BTN, m_pSelPreset != nullptr && m_pSelFilter != nullptr);
	Enable(IDC_DISTR_FILTER_DEL_BTN, m_pSelPreset != nullptr && m_pSelFilter != nullptr);
	Enable(IDC_DISTR_FILTER_FLAG_EXECUTABLE_CHK, m_pSelPreset != nullptr && m_pSelFilter != nullptr);
	Enable(IDC_DISTR_FILTER_RENAME_EDIT, m_pSelPreset != nullptr && m_pSelFilter != nullptr);
}

class DistriThread : public C4DThread
{
public:
	DistriThread() { m_bReady = false; m_bCancel = false; m_pszPassword = nullptr; }
	MakeDistriDialog* m_pDlg;
	Bool m_bReady, m_bCancel;
	PresetElement* m_pElement;
	CCompressProgressDlg* m_pProgressDlg;
	String m_strMissingCDL, m_strMissingCDL64, m_strMissingDYLIB, m_strCDLDebug, m_strMissingPDB;
	String* m_pstrError;
	char* m_pszPassword;

	void Main(void)
	{
		while (!m_bReady) { GeSleep(30); }
		m_pDlg->MakeDistributionThread(m_pElement, m_pProgressDlg, this);
		m_pProgressDlg->CloseDialog();
	}
	virtual const Char *GetThreadName(void) { return "DistriThread"; }
};

Bool MakeDistriDialog::MakeDistribution(PresetElement* pElement, const String &strPassword, String* pstrError)
{
	if (!pElement)
		return true;

	if (m_pszPassword)
		DeleteMem(m_pszPassword);

	if (strPassword.Content())
	{
		if (strPassword == "($LANGUAGE_PASSWORD$)")
		{
			char pszPassword[100];
			Int32 lVersion = GetC4DVersion(), lVersionCheck = 0, n = lVersion;
			while (n > 0)
			{
				lVersionCheck += (n % 10);
				n /= 10;
			}
			sprintf_safe(pszPassword, sizeof(pszPassword), "%d.%3d%d", (int)(lVersion / 1000), (int)(lVersion % 1000), (int)lVersionCheck);

			Int32 lLen = (Int32)strlen(pszPassword) + 2;
			m_pszPassword = NewMemClear(Char,lLen + 2);
			if (!m_pszPassword)
				return true;
			strcpy(m_pszPassword, pszPassword);
		}
		else
		{
			Int32 lLen = strPassword.GetCStringLen(STRINGENCODING_8BIT);
			m_pszPassword = NewMemClear(Char,lLen + 2);
			if (!m_pszPassword)
				return true;
			strPassword.GetCString(m_pszPassword, lLen + 1, STRINGENCODING_8BIT);
		}
	}

	DistriThread th;
	CCompressProgressDlg dlg(&th.m_bReady, &th.m_bCancel);
	th.m_pDlg = this;
	th.m_pElement = pElement;
	th.m_pProgressDlg = &dlg;
	th.m_pszPassword = m_pszPassword;
	th.m_pstrError = pstrError;

	/*GetBool(IDC_DISTR_GENERATE_ZIP_CHK, m_bCreateZipFile);
	GetBool(IDC_DISTR_CHECK_VERSION_CHK, m_bCheckVersion);
	GetBool(IDC_DISTR_PARSE_SYMBOLS_CHK, m_bParseSymbols);
	GetBool(IDC_DISTR_CREATE_BUILDINFO_CHK, m_bBuildInfo);*/
	m_bCreateZipFile = pElement->bCreateZipFile;
	m_lZipCompressionLevel = pElement->lCreateZipCompressionLevel;
	m_bCheckVersion = pElement->bCheckVersion;
	m_bParseSymbols = pElement->bParseSymbols;
	m_bBuildInfo = pElement->bWriteBuildInfo;
	m_bRemoveSCC = pElement->bRemoveSCC;
	m_fnCinemaRes = GeGetStartupPath() + String("resource");

	if (GeGetSystemInfo()&SYSTEMINFO_COMMANDLINE)
		th.m_bReady = true;

	if (!th.Start(GeGetSystemInfo()&SYSTEMINFO_COMMANDLINE ? THREADMODE_DEPRECATED_SYNCHRONOUS : THREADMODE_ASYNC ))
		CriticalStop();

	if (!(GeGetSystemInfo()&SYSTEMINFO_COMMANDLINE))
	{
		dlg.Open(-2, -2, 600);
	}

	if (!(GeGetSystemInfo()&SYSTEMINFO_COMMANDLINE) && th.m_bCancel)
		GeOutString(GeLoadString(IDS_DISTRI_CACNEL), GEMB_ICONASTERISK);
	else
	{
		String strOut;
		if (th.m_strMissingCDL.Content())
		{
			strOut = GeLoadString(IDS_MISSING_CDL);
			strOut += "|";
			strOut += th.m_strMissingCDL;
		}
		if (th.m_strMissingCDL64.Content())
		{
			if (strOut.Content())
				strOut += "|";
			strOut += GeLoadString(IDS_MISSING_CDL64);
			strOut += "|";
			strOut += th.m_strMissingCDL64;
		}
		if (th.m_strMissingDYLIB.Content())
		{
			if (strOut.Content())
				strOut += "|";
			strOut += GeLoadString(IDS_MISSING_DYLIB);
			strOut += "|";
			strOut += th.m_strMissingDYLIB;
		}
		if (th.m_strMissingPDB.Content())
		{
			if (strOut.Content())
				strOut += "|";
			strOut += GeLoadString(IDS_MISSING_PDB);
			strOut += "|";
			strOut += th.m_strMissingPDB;
		}
		if (th.m_strCDLDebug.Content())
		{
			if (strOut.Content())
				strOut += "|";
			strOut += GeLoadString(IDS_CDL_DEBUG);
			strOut += "|";
			strOut += th.m_strCDLDebug;
		}
		if (!pstrError)
		{
			if (!strOut.Content())
			{
				strOut = GeLoadString(IDS_DISTRI_SUCCESS);
				if (!(GeGetSystemInfo()&SYSTEMINFO_COMMANDLINE))
					GeOutString(GeLoadString(IDS_DISTRI_SUCCESS), GEMB_ICONASTERISK);
				else
					GeConsoleOut("@MakeDistri successful build");
			}
			else
			{
				if (!(GeGetSystemInfo()&SYSTEMINFO_COMMANDLINE))
					GeOutString(strOut, GEMB_ICONASTERISK | GEMB_MULTILINE);
				else
				{
					GeConsoleOut("@MakeDistri warnings:");
					GeOutString(strOut, GEMB_ICONASTERISK);
					GeConsoleOut("@MakeDistri successful build");
				}
			}
		}
		else
		{
			if (strOut.Content())
			{
				*pstrError += pElement->strName;
				*pstrError += ":\n";
				*pstrError += strOut + "\n";
			}
		}
	}
	return !th.m_bCancel;
}


void MakeDistriDialog::MakeDistributionThread(PresetElement* pElement, CCompressProgressDlg *pProgressDlg, DistriThread* pThread)
{
	g_pStringReplace->Init(m_strBuildVersion);

	String str;
	Filename fnOrig, fnDest;
	str = pElement->strOrigin;
	g_pStringReplace->Replace(str);
	fnOrig = str;

	str = pElement->strDestination;
	g_pStringReplace->Replace(str);
	fnDest = str;

	Int32 l;

	if (m_bCreateZipFile)
	{
		if (!fnDest.CheckSuffix("zip"))
			fnDest = fnDest.GetString() + ".zip";
	}

	if (GeGetSystemInfo()&SYSTEMINFO_COMMANDLINE)
		GeConsoleOut("@MakeDistri Result: "+fnDest.GetString());

	if (!m_bCreateZipFile && fnDest.GetString().FindFirst(fnOrig.GetString(), &l) && l == 0)
	{
		GeOutString(GeLoadString(IDS_DEST_IN_SOURCE), GEMB_ICONEXCLAMATION);
		pThread->m_bCancel = true;
		return;
	}
	else if (!pThread->m_pstrError && !(GeGetSystemInfo()&SYSTEMINFO_COMMANDLINE))
	{
		if (GeOutString(GeLoadString(IDS_START_DISTRI) + " " + pElement->strName + " " + GeLoadString(IDS_FROM) +
				" " + fnOrig.GetString() + " " + GeLoadString(IDS_TO) + " " + fnDest.GetString() + "?", GEMB_ICONQUESTION | GEMB_YESNO) != GEMB_R_YES)
		{
			pThread->m_bCancel = true;
			return;
		}
	}

	ZipFile* pZipFile = nullptr;
	Filename fn;

	if (m_bCreateZipFile)
	{
		pZipFile = ZipFile::Alloc();
		if (!pZipFile)
			goto _exit;
		if (!pZipFile->Open(fnDest, false, false))
			goto _exit;
		pZipFile->SetCompressionLevel(m_lZipCompressionLevel);
	}

	if (m_bParseSymbols)
	{
		ParseSymbols();
	}

	if (!CopyFiles(fnDest, fnOrig, fn, pElement, pZipFile, pProgressDlg, pThread))
	{
		DebugAssert(false);
	}
	//SetFileTime(fnDest, 2001, 4, 3, 12, 34, 56, false);

_exit:
	if (pZipFile)
	{
		pZipFile->Close();
		ZipFile::Free(pZipFile);
	}
}

inline Char UpperChar(UChar w)
{
	if ((w>96 && w<123) || w==0xe4 || w==0xf6 || w==0xfc) // seems to be massively faster then toupper on XCode !!!
		return (Char)(w-32);
	return (Char)w;
}

static void RemoveString(Char *mem, const Char *search, Int32 len, Bool xcode)
{
	Char *pos = strstr(mem,search);
	if (pos)
	{
		while (pos<mem+len && (*pos!=10 && *pos!=13 && *pos != '>'))
		{
			if (xcode && *pos==' ') break;
			*pos++ = ' ';
		}
	}
}

static void RemoveLine(Char *mem, const Char *search, Int32 len)
{
	Char *pos = strstr(mem,search);
	if (!pos) return;

	while (pos<mem+len && (*pos!=10 && *pos!=13))
		*pos++ = ' ';
}

void Insert(Char *at, const Char *q)
{
	char *z = at;
	Int32 lq = (Int32)strlen(q), lz = (Int32)strlen(z), i;
	for (i = lz; i >= 0; i--) z[i + lq] = z[i];
	for (i = 0; i < lq; i++) z[i] = q[i];
	z[lq + lz] = 0;
}

void Delete(Char *s, Int32 anz)
{
	if (anz > 0)
	{
		Int32 i, l = (Int32)strlen((char *)s);
		if (l < anz)
			anz = l;

		char *z = s, *q = z + anz;
		l = (Int32)strlen(q);
		for (i = 0; i < l; i++) z[i] = q[i];
		z[i] = 0;
	}
}

static Bool ReplaceString(Char *mem, const Char *search, const Char *insert)
{
	Char *pos = strstr(mem,search);
	if (pos)
	{
		Delete(pos, (Int32)strlen(search));
		Insert(pos,insert);
		return true;
	}
	return false;
}

static Bool CreateTempVCFile(Filename &fn)
{
	Filename backup = fn;
	fn = GeGetC4DPath(C4D_PATH_PREFS) + Filename("tmp.tmp");

	AutoAlloc<BaseFile> bf1,bf2;
	if (bf1 && bf2 && bf1->Open(backup,FILEOPEN_READ) && bf2->Open(fn,FILEOPEN_WRITE))
	{
		Int32 len=bf1->GetLength();
		Char *mem = NewMemClear(Char,len+1); // one char bigger, so it's null terminated
		if (!mem) return false;

		Bool ok = bf1->ReadBytes(mem,len)==len;

		if (ok)
		{
			RemoveString(mem,"SccProjectName=",len,false);
			RemoveString(mem,"SccLocalPath=",len,false);
			RemoveString(mem,"SccProvider=",len,false);
			RemoveString(mem,"# PROP Scc_ProjName",len,false);
			RemoveString(mem,"# PROP Scc_LocalPath",len,false);
			RemoveString(mem,"# PROP Scc_Provider",len,false);

			RemoveLine(mem,"<SccProjectName>",len);
			RemoveLine(mem,"<SccLocalPath>",len);
			RemoveLine(mem,"<SccProvider>",len);
		}

		ok = ok && bf2->WriteBytes(mem,len);
		DeleteMem(mem);

		if (ok) return true;
	}

	return false;
}

static Bool CreateTempVSPropsFile(Filename &fn)
{
	Filename backup = fn;
	fn = GeGetC4DPath(C4D_PATH_PREFS) + Filename("tmp.tmp");

	AutoAlloc<BaseFile> bf1,bf2;
	if (bf1 && bf2 && bf1->Open(backup,FILEOPEN_READ) && bf2->Open(fn,FILEOPEN_WRITE))
	{
		Int32 len=bf1->GetLength();
		Char *mem = NewMemClear(Char,len+1+2000); // one char bigger, so it's null terminated + additional 2KB for multiple search & replaces
		if (!mem) return false;

		Bool ok = bf1->ReadBytes(mem,len)==len;

		if (ok)
		{
			while (ReplaceString(mem,"$(WebDeployPath)","..\\..\\resource")) { }
		}

		ok = ok && bf2->WriteBytes(mem,strlen(mem));
		DeleteMem(mem);

		if (ok) return true;
	}

	return false;
}

static Bool CreateTempXCodeFile(Filename &fn)
{
	Filename backup = fn;
	fn = GeGetC4DPath(C4D_PATH_PREFS) + Filename("tmp.tmp");

	AutoAlloc<BaseFile> bf1,bf2;
	if (bf1 && bf2 && bf1->Open(backup,FILEOPEN_READ) && bf2->Open(fn,FILEOPEN_WRITE))
	{
		Int32 len=bf1->GetLength();
		Char *mem = NewMemClear(Char,len+1); // one char bigger, so it's null terminated
		if (!mem) return false;

		Bool ok = bf1->ReadBytes(mem,len)==len;

		if (ok)
		{
			RemoveString(mem,"$(C4D_ROOTDIR)/resource/_api_maxon/lib_api_maxon_debug.a",len,true); // don't change the order!
			RemoveString(mem,"$(C4D_ROOTDIR)/resource/_api_maxon/lib_api_maxon_release.a",len,true); // don't change the order!
			RemoveString(mem,"$(C4D_ROOTDIR)/resource/_api_maxon/**",len,true);
			RemoveString(mem,"$(C4D_ROOTDIR)/resource/_api_maxon",len,true);
		}

		ok = ok && bf2->WriteBytes(mem,len);
		DeleteMem(mem);

		if (ok) return true;
	}

	return false;
}

Bool MakeDistriDialog::CopyFiles(const Filename &fnDestPath, Filename fnCurrentDir, Filename fnDest, PresetElement* pElement,
																 ZipFile* pZipFile, CCompressProgressDlg *pProgressDlg, DistriThread* pThread, Bool bPathExists)
{
	AutoAlloc <BrowseFiles> pBrowse;
	if (!pBrowse)
		return false;

	pBrowse->Init(fnCurrentDir, false);

	Filename fnBrowse;
	Filename fnTemp;
	Filename fnCurrent;
	Filename fnDestFile;
	Filename fnCompile;
	UInt32 ulFileAttributes, ulExternalZipAttributes, ulInternalZipAttributes;

	while (pBrowse->GetNext())
	{
		if (pThread->m_bCancel)
			return true;

		fnBrowse = pBrowse->GetFilename();
		if (!fnBrowse.Content())
			continue;
		fnCurrent = fnCurrentDir + fnBrowse;

#ifdef _DEBUG
		//Int32 lPos;
		//DebugAssert(!fnCurrent.GetString().FindFirst("_api_maxon", &lPos));
#endif

		FilterElement* pFilter = IncludeElement(fnCurrent, pBrowse->IsDir(), pElement);
		if (!pFilter && !pBrowse->IsDir())
			continue;

		if (pBrowse->IsDir())
		{
			if (m_bBuildInfo && fnCurrent == m_fnCinemaRes)
			{
				FilterElement* pBuildFilter = IncludeElement(fnCurrent + String("build.txt"), pBrowse->IsDir(), pElement);
				if (!pBuildFilter)
				{
					if (m_bCreateZipFile)
					{
						WriteBuildInfo(RenamePath((fnDest + fnBrowse + String("build.txt")).GetString(), pElement), pZipFile);
					}
					else
					{
						fnDestFile = fnDest + fnBrowse;
						fnDestFile = RenamePath(fnDestFile.GetString(), pElement);
						if (!bPathExists)
						{
							if (!AssertPathExists(fnDestFile))
								return false;
							bPathExists = true;
						}
						fnDestFile = fnDest + fnBrowse;
						fnDestFile = RenamePath(fnDestFile.GetString(), pElement);

						WriteBuildInfo(fnDestFile + String("build.txt"), nullptr);
					}
				}
			}

			if (!CopyFiles(fnDestPath, fnCurrent, fnDest + fnBrowse, pElement, pZipFile, pProgressDlg, pThread))
				return false;

			// check if we hit the empty directory rule
			if (pFilter)
			{
				if (pFilter->lCondition == FILTER_COND_DIRECTORY_IS && pFilter->lAction == FILTER_ACTION_FORCE_COPY)
				{
					if (m_bCreateZipFile)
					{
						String str;
						ZipFile::CreateLocalFileName(fnDest + fnBrowse, str);
						ZipFileTime t;
						if (!GetFileTime(fnCurrent, t.nYear, t.nMonth, t.nMDay, t.nHour, t.nMin, t.nSec, FILETIME_MODIFIED, true))
						{
							t.nYear = 2000;
							t.nMonth = 0;
						}
						else
							t.nMonth--;
						str = RenamePath(str, pElement);
						pZipFile->CreateDirectoryInZip(str, &t);
					}
					else
					{
						fnDestFile = fnDestPath;
						AddFilename(fnDestFile, fnDest);
						fnDestFile += fnBrowse;
						fnDestFile = RenamePath(fnDestFile.GetString(), pElement);
						AssertPathExists(fnDestFile);
					}
				}
			}
		}
		else
		{ // no directory
			if (m_bCheckVersion)
			{
				if (fnCurrent.CheckSuffix("cdl") || fnCurrent.CheckSuffix("cdl64"))
				{
					Filename fnTest = fnCurrent;
					Filename cn = fnCurrent.GetFile(); cn.ClearSuffix();
					if (fnCurrent.CheckSuffix("cdl"))
					{
						fnTest.SetSuffix("cdl64");
						if (!GeFExist(fnTest) && cn!=String("pswrap") && cn!=String("bugslife_server")&& cn!=String("fbx5") && cn!=String("openexr") && cn!=String("shockwave3d") && cn!=String("uzr"))
							pThread->m_strMissingCDL64 += "  " + fnTest.GetFileString() + "|";
						fnTest.SetSuffix("dylib");
						if (!GeFExist(fnTest) && cn!=String("bp2max") && cn!=String("bp2xsi") && cn!=String("bugslife_server"))
							pThread->m_strMissingDYLIB += "  " + fnTest.GetFileString() + "|";
					}

					fnTest.SetSuffix("ilk");
					if (GeFExist(fnTest))
						pThread->m_strCDLDebug += "  " + fnTest.GetFileString() + "|";

					if (cn!=String("flashex") && cn!=String("shockwave3d") && cn!=String("uzr") && cn!=String("pswrap") &&
							cn!=String("openexr") && cn!=String("fbx5") && cn!=String("fbx6"))
					{
						AutoAlloc <BaseFile> pFile;
						Char* pFileData = nullptr;
						Int n, lFileLen;
						Char* pszNB10Name = nullptr;
						Filename fnPDB;
						do
						{
							if (!pFile)
								break;
							if (!pFile->Open(fnCurrent))
								break;
							lFileLen = pFile->GetLength();
							pFileData = (lFileLen > 0) ? NewMem(Char,lFileLen) : nullptr;
							if (!pFileData)
								break;
							if (!pFile->ReadBytes(pFileData, lFileLen))
								break;

							for (n=0; n<lFileLen; n++)
								pFileData[n]=UpperChar((UChar)pFileData[n]);

							Int32 strlen=9;

							lFileLen -= strlen;
							for (n = 0; n < lFileLen; n++)
							{
								if (!strncmp(pFileData+n,"INTEL.PDB",strlen))
									break;
							}
							if (n == lFileLen)
								n = 0;
							if (!n)
								break;

							while (n>0 && pFileData[n-1])
								n--;

							pszNB10Name = &pFileData[n];
							fnPDB = String(pszNB10Name);

							if (!GeFExist(fnPDB))
								pThread->m_strMissingPDB += "  " + fnPDB.GetFileString() + "|";

						} while(0);
						if (!pszNB10Name)
							pThread->m_strMissingPDB += "  " + fnCurrent.GetFileString() + "|";

						if (pFile)
							pFile->Close();
						DeleteMem(pFileData);
					}
				}
				else if (fnCurrent.CheckSuffix("dylib"))
				{
					Filename fnTest = fnCurrent;
					fnTest.SetSuffix("cdl");
					if (!GeFExist(fnTest))
						pThread->m_strMissingCDL += "  " + fnTest.GetFileString() + "|";
				}
			}
			if (m_bBuildInfo && (fnCurrent.CheckSuffix("cdl") || fnCurrent.CheckSuffix("cdl64") || fnCurrent.CheckSuffix("so")))
			{
				FilterElement* pBuildFilter = IncludeElement(fnCurrent.GetDirectory() + String("res") + String("build.txt"), pBrowse->IsDir(), pElement);
				if (!pBuildFilter)
				{
					if (m_bCreateZipFile)
						WriteBuildInfo(RenamePath((fnDest + String("res") + String("build.txt")).GetString(), pElement), pZipFile);
					else
					{
						fnDestFile = fnDestPath;
						AddFilename(fnDestFile, fnDest + String("res"));
						fnDestFile = RenamePath(fnDestFile.GetString(), pElement);
						if (!bPathExists)
						{
							if (!AssertPathExists(fnDestFile))
								return false;
							bPathExists = true;
						}
						fnDestFile = fnDestPath;
						AddFilename(fnDestFile, fnDest + String("res"));
						fnDestFile += String("build.txt");
						fnDestFile = RenamePath(fnDestFile.GetString(), pElement);
						WriteBuildInfo(fnDestFile, nullptr);
					}
				}
			}
			if (pFilter->lAction == FILTER_ACTION_RENAME)
			{
				if (pFilter->lCondition == FILTER_COND_FN_IS)
					fnBrowse = pFilter->strRename;
				else if (pFilter->lCondition == FILTER_COND_FN_EXT)
					fnBrowse.SetSuffix(pFilter->strRename);
			}
			else if (pFilter->lAction == FILTER_ACTION_COMPILE_INCL || pFilter->lAction == FILTER_ACTION_COMPILE_INCL_KILL)
			{
				if (!CompileCoffeeFile(fnCurrent, &fnCompile))
				{
					return false;
				}
				fnBrowse = fnCompile.GetFile();
				fnCurrent = fnCurrent.GetDirectory() + fnCompile.GetFile();
			}
			else if (pFilter->lAction == FILTER_ACTION_INCLUDE && fnCurrent.CheckSuffix("dylib_stripped"))
			{
				fnBrowse.ClearSuffixComplete();
				fnBrowse = Filename(fnBrowse.GetString() + String(".dylib"));
			}
			if (m_bCreateZipFile)
			{
				String str;
				if (pFilter->lAction == FILTER_ACTION_MOVE_TO)
					ZipFile::CreateLocalFileName(Filename(pFilter->strRename), str);
				else
				{
					ZipFile::CreateLocalFileName(RenamePath((fnDest + fnBrowse).GetString(), pElement), str);
				}

_zip_copy_retry:

				fnTemp = fnCurrent;
				if (m_bRemoveSCC)
				{
					if (fnCurrent.CheckSuffix("VCPROJ") || fnCurrent.CheckSuffix("VCXPROJ") || fnCurrent.CheckSuffix("DSP"))
					{
						if (!CreateTempVCFile(fnTemp))
							return false;
					}
					else if (fnCurrent.CheckSuffix("VSPROPS"))
					{
						if (!CreateTempVSPropsFile(fnTemp))
							return false;
					}
					else if (fnCurrent.CheckSuffix("XCCONFIG"))
					{
						if (!CreateTempXCodeFile(fnTemp))
							return false;
					}
				}

				ulFileAttributes = GeFGetAttributes(fnTemp);
				ulExternalZipAttributes = ZIP_FILE_FLAG_USE_UNIX_FLAGS;
				ulInternalZipAttributes = ZIP_FILE_FLAG_UNIX_ATTR;
				if (ulFileAttributes != (UInt32)-1)
				{
#if defined __MAC || defined __LINUX
					if (ulFileAttributes & GE_FILE_ATTRIBUTE_OWNER_R) ulExternalZipAttributes |= ZIP_FILE_FLAG_OWNER_R;
					if (ulFileAttributes & GE_FILE_ATTRIBUTE_OWNER_W) ulExternalZipAttributes |= ZIP_FILE_FLAG_OWNER_W;
					if (ulFileAttributes & GE_FILE_ATTRIBUTE_OWNER_X) ulExternalZipAttributes |= ZIP_FILE_FLAG_OWNER_X;
					if (ulFileAttributes & GE_FILE_ATTRIBUTE_GROUP_R) ulExternalZipAttributes |= ZIP_FILE_FLAG_GROUP_R;
					if (ulFileAttributes & GE_FILE_ATTRIBUTE_GROUP_W) ulExternalZipAttributes |= ZIP_FILE_FLAG_GROUP_W;
					if (ulFileAttributes & GE_FILE_ATTRIBUTE_GROUP_X) ulExternalZipAttributes |= ZIP_FILE_FLAG_GROUP_X;
					if (ulFileAttributes & GE_FILE_ATTRIBUTE_PUBLIC_R) ulExternalZipAttributes |= ZIP_FILE_FLAG_PUBLIC_R;
					if (ulFileAttributes & GE_FILE_ATTRIBUTE_PUBLIC_W) ulExternalZipAttributes |= ZIP_FILE_FLAG_PUBLIC_W;
					if (ulFileAttributes & GE_FILE_ATTRIBUTE_PUBLIC_X) ulExternalZipAttributes |= ZIP_FILE_FLAG_PUBLIC_X;
#endif
					if (ulFileAttributes & GE_FILE_ATTRIBUTE_READONLY)
						ulExternalZipAttributes |= ZIP_FLAG_READONLY;
#if __PC
					ulExternalZipAttributes |= ZIP_FILE_FLAG_OWNER_R | ZIP_FILE_FLAG_GROUP_R | ZIP_FILE_FLAG_PUBLIC_R;
					if (ulFileAttributes & GE_FILE_ATTRIBUTE_READONLY)
						ulExternalZipAttributes |= ZIP_FLAG_READONLY;
					else
						ulExternalZipAttributes |= ZIP_FILE_FLAG_OWNER_W | ZIP_FILE_FLAG_GROUP_W | ZIP_FILE_FLAG_PUBLIC_W;
#endif
				}
				if (pFilter->bSetXBit)
				{
#if !defined __MAC && !defined __LINUX
					ulExternalZipAttributes |= (ZIP_FILE_FLAG_OWNER_R | ZIP_FILE_FLAG_GROUP_R | ZIP_FILE_FLAG_PUBLIC_R);
					if (!(ulFileAttributes & GE_FILE_ATTRIBUTE_READONLY))
						ulExternalZipAttributes |= (ZIP_FILE_FLAG_OWNER_W | ZIP_FILE_FLAG_GROUP_W | ZIP_FILE_FLAG_PUBLIC_W);
#endif
					ulExternalZipAttributes |= (ZIP_FILE_FLAG_OWNER_X | ZIP_FILE_FLAG_GROUP_X | ZIP_FILE_FLAG_PUBLIC_X);
				}

				if (!pZipFile->CopyInFileInZip(fnTemp, str, ulInternalZipAttributes, ulExternalZipAttributes, m_pszPassword))
				{
					GEMB_R lDlg = GeOutString(GeLoadString(IDS_ERROR) + " " + fnCurrent.GetString() + " " + GeLoadString(IDS_FILECOPY_ERROR) + " " + str + ".",
						GEMB_ICONEXCLAMATION | GEMB_ABORTRETRYIGNORE);
					if (lDlg == GEMB_R_ABORT)
						return false;
					else if (lDlg == GEMB_R_RETRY)
						goto _zip_copy_retry;
				}
				pProgressDlg->SetStrings(fnCurrent.GetString(), str);
			}
			else
			{
				if (pFilter->lAction == FILTER_ACTION_MOVE_TO)
				{
					fnDestFile = fnDestPath;
					AddFilename(fnDestFile, Filename(pFilter->strRename));
					fnDestFile = RenamePath(fnDestFile.GetString(), pElement);
					AssertPathExists(fnDestFile.GetDirectory());
_file_move_retry:
					if (!GeFCopyFile(fnCurrent, fnDestFile, GE_FCOPY_OVERWRITE))
					{
						GEMB_R lDlg = GeOutString(GeLoadString(IDS_ERROR) + " " + fnCurrent.GetString() + " " + GeLoadString(IDS_FILECOPY_ERROR) + " " + fnDestFile.GetString() + ".",
							GEMB_ICONEXCLAMATION | GEMB_ABORTRETRYIGNORE);
						if (lDlg == GEMB_R_ABORT)
							return false;
						else if (lDlg == GEMB_R_RETRY)
							goto _file_move_retry;
					}
				}
				else
				{
					fnDestFile = fnDestPath;
					AddFilename(fnDestFile, fnDest);
					fnDestFile = RenamePath(fnDestFile.GetString(), pElement);
					if (!bPathExists)
					{
						if (!AssertPathExists(fnDestFile))
							return false;
						bPathExists = true;
					}
					fnDestFile = fnDestPath;
					AddFilename(fnDestFile, fnDest);
					fnDestFile += fnBrowse;
					fnDestFile = RenamePath(fnDestFile.GetString(), pElement);
				}

_file_copy_retry:

				fnTemp = fnCurrent;

				if (fnCurrent.CheckSuffix("VCPROJ") || fnCurrent.CheckSuffix("VCXPROJ") || fnCurrent.CheckSuffix("DSP"))
				{
					if (!CreateTempVCFile(fnTemp))
						return false;
				}
				else if (fnCurrent.CheckSuffix("VSPROPS"))
				{
					if (!CreateTempVSPropsFile(fnTemp))
						return false;
				}
				else if (fnCurrent.CheckSuffix("XCCONFIG"))
				{
					if (!CreateTempXCodeFile(fnTemp))
						return false;
				}

				if (!GeFCopyFile(fnTemp, fnDestFile, GE_FCOPY_OVERWRITE))
				{
					GEMB_R lDlg = GeOutString(GeLoadString(IDS_ERROR) + " " + fnCurrent.GetString() + " " + GeLoadString(IDS_FILECOPY_ERROR) + " " + fnDestFile.GetString() + ".",
						GEMB_ICONEXCLAMATION | GEMB_ABORTRETRYIGNORE);
					if (lDlg == GEMB_R_ABORT)
						return false;
					else if (lDlg == GEMB_R_RETRY)
						goto _file_copy_retry;
				}
				pProgressDlg->SetStrings(fnCurrent.GetString(), fnDestFile.GetString());
			}
			if (pFilter->lAction == FILTER_ACTION_COMPILE_INCL_KILL)
			{
				GeFKill(fnCompile);
			}
		}
	}

	return true;
}

FilterElement* MakeDistriDialog::IncludeElement(const Filename &fn, Bool bIsDir, PresetElement* pElement)
{
	Int32 lFilter = pElement->arFilters.GetElementCount();
	FilterElement** ppFilters = pElement->arFilters.GetArray()->GetArray();
	Int32 a;

	if (bIsDir)
	{
		for (a = 0; a < lFilter; a++)
		{
			FilterElement* pFilter = ppFilters[a];
			if (!pFilter)
				continue;

			if (pFilter->lCondition == FILTER_COND_PATH_CONTAINS && pFilter->lAction == FILTER_ACTION_EXCLUDE)
			{
				Int32 lPos;
				String str = fn.GetString();
				str = str.ToLower();
				String strFilter = pFilter->str;
				g_pStringReplace->Replace(strFilter);
				strFilter = strFilter.ToLower();
				if (str.FindFirst(strFilter, &lPos))
					return nullptr;
			}
			if (pFilter->lCondition != FILTER_COND_DIRECTORY_IS && pFilter->lAction != FILTER_ACTION_FORCE_COPY)
				continue;

			String str = fn.GetFile().GetString();
			str = str.ToLower();
			String strFilter = pFilter->str;
			g_pStringReplace->Replace(strFilter);
			strFilter = strFilter.ToLower();
			if (str == strFilter)
				return pFilter;
		}
		return nullptr;
	}

	for (a = 0; a < lFilter; a++)
	{
		FilterElement* pFilter = ppFilters[a];
		if (!pFilter)
			continue;
		Bool bMatchFound = false;
		if (pFilter->str == "*")
		{
			if (pFilter->lCondition == FILTER_COND_PATH_CONTAINS || pFilter->lCondition == FILTER_COND_PATH_IS)
				bMatchFound = true;
		}
		else
		{
			switch (pFilter->lCondition)
			{
			case FILTER_COND_PATH_CONTAINS:
				{
					Int32 lPos;
					String str = fn.GetString();
					str = str.ToLower();
					String strFilter = pFilter->str;
					g_pStringReplace->Replace(strFilter);
					strFilter = strFilter.ToLower();
					if (str.FindFirst(strFilter, &lPos))
						bMatchFound = true;
				}
				break;
			case FILTER_COND_PATH_IS:
				{
					String str = fn.GetDirectory().GetString();
					str = str.ToLower();
					String strFilter = pFilter->str;
					g_pStringReplace->Replace(strFilter);
					strFilter = strFilter.ToLower();
					if (str == strFilter)
						bMatchFound = true;
				}
				break;
			}
		}
		if (bMatchFound)
		{
			//if (pFilter->lAction == FILTER_ACTION_EXCLUDE)
				//return true;
			/*if (bIsDir)
				return pFilter->lAction == FILTER_ACTION_INCLUDE;
			else*/
			{
				if (pFilter->lAction == FILTER_ACTION_INCLUDE || pFilter->lAction == FILTER_ACTION_RENAME)
					goto _check_files;
				else
					return nullptr;
			}
		}
	}

_check_files:
	for (a = 0; a < lFilter; a++)
	{
		FilterElement* pFilter = ppFilters[a];
		if (!pFilter)
			continue;
		Bool bMatchFound = false;
		if (pFilter->str == "*")
		{
			if (pFilter->lCondition == FILTER_COND_FN_IS || pFilter->lCondition == FILTER_COND_FN_EXT)
				bMatchFound = true;
		}
		else
		{
			switch (pFilter->lCondition)
			{
			case FILTER_COND_FN_IS:
				{
					String str = fn.GetFile().GetString();
					if (str.LexCompare(pFilter->str) == 0)
						bMatchFound = true;
				}
				break;
			case FILTER_COND_FN_EXT:
				{
					if (!bIsDir)
					{
						if (pFilter->str == "dylib")
						{
							// take dylib_stripped instead of dylib
							if (fn.CheckSuffix("dylib_stripped"))
								bMatchFound = true;
							else if (fn.CheckSuffix("dylib"))
							{
								// take the dylib if there is no dylib_stripped
								Filename f = Filename(fn.GetString() + "_stripped");
								if (!GeFExist(f))
									bMatchFound = true;
							}
						}
						else
						{
							if (fn.CheckSuffix(pFilter->str))
								bMatchFound = true;
						}
					}
				}
				break;
			}
		}
		if (bMatchFound)
			return (pFilter->lAction == FILTER_ACTION_INCLUDE || pFilter->lAction == FILTER_ACTION_RENAME ||
							pFilter->lAction == FILTER_ACTION_MOVE_TO || pFilter->lAction == FILTER_ACTION_COMPILE_INCL ||
							pFilter->lAction == FILTER_ACTION_COMPILE_INCL_KILL) ? pFilter : nullptr;
	}

	return nullptr;
}

String MakeDistriDialog::RenamePath(const String &str, PresetElement* pElement)
{
	String strRet = str;
	Int32 lFilter = pElement->arFilters.GetElementCount();
	FilterElement** ppFilters = pElement->arFilters.GetArray()->GetArray();
	Int32 a;
	for (a = 0; a < lFilter; a++)
	{
		FilterElement* pFilter = ppFilters[a];
		if (!pFilter)
			continue;
		if (pFilter->lCondition != FILTER_COND_PATH_CONTAINS || pFilter->lAction != FILTER_ACTION_RENAME)
			continue;

		String s = strRet.ToLower();
		String strSearch = pFilter->str.ToLower();
		String s1;
		Int32 lPos, lStart = 0;
		while (s.FindFirst(strSearch, &lPos, lStart))
		{
			// alles von lStart bis lPos kopieren
			s1 += strRet.SubStr(lStart, lPos - lStart);
			// String anhängen
			s1 += pFilter->strRename;
			lStart = lPos + pFilter->str.GetLength();
		}
		// Rest von lStart bis zum Ende kopieren
		s1 += strRet.SubStr(lStart, strRet.GetLength() - lStart);
		strRet = s1;
	}
	return strRet;
}

void MakeDistriDialog::WriteBuildInfo(const Filename fn, ZipFile* pZipFile)
{
	String str = m_strBuildVersion + GeGetLineEnd();
	Int32 lLen = str.GetCStringLen(STRINGENCODING_8BIT);
	Char* pStr = NewMem(Char,lLen + 2);
	if (!pStr)
		return;
	str.GetCString(pStr, lLen + 1, STRINGENCODING_8BIT);

	if (!pZipFile)
	{
		AutoAlloc <BaseFile> pFile;
		if (pFile)
		{
			ForceOpenFileWrite(pFile, fn);
			pFile->WriteBytes(pStr, lLen);
			pFile->Close();
		}
	}
	else
	{
		String strName;
		if (!ZipFile::CreateLocalFileName(fn, strName))
			return;

		DateTime dt;
		GetDateTimeNow(dt);

		ZipWriteInfo info;
		info.ti.nYear = dt.year;
		info.ti.nMonth = dt.month - 1;
		info.ti.nMDay = dt.day;
		info.ti.nHour = dt.hour;
		info.ti.nMin = dt.minute;
		info.ti.nSec = dt.second;
		info.lExternalAttr = 0;
		info.lInternalAttr = 0;
		pZipFile->CreateFileInZip(strName, &info, nullptr, 0, nullptr, 0, nullptr, ZipMethodStore, 0);
		pZipFile->WriteInFileInZip(pStr, lLen, lLen);
		pZipFile->CloseFileInZip();
	}
	DeleteMem(pStr);
}

Bool MakeDistriCommandLine(String presetname, const String &buildid)
{
	MakeDistriDialog dummydlg;
	Int32 i=0;

	// check if the preset name is a filename
	Filename fn(presetname);
	if (GeFExist(fn))
	{
		g_Presets.Free();
		ReadPreset(fn);

		fn = fn.GetFile();
		fn.ClearSuffix();
		presetname = fn.GetString();
	}

	for (i=0;i<g_Presets.GetElementCount();i++)
	{
		PresetElement *pe = g_Presets[i];
		if (!pe->strName.LexCompare(presetname))
		{
			String error;
			dummydlg.m_strBuildVersion = buildid;
			return dummydlg.MakeDistribution(pe, pe->strPassword, nullptr);
		}
	}
	GeConsoleOut("Error: "+presetname+" not found");
	return false;
}
