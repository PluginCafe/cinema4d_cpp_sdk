/*********************************************************************\
	File name        : ResEditBrowser.h
	Description      : Schnittstelle für die Klasse CResEditBrowser
	Created at       : 25.09.01, @ 22:18:58
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#if !defined(AFX_RESEDITBROWSER_H__353046E0_B203_11D5_9B3B_004095418E0F__INCLUDED_)
#define AFX_RESEDITBROWSER_H__353046E0_B203_11D5_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CResEditBrowser : public GeDialog
{
public:
	CResEditBrowser();
	virtual ~CResEditBrowser();

	virtual Bool CreateLayout(void);
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
	virtual Bool InitValues(void);
	virtual Int32 Message(const BaseContainer &msg, BaseContainer &result);

protected:
	void FillList();
	void BrowsePath(Filename fn);

	Filename m_strPath;
	/*SimpleListView m_wndListView;*/
	TreeView m_wndTreeView;

	TreeViewItem *m_pDialogsItem, *m_pImagesItem, *m_pStringTablesItem;
};

#endif // !defined(AFX_RESEDITBROWSER_H__353046E0_B203_11D5_9B3B_004095418E0F__INCLUDED_)
