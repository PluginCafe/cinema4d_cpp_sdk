// MakeDistriDialog.h :  Interface definition of class MakeDistriDialog
//
//////////////////////////////////////////////////////////////////////

#if !defined MAKEDISTRIDIALOG_H_3fbceeee
#define MAKEDISTRIDIALOG_H_3fbceeee

#define FILTER_COND_FN_IS               0
#define FILTER_COND_FN_EXT              1
#define FILTER_COND_PATH_CONTAINS       2
#define FILTER_COND_PATH_IS             3
#define FILTER_COND_DIRECTORY_IS        4

#define FILTER_ACTION_INCLUDE           0
#define FILTER_ACTION_EXCLUDE           1
#define FILTER_ACTION_RENAME            2
#define FILTER_ACTION_COMPILE_INCL      3
#define FILTER_ACTION_MOVE_TO           4
#define FILTER_ACTION_FORCE_COPY        5
#define FILTER_ACTION_COMPILE_INCL_KILL 6

#include "customgui_listview.h"
#include "customgui_filename.h"
#include "lib_zipfile.h"

struct FilterElement
{
	Int32 lCondition;
	Int32 lAction;
	String str, strRename;
	Bool bSetXBit;
};

class FilterArray : public CDynamicObjectArray<FilterElement>
{
public:
	~FilterArray()
	{
		Free();
	}

	FilterElement* AllocObject() const
	{
		FilterElement* pNew = NewObjClear(FilterElement);
		return pNew;
	}

	void FreeObject(FilterElement *&pObj) const
	{
		DeleteObj(pObj);
	}

	Bool ReadElement(BaseFile* pFile, FilterElement* pElement, Int32 lLevel)
	{
		Bool bOK = true;

		if (lLevel >= 0)
		{
			bOK = bOK && pFile->ReadInt32(&pElement->lCondition);
			bOK = bOK && pFile->ReadInt32(&pElement->lAction);
			bOK = bOK && pFile->ReadString(&pElement->str);
			bOK = bOK && pFile->ReadString(&pElement->strRename);
		}
		DebugAssert(bOK);
		return bOK;
	}

	Bool WriteElement(BaseFile* pFile, const FilterElement* pElement) const
	{
		Bool bOK = true;
		// level 0
		bOK = bOK && pFile->WriteInt32(pElement->lCondition);
		bOK = bOK && pFile->WriteInt32(pElement->lAction);
		bOK = bOK && pFile->WriteString(pElement->str);
		bOK = bOK && pFile->WriteString(pElement->strRename);
		DebugAssert(bOK);
		return bOK;
	}
};

struct PresetElement
{
	Filename fnPath;
	String strOrigin;
	String strDestination;
	String strName;
	String strPassword;
	FilterArray arFilters;
	Bool bCreateZipFile, bCheckVersion, bParseSymbols, bWriteBuildInfo, bBatch, bRemoveSCC;
	Int32 lCreateZipCompressionLevel;

	UInt32 ulCRC;

	PresetElement()
	{
		lCreateZipCompressionLevel = 9;
	}
};

class PresetArray : public CDynamicObjectArray<PresetElement>
{
public:
	~PresetArray()
	{
		Free();
	}

	PresetElement* AllocObject() const
	{
		PresetElement* pNew = NewObjClear(PresetElement);
		if (pNew)
			pNew->ulCRC = (UInt32)(-1);
		return pNew;
	}

	void FreeObject(PresetElement *&pObj) const
	{
		DeleteObj(pObj);
	}

	Bool ReadElement(BaseFile* pFile, PresetElement* pElement, Int32 lLevel)
	{
		Bool bOK = true;
		if (lLevel >= 0)
		{
			bOK = bOK && pFile->ReadString(&pElement->strName);
			bOK = bOK && pFile->ReadString(&pElement->strOrigin);
			bOK = bOK && pFile->ReadString(&pElement->strDestination);
			bOK = bOK && pFile->ReadBool(&pElement->bCreateZipFile);
			bOK = bOK && pElement->arFilters.Read(pFile);
		}
		if (lLevel >= 1)
		{
			bOK = bOK && pFile->ReadBool(&pElement->bCheckVersion);
		}
		if (lLevel >= 2)
		{
			bOK = bOK && pFile->ReadBool(&pElement->bParseSymbols);
		}
		if (lLevel >= 3)
		{
			bOK = bOK && pFile->ReadBool(&pElement->bWriteBuildInfo);
		}
		if (lLevel >= 4)
		{
			bOK = bOK && pFile->ReadBool(&pElement->bRemoveSCC);
		}
		if (lLevel >= 5)
		{
			bOK = bOK && pFile->ReadInt32(&pElement->lCreateZipCompressionLevel);
		}
		DebugAssert(bOK);
		return bOK;
	}

	Bool WriteElement(BaseFile* pFile, const PresetElement* pElement) const
	{
		DebugAssert(false);
		return false;
	}
};

Bool MakeDistriCommandLine(String presetname, const String &buildid);

class CCompressProgressDlg;
class DistriThread;
class MakeDistriDialog : public GeModalDialog
{
	friend Bool MakeDistriCommandLine(String presetname, const String &buildid);
public:
	MakeDistriDialog();
	virtual ~MakeDistriDialog();

	virtual Bool CreateLayout(void);
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
	virtual Bool InitValues(void);
	virtual Int32 Message(const BaseContainer &msg, BaseContainer &result);
	virtual Bool AskClose();

	void RefreshPresets();
	void RefreshFilters();
	void DeleteSelected();
	void DeleteSelectedFilter();
	void SelectPreset(PresetElement* pElement);
	void SelectFilter(FilterElement* pElement);
	void DoEnable();
	Bool MakeDistribution(PresetElement* pElement, const String &strPassword, String* pstrError);
	void MakeDistributionThread(PresetElement* pElement, CCompressProgressDlg *pProgressDlg, DistriThread* pThread);
	void ParseSymbols();
	Bool IsSelected(PresetElement* pElement) { return m_pSelPreset == pElement; }
	Bool IsSelected(FilterElement* pElement) { return m_pSelFilter == pElement; }
	GeUserArea* GetPresetListArea() { return m_pPresetList->GetTreeViewArea(); }
	Bool CopyFiles(const Filename &fnDestPath, Filename fnCurrent, Filename fnDest, PresetElement* pElement, ZipFile* pZipFile,
		CCompressProgressDlg *pProgressDlg, DistriThread* pThread, Bool bPathExists = false);
	FilterElement* IncludeElement(const Filename &fn, Bool bIsDir, PresetElement* pElement);
	String RenamePath(const String &str, PresetElement* pElement);
	void WriteBuildInfo(const Filename fn, ZipFile* pZipFile);

protected:
	PresetElement* m_pSelPreset;
	FilterElement* m_pSelFilter;

	FilenameCustomGui *m_pRootDir, *m_pDestDir;
	TreeViewCustomGui *m_pPresetList, *m_pFilterList;
	String m_strBuildVersion;
	Filename m_fnCinemaRes;

	Bool m_bCreateZipFile, m_lZipCompressionLevel, m_bCheckVersion, m_bParseSymbols, m_bBuildInfo, m_bRemoveSCC;
	char* m_pszPassword;
};

#endif // MAKEDISTRIDIALOG_H_3fbceeee
