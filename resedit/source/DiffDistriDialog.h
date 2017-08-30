// DiffDistriDialog.h
//////////////////////////////////////////////////////////////////////

#ifndef DIFFDISTRIDIALOG_H__
#define DIFFDISTRIDIALOG_H__

#include "c4d_gui.h"
#include "customgui_listview.h"
#include "customgui_filename.h"
#include "dynarray.h"

class FileListFunctions;

struct ZipArrayEntry
{
	String strName;
	UInt32 ulCRCNew;
	UInt32 lNewSize;
	Bool bInOld, bInNew, bChanged;
	Bool bInAppFolder;
};

class ZipArray : public CDynamicSortObjectSet<ZipArrayEntry>
{
protected:
	ZipArrayEntry* AllocObject() const { return NewObjClear(ZipArrayEntry); }
	void FreeObject(ZipArrayEntry *&pObj) const { DeleteObj(pObj); }
	Int32 Compare(const ZipArrayEntry* a, const ZipArrayEntry* b) const { return a->strName.LexCompare(b->strName); }
	Int32 CompareSearch(const ZipArrayEntry* a, const void* b) const { return a->strName.LexCompare(*((String*)b)); }
};

class DiffDistriDialog : public GeDialog
{
	friend class FileListFunctions;

public:
	DiffDistriDialog();
	virtual ~DiffDistriDialog();

	virtual Bool CreateLayout(void);
	virtual Bool InitValues(void);
	virtual Bool Command(Int32 id, const BaseContainer &msg);

protected:
	TreeViewCustomGui* m_pTreeView;
	FilenameCustomGui* m_pNewZipEdit, *m_pDestPathEdit;
	Filename m_fnSelection;
};

#endif	// DIFFDISTRIDIALOG_H__
