/*********************************************************************\
	File name        : DirectListView.h
	Description      : Schnittstelle für die Klasse CDirectListView.
	Created at       : 27.11.01, @ 17:32:56
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#if !defined(AFX_DIRECTLISTVIEW_H__D470F7C0_E35B_11D5_B60C_0040D01B1EC1__INCLUDED_)
#define AFX_DIRECTLISTVIEW_H__D470F7C0_E35B_11D5_B60C_0040D01B1EC1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum tagDirectListViewMode { LIST_SELECTION, LIST_SPRINGS };

class CDirectListView : public SimpleListView
{
public:
	CDirectListView();
	virtual ~CDirectListView();

	virtual void LvCallBack(Int32 &res_type, void *&result, void *secret, Int32 cmd, Int32 line, Int32 col, void *data1);
protected:
	tagDirectListViewMode m_mode;
};

#endif // !defined(AFX_DIRECTLISTVIEW_H__D470F7C0_E35B_11D5_B60C_0040D01B1EC1__INCLUDED_)
