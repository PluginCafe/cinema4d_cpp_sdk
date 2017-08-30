/*********************************************************************\
	File name        : DialogTemplate.h
	Description      :
	Created at       : 26.09.01, @ 14:09:13
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#ifndef DIALOGTEMPLATE_H__
#define DIALOGTEMPLATE_H__

class CDialogTemplate : public CDialogItem
{
public:
	CDialogTemplate(CDialogDoc* pDoc);

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);

	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);
	CStringTable* GetStringTable() { return &m_StringTable; }

protected:
	CStringTable m_StringTable;
};

#endif	// DIALOGTEMPLATE_H__
