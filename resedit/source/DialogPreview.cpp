/*********************************************************************\
	File name        : DialogPreview.cpp
	Description      : Implementation of the class CDialogPreview
	Created at       : 11.08.01, @ 10:25:38
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "DialogPreview.h"
#include "DialogDoc.h"
#include "DialogItem.h"
#include "TreeDialog.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

#define LINKBOX_ACCEPT_DRAG_OBJECT      'lado' // sent to the parent dialog to ask if the control should accept the object

enum {
	IDC_PREVIEW_SUB_DLG = 1000,
	IDC_FIRST_SUB_CONTROL,

	IDC_DUMMY
};

CPreviewSub::CPreviewSub()
{
}

/*********************************************************************\
	Function name    : CPreviewSub::SetDialog
	Description      :
	Created at       : 11.08.01, @ 21:20:22
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CPreviewSub::SetDialog(CDialogPreview* pDlg, CDialogTemplate* pTempl)
{
	m_pDialog = pDlg;
	m_pTempl = pTempl;
}

/*********************************************************************\
	Function name    : CPreviewSub::CreateLayout
	Description      :
	Created at       : 11.08.01, @ 21:03:41
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CPreviewSub::CreateLayout(void)
{
	Bool bRes = SubDialog::CreateLayout();
	Int32 lID = IDC_FIRST_SUB_CONTROL;
	m_pDialog->m_pDocument->CreateElements(lID, this);

	return bRes;
}

/*********************************************************************\
	Function name    : CPreviewSub::InitValues
	Description      :
	Created at       : 27.04.02, @ 16:42:38
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CPreviewSub::InitValues()
{
	m_pDialog->m_pDocument->InitValues(nullptr);

	return true;
}

/*********************************************************************\
	Function name    : CPreviewSub::Command
	Description      :
	Created at       : 11.08.01, @ 21:30:06
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CPreviewSub::Command(Int32 lId, const BaseContainer &msg)
{
	CDialogDoc* pDoc = m_pDialog->m_pDocument;

	CDialogItem* pItem = pDoc->SearchItem(pDoc->m_pTreeDlg->GetTreeView()->GetRootItem(), lId);
	if (pItem)
	{
		if (pItem->GetType() != Group)
			pDoc->SelChanged(pItem);
		if (pItem->GetType() == CustomElement)
		{
			CDialogCustomElement* pCustom = (CDialogCustomElement*)pItem;
			Int32 l = pCustom->GetElementPluginID();
			if (l >= 0)
			{
				_BaseCustomGui* pGUI = (_BaseCustomGui*)FindCustomGui(lId, l);
				if (pGUI)
				{
					if (pCustom->m_lElement >= 0 && pCustom->m_lElement < g_pCustomElements->Entries() && pCustom->m_pData)
					{
						pCustom->m_pData[pCustom->m_lElement] = pGUI->GetData(l);
					}
				}
			}
		}
	}
	return false;
}

Int32 CPreviewSub::Message(const BaseContainer &msg, BaseContainer &result)
{
	/*if (msg.GetId() == MSG_DESCRIPTION_CHECKDRAGANDDROP)
	{
		Int32 item_type = msg.GetInt32(LINKBOX_ACCEPT_MESSAGE_TYPE);
		BaseList2D* obj = (BaseList2D*)msg.GetVoid(LINKBOX_ACCEPT_MESSAGE_ELEMENT);
		Bool *accept = (Bool*)msg.GetVoid(LINKBOX_ACCEPT_MESSAGE_ACCEPT);
		Int32 linkbox_id = msg.GetInt32(LINKBOX_ACCEPT_MESSAGE_CONTROL_ID);
		linkbox_id = linkbox_id;
	}*/
	return SubDialog::Message(msg, result);
}


CDialogPreview::CDialogPreview(CDialogDoc* pDoc)
{
	m_pDocument = pDoc;
	m_pDocument->m_pDlgPreview = this;
	m_bCloseState = false;
}

CDialogPreview::~CDialogPreview()
{
}

/*********************************************************************\
	Funktionsname    : CDialogPreview::CreateLayout
	Beschreibung     :
	Rückgabewert     : Bool
	Erstellt am      : 11.08.01, @ 10:29:05
	Argument         : void
\*********************************************************************/
Bool CDialogPreview::CreateLayout(void)
{
	Bool bRes = GeDialog::CreateLayout();

	if (Get()) C4DOS.Cd->AddGadget(Get(), DIALOG_SUBDIALOG, IDC_PREVIEW_SUB_DLG, nullptr, BFH_SCALEFIT | BFV_SCALEFIT, 0, 0, 0, nullptr, nullptr);

	if (++g_lNeedFileNew == 4) m_pDocument->OnFileNew();

	return bRes;
}

/*********************************************************************\
	Funktionsname    : CDialogPreview::Command
	Beschreibung     :
	Rückgabewert     : Bool
	Erstellt am      : 11.08.01, @ 10:44:06
	Argumente        : Int32 nId
										 const BaseContainer &msg
\*********************************************************************/
Bool CDialogPreview::Command(Int32 lId, const BaseContainer &msg)
{
	return true;
}


Int32 CDialogPreview::Message(const BaseContainer &msg, BaseContainer &result)
{
	if (msg.GetId() == LINKBOX_ACCEPT_DRAG_OBJECT)
	{
		DebugAssert(false);
	}
	return GeDialog::Message(msg, result);
}


/*********************************************************************\
	Function name    : CDialogPreview::Update
	Description      :
	Created at       : 11.08.01, @ 15:09:59
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogPreview::Update()
{
	CDialogTemplate* pTempl = m_pDocument->GetDialog();
	if (!pTempl) return;

	m_wndSubDlg.SetDialog(this, pTempl);

	AttachSubDialog(&m_wndSubDlg, IDC_PREVIEW_SUB_DLG);
	LayoutChanged(IDC_PREVIEW_SUB_DLG);

	String strFile = m_pDocument->UpdateDialogCaptions();

	String strCaption = pTempl->m_strName;
	strCaption += "  [";
	strCaption += GeLoadString(IDS_RES_EDIT_PREVIEW_CAPT) + " - " + strFile;
	strCaption += "]";
	SetTitle(strCaption);
}

/*********************************************************************\
	Function name    : CDialogPreview::MakeVisible
	Description      :
	Created at       : 07.09.01, @ 16:14:42
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogPreview::MakeVisible(CDialogItem* pItem)
{
	TreeViewItem* pTreeItem = pItem->m_pTreeViewItem;
	Int32 lLastID = pItem->m_lID;
	CDialogItem* pSearchItem;

	while (pTreeItem)
	{
		pSearchItem = (CDialogItem*)pTreeItem->GetData()->GetVoid(TREEVIEW_DATA);
		if (pSearchItem)
		{
			if (((pSearchItem->GetType()) == Group) && (pSearchItem != pItem))
			{
				CDialogGroup* pGroup = (CDialogGroup*)pSearchItem;
				if (pGroup->m_lGroupType == GROUP_TYPE_TAB)
				{
					pGroup->m_lLastChildID = lLastID;
					m_wndSubDlg.SetInt32(pGroup->m_lID, lLastID);
				}
			}
			lLastID = pSearchItem->m_lID;
		}
		pTreeItem = pTreeItem->GetParent();
	};
}

/*********************************************************************\
	Function name    : CDialogPreview::AskClose
	Description      :
	Created at       : 11.10.01, @ 21:39:11
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogPreview::AskClose()
{
	if (m_bCloseState) return false;

	m_bCloseState = true;
	if (!m_pDocument->CloseEditor()) return true;

	return false;
}
