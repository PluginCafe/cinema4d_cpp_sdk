// ExtractZipDialog.cpp
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "globals.h"
#include "c4d_gui.h"
#include "lib_zipfile.h"
#include "ExtractZipDialog.h"
#include "DistriCompressProgr.h"
#include <stdio.h>

#define COL_FILENAME    'cofn'
#define COL_DATE        'coda'
#define COL_TIME        'coti'
#define COL_ORIG_SIZE   'coos'
#define COL_NEW_SIZE    'cons'
#define COL_RATIO       'cora'
#define COL_CRC32       'ccrc'
#define COL_FLAGS       'cfla'

void SetModalResizeFlag(void* pHandle);

class ZipTreeFunctions : public TreeViewFunctions
{
public:
	void*		GetFirst(void *root,void *userdata)
	{
		CDynamicObjectArray<ZipElement>* pList = (CDynamicObjectArray<ZipElement>*)root;
		if (!root)
			return nullptr;
		return pList->GetObjectAt(0);
	}

	void*		GetDown(void *root,void *userdata,void *obj)
	{
		return nullptr;
	}

	void*		GetNext(void *root,void *userdata,void *obj)
	{
		CDynamicObjectArray<ZipElement>* pList = (CDynamicObjectArray<ZipElement>*)root;
		ZipElement* pItem = (ZipElement*)obj;
		if (!root || !pItem)
			return nullptr;
		return pList->GetObjectAt(pItem->lIndex + 1);
	}

	Bool		IsSelected(void *root,void *userdata,void *obj)
	{
		return false;
	}

	Bool		IsOpened(void *root,void *userdata,void *obj)
	{
		return false;
	}

	String	GetName(void *root,void *userdata,void *obj)
	{
		ZipElement* pItem = (ZipElement*)obj;
		if (!root || !pItem)
			return String();
		return pItem->strName;
	}

	Int		GetId(void *root,void *userdata,void *obj)
	{
		return 0;
	}

	Int32		GetDragType(void *root,void *userdata,void *obj)
	{
		return NOTOK;
	}

	void		DrawCell(void *root,void *userdata,void *obj,Int32 col,DrawInfo *drawinfo, const GeData& bgColor)
	{
		ZipElement* pItem = (ZipElement*)obj;
		if (!pItem)
			return;
		Int32 lYOffset = drawinfo->ypos + (drawinfo->height - drawinfo->frame->DrawGetFontHeight()) / 2;
		drawinfo->frame->DrawSetTextCol(COLOR_TEXT, COLOR_TRANS);
		if (col == COL_ORIG_SIZE)
			drawinfo->frame->DrawText(pItem->strOrigSize, drawinfo->xpos + LMax(0, drawinfo->width - drawinfo->frame->DrawGetTextWidth(pItem->strOrigSize)), lYOffset);
		else if (col == COL_NEW_SIZE)
			drawinfo->frame->DrawText(pItem->strNewSize, drawinfo->xpos + LMax(0, drawinfo->width - drawinfo->frame->DrawGetTextWidth(pItem->strNewSize)), lYOffset);
		else if (col == COL_DATE)
			drawinfo->frame->DrawText(pItem->strDate, drawinfo->xpos + LMax(0, drawinfo->width - drawinfo->frame->DrawGetTextWidth(pItem->strDate)), lYOffset);
		else if (col == COL_TIME)
			drawinfo->frame->DrawText(pItem->strTime, drawinfo->xpos + LMax(0, drawinfo->width - drawinfo->frame->DrawGetTextWidth(pItem->strTime)), lYOffset);
		else if (col == COL_RATIO)
			drawinfo->frame->DrawText(pItem->strRatio, drawinfo->xpos + LMax(0, drawinfo->width - drawinfo->frame->DrawGetTextWidth(pItem->strRatio)), lYOffset);
		else if (col == COL_CRC32)
			drawinfo->frame->DrawText(pItem->strCRC32, drawinfo->xpos + LMax(0, drawinfo->width - drawinfo->frame->DrawGetTextWidth(pItem->strCRC32)), lYOffset);
		else if (col == COL_FLAGS)
			drawinfo->frame->DrawText(pItem->strFlags, drawinfo->xpos, lYOffset);
	}

	Int32    GetColumnWidth(void *root,void *userdata,void *obj,Int32 col, GeUserArea* area)
	{
		ZipElement* pItem = (ZipElement*)obj;
		if (!pItem)
			return 10;
		if (col == COL_ORIG_SIZE)
			return area->DrawGetTextWidth(pItem->strOrigSize);
		else if (col == COL_NEW_SIZE)
			return area->DrawGetTextWidth(pItem->strNewSize);
		else if (col == COL_DATE)
			return area->DrawGetTextWidth(pItem->strDate);
		else if (col == COL_TIME)
			return area->DrawGetTextWidth(pItem->strTime);
		else if (col == COL_RATIO)
			return area->DrawGetTextWidth(pItem->strRatio);
		else if (col == COL_CRC32)
			return area->DrawGetTextWidth(pItem->strCRC32);
		else if (col == COL_FLAGS)
			return area->DrawGetTextWidth(pItem->strFlags);
		return 0;
	}
};

ZipTreeFunctions zipTreeFuncs;

CExtractZipDialog::CExtractZipDialog(const Filename& fn)
{
	m_fnZip = fn;
	m_pchPassword = nullptr;
}

CExtractZipDialog::~CExtractZipDialog()
{
	m_zipElements.Free();
	DeleteMem(m_pchPassword);
}

Bool CExtractZipDialog::CreateLayout()
{
	if (!m_pZipFile)
		return false;

	if (!GeModalDialog::CreateLayout() ||
					!LoadDialogResource(IDC_EXTRACT_ZIP_FILE, nullptr, 0))
		return false;

	m_pTreeView = (TreeViewCustomGui*)FindCustomGui(IDC_EXTRACT_FILES_TREE, CUSTOMGUI_TREEVIEW);
	m_pFilenameGUI = (FilenameCustomGui*)FindCustomGui(IDC_EXTRACT_DEST_PATH, CUSTOMGUI_FILENAME);
	if (!m_pTreeView || !m_pFilenameGUI)
		return false;

	BaseContainer bcLayout;
	bcLayout.SetInt32(COL_FILENAME, LV_TREE);
	bcLayout.SetInt32(COL_DATE, LV_USER);
	bcLayout.SetInt32(COL_TIME, LV_USER);
	bcLayout.SetInt32(COL_ORIG_SIZE, LV_USER);
	bcLayout.SetInt32(COL_NEW_SIZE, LV_USER);
	bcLayout.SetInt32(COL_RATIO, LV_USER);
	bcLayout.SetInt32(COL_CRC32, LV_USER);
	bcLayout.SetInt32(COL_FLAGS, LV_USER);
	m_pTreeView->SetLayout(8, bcLayout);
	m_pTreeView->SetRoot(&m_zipElements, &zipTreeFuncs, nullptr);

	m_pTreeView->SetHeaderText(COL_FILENAME, GeLoadString(IDS_EXTRACT_ZIP_COL_FILENAME));
	m_pTreeView->SetHeaderText(COL_DATE, GeLoadString(IDS_EXTRACT_ZIP_COL_DATE));
	m_pTreeView->SetHeaderText(COL_TIME, GeLoadString(IDS_EXTRACT_ZIP_COL_TIME));
	m_pTreeView->SetHeaderText(COL_ORIG_SIZE, GeLoadString(IDS_EXTRACT_ZIP_COL_ORIG_SIZE));
	m_pTreeView->SetHeaderText(COL_NEW_SIZE, GeLoadString(IDS_EXTRACT_ZIP_COL_NEW_SIZE));
	m_pTreeView->SetHeaderText(COL_RATIO, GeLoadString(IDS_EXTRACT_ZIP_COL_RATIO));
	m_pTreeView->SetHeaderText(COL_CRC32, GeLoadString(IDS_EXTRACT_ZIP_COL_CRC32));
	m_pTreeView->SetHeaderText(COL_FLAGS, GeLoadString(IDS_EXTRACT_ZIP_COL_FLAGS));

	SetModalResizeFlag(GetWindowHandle());

	SetString(IDC_EXTRACT_PASSWORD_EDIT, String(""), 0, EDITTEXT_PASSWORD);

	return true;
}

Bool CExtractZipDialog::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
	case IDC_EXTRACT_CHANGE_FILE_BTN:
		{
			Filename fn;
			if (fn.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_LOAD, String()))
			{
				m_zipElements.Free();
				m_fnZip = fn;
				InitValues();
			}
		}
		break;
	case IDC_EXTRACT_DEST_PATH:
		g_pResEditPrefs->strExtractPath = m_pFilenameGUI->GetData().GetValue().GetFilename().GetString();
		g_pStringReplace->Replace(g_pResEditPrefs->strExtractPath);
		break;
	case IDC_EXTRACT_EXTRACT_BTN:
		{
			String strPassword;
			GetString(IDC_EXTRACT_PASSWORD_EDIT, strPassword);
			ExtractFile(strPassword);
		}
		break;
	case IDC_OK_BTN:
		Close(true);
		break;
	default:
		break;
	}
	return true;
}

Bool CExtractZipDialog::InitValues()
{
	if (!GeModalDialog::InitValues()) return false;

	SetString(IDC_EXTRACT_ZIP_FILE_EDIT, m_fnZip.GetString());
	m_pFilenameGUI->SetData(TriState<GeData>(Filename(g_pResEditPrefs->strExtractPath)));

	Bool bOK = true;
	ZipFileGlobalInfo info;
	ZipFileInfo zipInfo;
	UInt32 lEntries = 0;
	ZipElement* pNew;
	String strName;

	do
	{
		bOK &= m_pZipFile->Open(m_fnZip, true);
		if (!bOK) break;
		bOK &= m_pZipFile->GetGlobalInfo(info);
		if (!bOK) break;

		m_zipElements.Free();
		m_zipElements.GetArray()->SetMinSizeNoCopy(info.lEntries + 20, true);

		if (m_pZipFile->GoToFirstFile())
		{
			do
			{
				if (!m_pZipFile->GetCurrentFileInfo(zipInfo))
					break;

				if (!m_pZipFile->GetCurrentFileInfo(&strName))
					break;

				pNew = m_zipElements.GetNextObject();
				if (!pNew) break;
				pNew->lIndex = lEntries;
				pNew->strName = strName;

				if (zipInfo.lUncompressedSize < 1024)
					pNew->strOrigSize = String::UIntToString(zipInfo.lUncompressedSize) + " B";
				else if (zipInfo.lUncompressedSize < 1024 * 1024)
					pNew->strOrigSize = String::FloatToString((Float)zipInfo.lUncompressedSize / 1024.0f + .5f, -1, 0) + " KB";
				else
					pNew->strOrigSize = String::FloatToString((Float)zipInfo.lUncompressedSize / 1024.0f / 1024.0f + .5f, -1, 0) + " MB";

				if (zipInfo.lCompressedSize < 1024)
					pNew->strNewSize = String::UIntToString(zipInfo.lCompressedSize) + " B";
				else if (zipInfo.lUncompressedSize < 1024 * 1024)
					pNew->strNewSize = String::FloatToString((Float)zipInfo.lCompressedSize / 1024.0f + .5f, -1, 0) + " KB";
				else
					pNew->strNewSize = String::FloatToString((Float)zipInfo.lCompressedSize / 1024.0f / 1024.0f + .5f, -1, 0) + " MB";

				pNew->strDate = String::UIntToString(zipInfo.t.nMonth + 1) + "-" + String::UIntToString(zipInfo.t.nMDay) + "-" + String::UIntToString(zipInfo.t.nYear);
				pNew->strTime = String::UIntToString(zipInfo.t.nHour) + ":" + String::FloatToString((Float)zipInfo.t.nMin, 2, 0) + ":" + String::FloatToString((Float)zipInfo.t.nSec, 2, 0);
				pNew->strRatio = (zipInfo.lUncompressedSize != 0 ? String::FloatToString(100.0f - (Float)zipInfo.lCompressedSize / (Float)zipInfo.lUncompressedSize * 100.0f, 0, 2) + "%" : "");
				pNew->strCRC32 = OldFormatString("%08X", zipInfo.lCRC32);
				pNew->strFlags += (zipInfo.lExternalAttr & ZIP_FILE_FLAG_OWNER_R) ? "r" : "-";
				pNew->strFlags += (zipInfo.lExternalAttr & ZIP_FILE_FLAG_OWNER_W) ? "w" : "-";
				pNew->strFlags += (zipInfo.lExternalAttr & ZIP_FILE_FLAG_OWNER_X) ? "x" : "-";
				pNew->strFlags += (zipInfo.lExternalAttr & ZIP_FILE_FLAG_GROUP_R) ? "r" : "-";
				pNew->strFlags += (zipInfo.lExternalAttr & ZIP_FILE_FLAG_GROUP_W) ? "w" : "-";
				pNew->strFlags += (zipInfo.lExternalAttr & ZIP_FILE_FLAG_GROUP_X) ? "x" : "-";
				pNew->strFlags += (zipInfo.lExternalAttr & ZIP_FILE_FLAG_PUBLIC_R) ? "r" : "-";
				pNew->strFlags += (zipInfo.lExternalAttr & ZIP_FILE_FLAG_PUBLIC_W) ? "w" : "-";
				pNew->strFlags += (zipInfo.lExternalAttr & ZIP_FILE_FLAG_PUBLIC_X) ? "x" : "-";
				if (zipInfo.lExternalAttr & ZIP_FLAG_READONLY)
					pNew->strFlags += " (r)";

				lEntries++;
			} while(m_pZipFile->GoToNextFile());
		}
		DebugAssert(lEntries == info.lEntries);
	} while (0);
	m_pZipFile->Close();

	m_pTreeView->Refresh();

	return true;
}

Int32 CExtractZipDialog::Message(const BaseContainer &msg, BaseContainer &result)
{
/*  switch (msg.GetId())
	{
	default:
		break;
	}*/
	return GeModalDialog::Message(msg, result);
}

Bool CExtractZipDialog::AskClose()
{
	// return true, if the dialog should not be closed
	return false;
}


class ExtractThread : public C4DThread
{
public:
	ExtractThread() { m_bReady = false; m_bCancel = false; m_bError = false; m_lOverwrite = GEMB_R_UNDEFINED; m_pchPassword = nullptr; }
	CExtractZipDialog* m_pDlg;
	Bool m_bReady, m_bCancel, m_bError;
	CCompressProgressDlg* m_pProgressDlg;
	Filename m_fnZip, m_fnPath;
	char* m_pchPassword;
	GEMB_R m_lOverwrite;

	void Main(void)
	{
		while (!m_bReady) { GeSleep(30); }
		m_pDlg->ExtractFileThread(this);
		m_pProgressDlg->CloseDialog();
	}
	virtual const Char *GetThreadName(void) { return "ExtractThread"; }
};

void CExtractZipDialog::ExtractFile(const String &strPassword)
{
	Filename fnPath = m_pFilenameGUI->GetData().GetValue().GetFilename();
	if (!AssertPathExists(fnPath))
	{
		GeOutString(GeLoadString(IDS_DIRECTORY_NOT_EXIST), GEMB_ICONASTERISK | GEMB_OK);
		return;
	}

	if (m_pchPassword)
		DeleteMem(m_pchPassword);

	if (strPassword.Content())
	{
		Int32 lLen = strPassword.GetCStringLen(STRINGENCODING_8BIT);
		m_pchPassword = NewMemClear(Char,lLen + 2);
		if (!m_pchPassword)
			return;
		strPassword.GetCString(m_pchPassword, lLen + 1, STRINGENCODING_8BIT);
	}

	ExtractThread th;
	CCompressProgressDlg dlg(&th.m_bReady, &th.m_bCancel);
	th.m_pDlg = this;
	th.m_fnZip = m_fnZip;
	th.m_fnPath = fnPath;
	th.m_pProgressDlg = &dlg;
	th.m_pchPassword = m_pchPassword;

	if (!th.Start())
		CriticalStop();

	dlg.Open(-2, -2, 600);

	if (th.m_bCancel)
		GeOutString(GeLoadString(IDS_EXTRACT_CACNEL), GEMB_ICONASTERISK);
	else
	{
		if (th.m_bError)
			GeOutString(GeLoadString(IDS_EXTRACT_ERROR), GEMB_ICONASTERISK);
		else
			GeOutString(GeLoadString(IDS_EXTRACT_SUCCESS), GEMB_ICONASTERISK);
	}
}

Int32 ZipExtractDirectoryCallback(const Filename &fnSrc, const Filename &fnDest, void* pData, Int32 lInfo)
{
	ExtractThread* pThread = (ExtractThread*)pData;
	if (pThread->m_bCancel)
		return EXTRACT_INFO_R_CANCEL;

	if (lInfo == EXTRACT_INFO_SUCCESS)
		pThread->m_pProgressDlg->SetStrings(fnSrc.GetString(), fnDest.GetString());
	if (lInfo == EXTRACT_INFO_FAILED)
	{
		GeOutString(fnSrc.GetString() + " " + GeLoadString(IDS_FILECOPY_ERROR) + " " + fnDest.GetString() + ".", GEMB_ICONSTOP);
		pThread->m_bError = true;
		return EXTRACT_INFO_R_CANCEL;
	}
	if (lInfo == EXTRACT_INFO_CANT_WRITE)
	{
		if (pThread->m_lOverwrite == GEMB_R_RETRY || (pThread->m_lOverwrite = GeOutString(GeLoadString(IDS_EXTRACT_CANT_COPY, fnSrc.GetString()), GEMB_ICONEXCLAMATION | GEMB_ABORTRETRYIGNORE)) == GEMB_R_RETRY)
			return EXTRACT_INFO_CANT_WRITE_R_FORCE_OVERWRITE;
		if (pThread->m_lOverwrite == GEMB_R_IGNORE)
			return EXTRACT_INFO_CANT_WRITE_R_IGNORE;
		pThread->m_bError = true;
		return EXTRACT_INFO_R_CANCEL;
	}

	return 0;
}

void CExtractZipDialog::ExtractFileThread(C4DThread* pThread)
{
	ExtractThread* pEThread = (ExtractThread*)pThread;
	pEThread->m_lOverwrite = GEMB_R_UNDEFINED;
	m_pZipFile->ExtractToDirectory(pEThread->m_fnZip, pEThread->m_fnPath, ZIP_EXTRACT_CREATE_SUBDIR, ZipExtractDirectoryCallback, pEThread, pEThread->m_pchPassword);
}
