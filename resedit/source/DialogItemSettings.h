#ifndef DIALOGITEMSETTINGS_H__
#define DIALOGITEMSETTINGS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDialogSettings : public CSubSettingsDialog
{
public:
	CDialogSettings();
	virtual ~CDialogSettings();
	void SetData();

	void SetItem(CDialogTemplate* pDlg) { m_pDialog = pDlg; }
protected:
	CDialogTemplate* m_pDialog;

};

class CStaticSettings : public CSubSettingsDialog
{
public:
	CStaticSettings();
	virtual ~CStaticSettings();

	void SetData();
	void SetItem(CDialogStatic* pStatic) { m_pStatic = pStatic; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogStatic* m_pStatic;
};

class CEditboxSettings : public CSubSettingsDialog
{
public:
	CEditboxSettings();
	virtual ~CEditboxSettings();

	void SetData();

	void SetItem(CDialogEditBox* pBox) { m_pEditBox = pBox; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogEditBox* m_pEditBox;
};

class CGroupSettings : public CSubSettingsDialog
{
public:
	CGroupSettings();
	virtual ~CGroupSettings();
	void SetData();

	void SetItem(CDialogGroup* pGroup) { m_pGroup = pGroup; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogGroup* m_pGroup;

	Int32 m_lBorderPage;
};

class CColorSettings : public CSubSettingsDialog
{
public:
	CColorSettings();
	virtual ~CColorSettings();

	void SetItem(CDialogColor* pColor) { m_pColor = pColor; }

	void SetData();
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogColor* m_pColor;
};

class CCheckboxSettings : public CSubSettingsDialog
{
public:
	CCheckboxSettings();
	virtual ~CCheckboxSettings();

	void SetData();

	void SetItem(CDialogCheckBox* pBox) { m_pCheckBox = pBox; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg) { return true; }
protected:
	CDialogCheckBox* m_pCheckBox;
};

class CButtonSettings : public CSubSettingsDialog
{
public:
	CButtonSettings();
	virtual ~CButtonSettings();

	void SetData();
	void SetItem(CDialogButton* pBtn) { m_pButton = pBtn; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg) { return true; }
protected:
	CDialogButton* m_pButton;
};

class CArrowSettings : public CSubSettingsDialog
{
public:
	CArrowSettings();
	virtual ~CArrowSettings();

	void SetData();
	void SetItem(CDialogArrowBtn* pArrow) { m_pArrow = pArrow; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogArrowBtn* m_pArrow;
};

class CSliderSettings : public CSubSettingsDialog
{
public:
	CSliderSettings();
	virtual ~CSliderSettings();

	void SetData();
	void SetItem(CDialogSlider* pSlider) { m_pSlider = pSlider; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogSlider* m_pSlider;
};

class CRadioBtnSettings : public CSubSettingsDialog
{
public:
	CRadioBtnSettings();
	virtual ~CRadioBtnSettings();

	void SetData();
	void SetItem(CDialogRadioBtn* pRadioBtn) { m_pRadioBtn = pRadioBtn; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogRadioBtn* m_pRadioBtn;
};

class CSeparatorSettings : public CSubSettingsDialog
{
public:
	CSeparatorSettings();
	virtual ~CSeparatorSettings();

	void SetData();
	void SetItem(CDialogSeparator* pSeparator) { m_pSeparator = pSeparator; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogSeparator* m_pSeparator;
};

class CListBoxSettings : public CSubSettingsDialog
{
public:
	CListBoxSettings();
	virtual ~CListBoxSettings();

	void SetData();
	void SetItem(CDialogListBox* pListBox) { m_pListBox = pListBox; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogListBox* m_pListBox;
};

class CComboBoxSettings : public CSubSettingsDialog
{
public:
	CComboBoxSettings();
	virtual ~CComboBoxSettings();

	void SetData();
	void SetItem(CDialogComboBox* pComboBox) { m_pComboBox = pComboBox; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogComboBox* m_pComboBox;
};

class CComboButtonSettings : public CSubSettingsDialog
{
public:
	CComboButtonSettings();
	virtual ~CComboButtonSettings();

	void SetData();
	void SetItem(CDialogComboButton* pComboButton) { m_pComboButton = pComboButton; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogComboButton* m_pComboButton;
};

class CDialogGroupSettings : public CSubSettingsDialog
{
public:
	CDialogGroupSettings();
	virtual ~CDialogGroupSettings();

	void SetData();
	void SetItem(CDialogDlgGroup* pDialogGroup) { m_pDialogGroup = pDialogGroup; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogDlgGroup* m_pDialogGroup;
};

class CUserAreaSettings : public CSubSettingsDialog
{
public:
	CUserAreaSettings();
	virtual ~CUserAreaSettings();

	void SetData();
	void SetItem(CDialogUserArea* pUserArea) { m_pUserArea = pUserArea; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogUserArea* m_pUserArea;
};

class CSubDialogSettings : public CSubSettingsDialog
{
public:
	CSubDialogSettings();
	virtual ~CSubDialogSettings();

	void SetData();
	void SetItem(CDialogSubDlg* pUserArea) { m_pUserArea = pUserArea; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
protected:
	CDialogSubDlg* m_pUserArea;
};


class CDialogCustomElement;
class CCustomSubDialog : SubDialog
{
public:
	CCustomSubDialog() { m_pProp = nullptr; m_pDialog = nullptr; }

	virtual Bool CreateLayout(void);
	virtual Bool InitValues(void);
	virtual Bool Command(Int32 id,const BaseContainer &msg);

	void Refresh(GeDialog* pDlg, CustomProperty *prop, CDialogCustomElement* pElement);

	CustomProperty* m_pProp;
	GeDialog* m_pDialog;
	CDialogCustomElement* m_pElement;
};

class CCustomElementSettings : public CSubSettingsDialog
{
public:
	CCustomElementSettings();
	virtual ~CCustomElementSettings();

	void SetData();
	void SetItem(CDialogCustomElement* pElement) { m_pElement = pElement; }
	virtual Bool Command(Int32 lID, const BaseContainer &msg);

	void Init();
protected:
	CDialogCustomElement* m_pElement;

	SimpleListView m_wndListView;
	CCustomSubDialog m_wndSubDlg;

	Int32 m_lLastID;
};

#endif	// DIALOGITEMSETTINGS_H__
