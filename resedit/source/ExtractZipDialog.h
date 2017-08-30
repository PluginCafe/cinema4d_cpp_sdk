// ExtractZipDialog.h :  Interface definition of class CExtractZipDialog
//
//////////////////////////////////////////////////////////////////////

#if !defined EXTRACTZIPDIALOG_H_3f199ca8
#define EXTRACTZIPDIALOG_H_3f199ca8

#include "lib_zipfile.h"
#include "customgui_filename.h"

struct ZipElement
{
	Int32 lIndex;
	String strName, strDate, strTime, strOrigSize, strNewSize, strRatio, strCRC32, strFlags;
};

class CZipElements : public CDynamicObjectArray<ZipElement>
{
protected:
	virtual ZipElement* AllocObject() const
	{
		return NewObjClear(ZipElement);
	}
	virtual void FreeObject(ZipElement *&pObj) const
	{
		DeleteObj(pObj);
		pObj = nullptr;
	}
};

class CExtractZipDialog : public GeModalDialog
{
public:
	CExtractZipDialog(const Filename& fn);
	virtual ~CExtractZipDialog();

	virtual Bool CreateLayout(void);
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
	virtual Bool InitValues(void);
	virtual Int32 Message(const BaseContainer &msg, BaseContainer &result);
	virtual Bool AskClose();
	void ExtractFileThread(C4DThread* pThread);

protected:
	Filename m_fnZip;
	TreeViewCustomGui* m_pTreeView;
	FilenameCustomGui* m_pFilenameGUI;
	CZipElements m_zipElements;
	AutoAlloc <ZipFile> m_pZipFile;
	void ExtractFile(const String &strPassword);
	char* m_pchPassword;
};

#endif // EXTRACTZIPDIALOG_H_3f199ca8
