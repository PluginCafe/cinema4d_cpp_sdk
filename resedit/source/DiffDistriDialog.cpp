// DiffDistriDialog.cpp
//////////////////////////////////////////////////////////////////////

#include "c4d_commanddata.h"
#include "c4d_memory.h"
#include "c4d_resource.h"
#include "c4d_basedocument.h"
#include "DiffDistriDialog.h"
#include "lib_zipfile.h"
#include "c4d_symbols.h"
#include "globals.h"

#define DIFF_DISTRI_ID			450000221

DiffDistriDialog* g_pDistriDialog = nullptr;

Filename* GetFile(void* pRoot, void* pObj)
{
	CDynamicFilenameSet* pFiles = (CDynamicFilenameSet*)pRoot;
	if (!pFiles || !pObj)
		return nullptr;
	Int o = (Int)pObj;
	o--;
	return pFiles->GetObjectAt((Int32)o);
}

class FileListFunctions : public TreeViewFunctions
{
	void*		GetFirst(void *root, void *userdata)
	{
		CDynamicFilenameSet* pFiles = (CDynamicFilenameSet*)root;
		if (!pFiles || !pFiles->GetElementCount())
			return nullptr;
		return (void*)1;
	}

	void*		GetDown(void *root, void *userdata,void *obj)
	{
		return nullptr;
	}

	void*		GetNext(void *root, void *userdata,void *obj)
	{
		CDynamicFilenameSet* pFiles = (CDynamicFilenameSet*)root;
		if (!pFiles || !obj)
			return nullptr;
		Int o = (Int)obj;
		Int e = pFiles->GetElementCount();
		o++;
		return o <= e ? (void*)o : nullptr;
	}

	Bool		IsSelected(void *root,void *userdata,void *obj)
	{
		DiffDistriDialog* pDialog = (DiffDistriDialog*)userdata;
		Filename* pfn = GetFile(root, obj);
		return pfn ? *pfn == pDialog->m_fnSelection : false;
	}

	void		Select(void *root,void *userdata,void *obj,Int32 mode)
	{
		DiffDistriDialog* pDialog = (DiffDistriDialog*)userdata;
		Filename* pfn = GetFile(root, obj);
		if (!pfn)
			return;
		if (mode == SELECTION_ADD || mode == SELECTION_NEW)
			pDialog->m_fnSelection = *pfn;
		else
			pDialog->m_fnSelection = Filename();
	}

	Bool		IsOpened(void *root,void *userdata,void *obj)
	{
		return false;
	}

	String	GetName(void *root,void *userdata,void *obj)
	{
		Filename* pfn = GetFile(root, obj);
		return pfn ? pfn->GetString() : String();
	}

	Int		GetId(void *root,void *userdata,void *obj)
	{
		return 0;
	}

	Int32		GetDragType(void *root,void *userdata,void *obj)
	{
		return NOTOK;
	}

	void		DeletePressed(void *root, void *userdata)
	{
		DiffDistriDialog* pDialog = (DiffDistriDialog*)userdata;
		CDynamicFilenameSet* pFiles = (CDynamicFilenameSet*)root;
		if (pDialog->m_fnSelection.Content())
			pFiles->RemoveFilename(pDialog->m_fnSelection);
	}

	Bool		ContextMenuCall(void *root,void *userdata,void *obj, Int32 lColumn, Int32 lCommand)
	{
		CDynamicFilenameSet* pFiles = (CDynamicFilenameSet*)root;
		if (lCommand == ID_TREEVIEW_CONTEXT_RESET)
		{
			pFiles->Free();
			return true;
		}
		return false;
	}

} funcs;

Bool DiffZipFiles(const CDynamicFilenameSet &arFiles, const Filename &fnNew, const Filename &fnDestZip, const char* pchPassword)
{
	AutoAlloc <ZipFile> pOldZip, pNewZip, pDiffZip;
	ZipArray arStrings;
	ZipFileInfo info;
	String strName;
	Bool bExisted;
	Bool bOK = false;
	Int32 l, lCount;
	ZipArrayEntry* pEntry;
	void* pchData = nullptr;
	UInt32 lMaxDataLen = 0;
	ZipWriteInfo writeinfo;
	Bool bAppFolderChanged = false;

	if (!pOldZip || !pNewZip || !pDiffZip)
		return false;

	if (!pNewZip->Open(fnNew, true) || !pDiffZip->Open(fnDestZip, false))
		return false;

	// first, add all files of the new zip file into our array
	if (pNewZip->GoToFirstFile())
	{
		do
		{
			if (!pNewZip->GetCurrentFileInfo(info) ||
					!pNewZip->GetCurrentFileInfo(&strName))
				GOTO_ERROR;

			ZipArrayEntry* pEntry = arStrings.SearchObject(&strName);
			if (!pEntry)
			{
				pEntry = NewObjClear(ZipArrayEntry);
				if (!pEntry)
					GOTO_ERROR;
				pEntry->strName = strName;
				pEntry->bInNew = true;
				pEntry->bInOld = false;
				pEntry->ulCRCNew = info.lCRC32;
				pEntry->lNewSize = info.lUncompressedSize;
				pEntry->bChanged = false;
				pEntry->bInAppFolder = strName.FindFirst(".app/", nullptr);
				arStrings.InsertObject(pEntry, bExisted);
				DebugAssert(!bExisted);
			}
			else
			{
				// exists twice?!
				DebugAssert(false);
			}
		} while (pNewZip->GoToNextFile());
	}

	// browse through the array and check if something has changed
	for (l = 0; l < arFiles.GetElementCount(); l++)
	{
		if (!pOldZip->Open(*arFiles[l], true))
			GOTO_ERROR;
		if (pOldZip->GoToFirstFile())
		{
			do
			{
				if (!pOldZip->GetCurrentFileInfo(info) ||
						!pOldZip->GetCurrentFileInfo(&strName))
					GOTO_ERROR;

				ZipArrayEntry* pEntry = arStrings.SearchObject(&strName);
				if (!pEntry)
				{
					// file has been deleted
					pEntry = NewObjClear(ZipArrayEntry);
					if (!pEntry)
						GOTO_ERROR;
						pEntry->strName = strName;
					pEntry->bInNew = false;
					pEntry->bInOld = true;
					arStrings.InsertObject(pEntry, bExisted);
					DebugAssert(!bExisted);
				}
				else
				{
					// check for size and CRC
					pEntry->bInOld++;
					if ((pEntry->lNewSize != info.lUncompressedSize || pEntry->ulCRCNew != info.lCRC32) && pEntry->bInNew)
					{
						pEntry->bChanged = true;
						if (pEntry->bInAppFolder)
							bAppFolderChanged = true;
					}
				}
			} while (pOldZip->GoToNextFile());
		}
		pOldZip->Close();
	}

	lCount = arFiles.GetElementCount();
	for (l = 0; l < arStrings.GetElementCount(); l++)
	{
		if (arStrings[l]->bInNew)
			// we want to add it - mark as old if it exists in all old archives
			arStrings[l]->bInOld = (arStrings[l]->bInOld == lCount);
		else
			// old if it exists in at least one archive
			arStrings[l]->bInOld = (arStrings[l]->bInOld > 0);
	}

	lCount = arStrings.GetElementCount();
	if (!bAppFolderChanged)
	{
		// check if a new file as been added
		for (l = 0; l < lCount; l++)
		{
			pEntry = arStrings.GetObjectAt(l);
			if (!pEntry)
			{
				DebugAssert(false);
				continue;
			}
			if (pEntry->bInAppFolder && pEntry->bInNew && !pEntry->bInOld)
			{
				bAppFolderChanged = true;
				break;
			}
		}
	}

	for (l = 0; l < lCount; l++)
	{
		pEntry = arStrings.GetObjectAt(l);
		if (!pEntry)
		{
			DebugAssert(false);
			continue;
		}
		if (pEntry->bChanged || (pEntry->bInNew && !pEntry->bInOld) || (bAppFolderChanged && pEntry->bInAppFolder))
		{
			// the file has changed or was added, copy it from the new distri to the diff
			if (!pNewZip->LocateFile(pEntry->strName))
				GOTO_ERROR;
			if (!pNewZip->GetCurrentFileInfo(info))
				GOTO_ERROR;
			if (!pchData || lMaxDataLen < pEntry->lNewSize)
			{
				DeleteMem(pchData);
				pchData = NewMem(UChar, pEntry->lNewSize + 1);
				if (!pchData)
					GOTO_ERROR;
				lMaxDataLen = pEntry->lNewSize;
			}

			if (!pNewZip->OpenCurrentFile(pchPassword))
				GOTO_ERROR;

			if (pNewZip->ReadCurrentFile(pchData, pEntry->lNewSize) != pEntry->lNewSize)
				GOTO_ERROR;

			pNewZip->CloseCurrentFile();

			writeinfo.ti = info.t;
			if (pEntry->strName[pEntry->strName.GetLength() - 1] == '/')
			{
				writeinfo.lExternalAttr = ZIP_FLAG_DIRECTORY;
				writeinfo.lInternalAttr = 0;
				if (!pDiffZip->CreateFileInZip(pEntry->strName, &writeinfo, nullptr, 0, nullptr, 0, nullptr, ZipMethodStore, 0, pchPassword, 0))
					GOTO_ERROR;
				if (!pDiffZip->CloseFileInZip())
					GOTO_ERROR;
			}
			else
			{
				writeinfo.lExternalAttr = info.lExternalAttr;
				writeinfo.lInternalAttr = info.lInternalAttr;
				if (!pDiffZip->CreateFileInZip(pEntry->strName, &writeinfo, nullptr, 0, nullptr, 0, nullptr, ZipMethodDeflate, 9, nullptr, 0))
					GOTO_ERROR;
				if (!pDiffZip->WriteInFileInZip(pchData, pEntry->lNewSize))
					GOTO_ERROR;
				if (!pDiffZip->CloseFileInZip())
					GOTO_ERROR;
			}
		}
		else if (!pEntry->bInNew && pEntry->bInOld)
		{
			if (pEntry->strName[pEntry->strName.GetLength() - 1] == '/')
			{
				writeinfo.lExternalAttr = ZIP_FLAG_DIRECTORY;
				writeinfo.lInternalAttr = 0;
				if (!pDiffZip->CreateFileInZip("----" + pEntry->strName, &writeinfo, nullptr, 0, nullptr, 0, nullptr, ZipMethodStore, 0, pchPassword, 0))
					GOTO_ERROR;
				if (!pDiffZip->CloseFileInZip())
					GOTO_ERROR;
			}
			else
			{
				writeinfo.lExternalAttr = 0;
				writeinfo.lInternalAttr = 0;
				if (!pDiffZip->CreateFileInZip("----" + pEntry->strName, &writeinfo, nullptr, 0, nullptr, 0, nullptr, ZipMethodStore, 9, nullptr, 0))
					GOTO_ERROR;
				if (!pDiffZip->CloseFileInZip())
					GOTO_ERROR;
			}
		}
	}

	bOK = true;
error:
	pOldZip->Close();
	pNewZip->Close();
	pDiffZip->Close();

	arStrings.Free();
	DeleteMem(pchData);
	return bOK;
}

DiffDistriDialog::DiffDistriDialog()
{
}

DiffDistriDialog::~DiffDistriDialog()
{
}

Bool DiffDistriDialog::CreateLayout(void)
{
	if (!GeDialog::CreateLayout())
		return false;
	if (!LoadDialogResource(IDD_DIFF_DISTRI_DLG, nullptr, 0))
		return false;

	m_pTreeView = (TreeViewCustomGui*)FindCustomGui(IDC_DIFF_DISTRI_FILES_TREE, CUSTOMGUI_TREEVIEW);
	m_pNewZipEdit = (FilenameCustomGui*)FindCustomGui(IDC_DIFF_DISTRI_NEWZIP_EDIT, CUSTOMGUI_FILENAME);
	m_pDestPathEdit = (FilenameCustomGui*)FindCustomGui(IDC_DIFF_DISTRI_DESTPATH_EDIT, CUSTOMGUI_FILENAME);
	if (!m_pTreeView || !m_pNewZipEdit || !m_pDestPathEdit)
		return false;

	BaseContainer bcLayout;
	bcLayout.SetInt32(0, LV_TREE);
	m_pTreeView->SetLayout(1, bcLayout);
	m_pTreeView->SetRoot(&g_pResEditPrefs->arOldFiles, &funcs, this);

	return true;
}

Bool DiffDistriDialog::InitValues(void)
{
	SetString(IDC_DIFF_DISTRI_PASSWORD_EDIT, String(""), 0, EDITTEXT_PASSWORD);

	m_pNewZipEdit->SetData(TriState<GeData>(GeData(g_pResEditPrefs->fnNewZip)));
	m_pDestPathEdit->SetData(TriState<GeData>(GeData(g_pResEditPrefs->fnDestZip)));

	return true;
}

Bool DiffDistriDialog::Command(Int32 id, const BaseContainer &msg)
{
	switch (id)
	{
	case IDC_DIFF_DISTRI_NEW_BTN:
		{
			Filename fn;
			if (fn.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_LOAD, GeLoadString(IDS_DIFF_DISTRI_CHOOSE)))
			{
				if (!fn.CheckSuffix("zip"))
				{
					GeOutString(GeLoadString(IDS_DIFF_DISTRI_INVALID_SUFFIX), GEMB_OK | GEMB_ICONSTOP);
				}
				else
				{
					g_pResEditPrefs->arOldFiles.AddFilename(fn);
					m_pTreeView->Refresh();
				}
			}
		}
		break;

	case IDC_DIFF_DISTRI_REMOVE_BTN:
		if (m_fnSelection.Content())
		{
			g_pResEditPrefs->arOldFiles.RemoveFilename(m_fnSelection);
			m_fnSelection = Filename();
			m_pTreeView->Refresh();
		}
		break;

	case IDC_DIFF_DISTRI_NEWZIP_EDIT:
		g_pResEditPrefs->fnNewZip = m_pNewZipEdit->GetData().GetValue().GetFilename();
		break;

	case IDC_DIFF_DISTRI_DESTPATH_EDIT:
		{
			Filename fnDestZip = m_pDestPathEdit->GetData().GetValue().GetFilename();
			g_pResEditPrefs->fnDestZip = fnDestZip;
			g_pResEditPrefs->fnDestZip.SetSuffix("zip");
			if (fnDestZip.Content() && g_pResEditPrefs->fnDestZip != fnDestZip)
				m_pDestPathEdit->SetData(TriState<GeData>(g_pResEditPrefs->fnDestZip));
		}
		break;

	case IDC_OK:
		{
			GeData d;
			String strPassword;
			Int32 lLen = 0;
			char* pchPassword = nullptr;

			Filename fnDestZip = m_pDestPathEdit->GetData().GetValue().GetFilename();
			g_pResEditPrefs->fnNewZip = m_pNewZipEdit->GetData().GetValue().GetFilename();
			g_pResEditPrefs->fnDestZip = fnDestZip;
			g_pResEditPrefs->fnDestZip.SetSuffix("zip");
			if (fnDestZip.Content() && g_pResEditPrefs->fnDestZip != fnDestZip)
				m_pDestPathEdit->SetData(TriState<GeData>(g_pResEditPrefs->fnDestZip));
			GetString(IDC_DIFF_DISTRI_PASSWORD_EDIT, strPassword);

			lLen = strPassword.GetCStringLen(STRINGENCODING_8BIT);
			if (lLen)
			{
				pchPassword = NewMemClear(Char,lLen + 2);
				if (pchPassword)
					strPassword.GetCString(pchPassword, lLen + 1, STRINGENCODING_8BIT);
			}

			if (!g_pResEditPrefs->arOldFiles.GetElementCount())
			{
				GeOutString(GeLoadString(IDS_DIFF_DISTRI_NO_OLD), GEMB_ICONSTOP);
				DeleteMem(pchPassword);
				break;
			}
			if (!g_pResEditPrefs->fnNewZip.Content())
			{
				GeOutString(GeLoadString(IDS_DIFF_DISTRI_NO_NEW), GEMB_ICONSTOP);
				DeleteMem(pchPassword);
				break;
			}
			if (!g_pResEditPrefs->fnDestZip.Content())
			{
				GeOutString(GeLoadString(IDS_DIFF_DISTRI_NO_DEST), GEMB_ICONSTOP);
				DeleteMem(pchPassword);
				break;
			}

			if (!DiffZipFiles(g_pResEditPrefs->arOldFiles, g_pResEditPrefs->fnNewZip, g_pResEditPrefs->fnDestZip, pchPassword))
				GeOutString(GeLoadString(IDS_DIFF_DISTRI_FAILED, g_pResEditPrefs->fnNewZip.GetString(), g_pResEditPrefs->fnDestZip.GetString()), GEMB_OK | GEMB_ICONEXCLAMATION);
			DeleteMem(pchPassword);
		}
		break;

	case IDC_CANCEL:
		Close();
		break;

	default:
		break;
	}

	return true;
}

class DiffDistriCommand : public CommandData
{
	Bool Execute(BaseDocument *doc)
	{
		if (!g_pDistriDialog)
			g_pDistriDialog = NewObjClear(DiffDistriDialog);
		if (!g_pDistriDialog)
			return false;
		g_pDistriDialog->Open(DLG_TYPE_ASYNC, DIFF_DISTRI_ID, -1, -1, 400, 300);
		return true;
	}

	Bool RestoreLayout(void *secret)
	{
		if (!g_pDistriDialog)
			g_pDistriDialog = NewObjClear(DiffDistriDialog);
		if (!g_pDistriDialog)
			return false;
		g_pDistriDialog->RestoreLayout(DIFF_DISTRI_ID, 0, secret);
		return true;
	}
};

Bool RegisterDiffZipCommand()
{
	RegisterCommandPlugin(DIFF_DISTRI_ID, GeLoadString(IDS_DIFF_DISTRI), 0, nullptr, GeLoadString(IDS_DIFF_DISTRI), NewObjClear(DiffDistriCommand));
	return true;
}

void FreeDiffZipCommand()
{
	if (g_pDistriDialog)
		DeleteObj(g_pDistriDialog);
}

String OldFormatString(const char* pChFormat, ...)
{
	va_list arp;
	va_start(arp, pChFormat);
	char pch[1000];
#ifdef __PC
	_vsnprintf(pch, 1000, pChFormat, arp);
#else
	vsnprintf(pch, 1000, pChFormat, arp);
#endif
	va_end(arp);
	return pch;
}
