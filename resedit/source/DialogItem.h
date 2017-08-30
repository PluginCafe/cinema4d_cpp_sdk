/*********************************************************************\
	File name        : DialogItem.h
	Description      : Interface for the class CDialogItem
	Created at       : 11.08.01, @ 14:49:32
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#if !defined(AFX_DIALOGITEM_H__D3452F40_8E67_11D5_9B3B_004095418E0F__INCLUDED_)
#define AFX_DIALOGITEM_H__D3452F40_8E67_11D5_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DialogIO.h"

class CChildItems
{
public:
	CChildItems();
	~CChildItems();

	inline Int32 GetItemCount() { return m_lElements; }
	void Destroy();
	void AddElement(String strID, String strStringID);
	Bool GetElement(Int32 lID, String &strID, String &strStringID);
	Bool SetElement(Int32 lID, String &strID, String &strStringID);
	Bool Write(BaseFile* pFile, String strFill);
	void Load(BaseContainer* pContainer);
	void FromString(String &str);

	void FillEditBox(GeDialog* pDlg, Int32 lEditID);

protected:
	String **m_pstrID, **m_pstrStringID;
	Int32 m_lElements;
};

class TreeViewItem;

class CDialogItem
{
//  friend class CDialogItem;
	friend class CDialogDoc;
public:
	CDialogItem();
	CDialogItem(CDialogDoc* pDoc);
	virtual ~CDialogItem();

	String GetDescr() { return m_strDescr; }
	void SetDescr(String str) { m_strDescr = str; }
	virtual String GetNameID() { return m_strNameID; }

	Int32 GetFlags() { return m_lFlags; }
	void SetFlags(Int32 l) { m_lFlags = l; }

	ItemType GetType() { return m_type; }

	virtual void CreateElementBegin(Int32 lID, GeDialog *pDlg) = 0;
	virtual void CreateElementEnd(Int32 lID, GeDialog *pDlg) { };
	virtual void InitValues() { } ;

	virtual Bool Load(BaseContainer* bc) = 0;
	virtual Bool Save(BaseFile* pFile, String strFill) = 0;
	Bool SaveAlignment(BaseFile* pFile, Bool bInsertBreakes = false, Bool bWriteSize = true);
	void AddString();
	inline void ItemChanged() { m_pDocument->ItemChanged(); }
	inline String GetControlID() { return m_strControlID; }
	CDialogDoc* GetDocument() { return m_pDocument; }

	Int32 m_lID; // the ID of the item in the preview dialog
	GeDialog* m_pDialog; // the preview dialog

	Int32 m_lFlags;
	Int32 m_lInitW, m_lInitH;
	Int32 m_lBorderStyle;
	String m_strControlID;
	String m_strName, m_strNameID;

	TreeViewItem* m_pTreeViewItem;

protected:
	String CreateControlID();

	CDialogDoc* m_pDocument;

	String m_strDescr;
	ItemType m_type;
};

class CDialogTemplate : public CDialogItem
{
public:
	CDialogTemplate(CDialogDoc* pDoc);

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);

	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);
};

class CDialogStatic : public CDialogItem
{
public:
	CDialogStatic(CDialogDoc* pDoc);
	void CreateElementBegin(Int32 lID, GeDialog *pDlg);

	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);
};

#define GROUP_TYPE_SIMPLE     0
#define GROUP_TYPE_TAB        1
#define GROUP_TYPE_SCROLL     2
#define GROUP_TYPE_RADIO      3

class CDialogGroup : public CDialogItem
{
public:
	CDialogGroup(CDialogDoc* pDoc);
	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	void CreateElementEnd(Int32 lID, GeDialog *pDlg);
	String GetNameID();

	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);

	Int32 m_lGroupType;
	// simple
	Int32 m_lCols, m_lRows, m_lGroupFlags;
	Bool m_bHasBorder, m_bBorderNoTitle;
	Bool m_bHasTitleCheckbox;
	Int32 m_lLeftBorder, m_lRightBorder, m_lTopBorder, m_lBottomBorder;
	Int32 m_lSpaceX, m_lSpaceY;

	// tab
	Int32 m_lTabType;
	// scroll
	Int32 m_lScrollType;
	// the ID of the currently activated child
	Int32 m_lLastChildID;
};

class CDialogCheckBox : public CDialogItem
{
public:
	CDialogCheckBox(CDialogDoc* pDoc);

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);

	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);
};

class CDialogButton : public CDialogItem
{
public:
	CDialogButton(CDialogDoc* pDoc);

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);

	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);
};

class CDialogEditBox : public CDialogItem
{
public:
	CDialogEditBox(CDialogDoc* pDoc);

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);

	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);

	Bool m_bIsNumberEdit, m_bHasArrows, m_bHasSlider, m_bIsMultilineText;
	Int32 m_lMultilineStyle;
};

class CDialogColor : public CDialogItem
{
public:
	CDialogColor(CDialogDoc* pDoc);

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);

	Bool m_bIsColorField;
	Int32 m_lLayoutFlags;
};

class CDialogArrowBtn : public CDialogItem
{
public:
	CDialogArrowBtn(CDialogDoc* pDoc);
	~CDialogArrowBtn();

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);
	Int32 m_lArrowType;
	Bool m_bIsPopupButton;
	//void GetChildItems(String str);

	//Int32 m_lChildren;
	CChildItems m_Children;
	//String* m_pstrChildren;
	//Int32* m_plChildIDs;

	inline CChildItems* GetChildren() { return &m_Children; }

	/*inline Int32 GetChildItemCount() { return m_lChildren; }
	inline String* GetChildItems() { return m_pstrChildren; }*/
};

class CDialogSlider : public CDialogItem
{
public:
	CDialogSlider(CDialogDoc* pDoc);

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);
};

class CDialogRadioBtn : public CDialogItem
{
public:
	CDialogRadioBtn(CDialogDoc* pDoc);

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);

	Bool m_bText;
};

class CDialogSeparator : public CDialogItem
{
public:
	CDialogSeparator(CDialogDoc* pDoc);

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);

	Bool m_bHorz;
};

class CDialogComboBox : public CDialogItem
{
public:
	CDialogComboBox(CDialogDoc* pDoc);
	~CDialogComboBox();

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);
	//void GetChildItems(String str);

	/*inline Int32 GetChildItemCount() { return m_lChildren; }
	inline String* GetChildItems() { return m_pstrChildren; }*/

	CChildItems m_Children;
	inline CChildItems* GetChildren() { return &m_Children; }

	/*Int32 m_lChildren;
	String* m_pstrChildren;
	Int32* m_plChildIDs;*/
};

class CDialogComboButton : public CDialogItem
{
public:
	CDialogComboButton(CDialogDoc* pDoc);
	~CDialogComboButton();

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);
	//void GetChildItems(String str);

	/*inline Int32 GetChildItemCount() { return m_lChildren; }
	inline String* GetChildItems() { return m_pstrChildren; }*/

	CChildItems m_Children;
	inline CChildItems* GetChildren() { return &m_Children; }

	/*Int32 m_lChildren;
	String* m_pstrChildren;
	Int32* m_plChildIDs;*/
};

class CDialogListBox : public CDialogItem
{
public:
	CDialogListBox(CDialogDoc* pDoc);
	~CDialogListBox();

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);
};

class CDialogDlgGroup : public CDialogItem
{
public:
	CDialogDlgGroup(CDialogDoc* pDoc);

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);

	Int32 m_lItems;
};

class CDialogUserArea;
class CDialogUserAreaWindow : public GeUserArea
{
	friend class CDialogUserArea;
public:
	CDialogUserAreaWindow();
	~CDialogUserAreaWindow();

	virtual void Sized(Int32 w,Int32 h);
	virtual Bool GetMinSize(Int32 &w,Int32 &h);
	Bool InputEvent(const BaseContainer &msg);
	void DrawMsg(Int32 x1, Int32 y1, Int32 x2, Int32 y2, const BaseContainer &bcMsg);

protected:
	GeDialog *m_pParent;
	Int32 m_lMinSizeX, m_lMinSizeY;
	BaseBitmap* m_pImage;
	Int32 m_lID;
};

class CDialogUserArea : public CDialogItem
{
public:
	CDialogUserArea(CDialogDoc* pDoc);
	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);

protected:
	CDialogUserAreaWindow m_wndUser;
};

class CDialogSubDlg : public CDialogItem
{
public:
	CDialogSubDlg(CDialogDoc* pDoc);
	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);
};

class CDialogCustomElement : public CDialogItem
{
public:
	CDialogCustomElement(CDialogDoc* pDoc);
	virtual ~CDialogCustomElement();

	void CreateElementBegin(Int32 lID, GeDialog *pDlg);
	Bool Load(BaseContainer* bc);
	Bool Save(BaseFile* pFile, String strFill);
	void InitValues();

	Int32 GetElementPluginID();
	TriState<GeData> GetElementData();
	void ResetData(Bool bResetAll = false);

	Int32 m_lElement;
	BaseContainer *m_pbcGUI;
	TriState<GeData> *m_pData;

	Bool m_bIsOpen;
};


#endif // !defined(AFX_DIALOGITEM_H__D3452F40_8E67_11D5_9B3B_004095418E0F__INCLUDED_)
