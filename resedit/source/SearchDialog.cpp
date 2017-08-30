// SearchDialog.cpp
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "globals.h"
#include "SearchDialog.h"

#include "StringTableTranslateDialog.h"

CSearchDialog::CSearchDialog()
{
	m_bRegExpr = false;
	m_pregExpr = RegularExprParser::Alloc();
}

CSearchDialog::~CSearchDialog()
{
	RegularExprParser::Free(m_pregExpr);
}

Bool CSearchDialog::CreateLayout()
{
	if (!GeDialog::CreateLayout() || !LoadDialogResource(IDD_SEARCH_DIALOG, nullptr, 0))
		return false;

	return true;
}

Bool CSearchDialog::Command(Int32 lID, const BaseContainer &msg)
{
	if (!m_pregExpr)
		return true;

	switch (lID)
	{
	case IDC_SEARCH_FIND_NEXT_CURRENT_BTN:
		{
			String str;
			Bool bCaseSensitive = false;
			GetString(IDC_SEARCH_EDIT, str);
			GetBool(IDC_SEARCH_CASE_SENSITIVE_CHK, bCaseSensitive);
			if (m_bRegExpr)
			{
				if (str != m_strText)
				{
					if (!m_pregExpr->Init(str))
					{
						GeOutString(GeLoadString(IDS_INVALID_REG_EXPR), GEMB_ICONEXCLAMATION);
						m_strText = "";
						return true;
					}
				}
				m_pDlg->FindString(str, m_pregExpr, true, bCaseSensitive);
			}
			else
				m_pDlg->FindString(str, nullptr, true, bCaseSensitive);
			m_strText = str;
		}
		break;

	case IDC_SEARCH_FIND_NEXT_ALL_BTN:
		{
			String str;
			Bool bCaseSensitive = false;
			GetString(IDC_SEARCH_EDIT, str);
			GetBool(IDC_SEARCH_CASE_SENSITIVE_CHK, bCaseSensitive);
			if (m_bRegExpr)
			{
				if (str != m_strText)
				{
					if (!m_pregExpr->Init(str))
					{
						GeOutString(GeLoadString(IDS_INVALID_REG_EXPR), GEMB_ICONEXCLAMATION);
						m_strText = "";
						return true;
					}
				}
				m_pDlg->FindString(str, m_pregExpr, false, bCaseSensitive);
			}
			else
				m_pDlg->FindString(str, nullptr, false, bCaseSensitive);
			m_strText = str;
		}
		break;
	case IDC_SEARCH_REG_EXPR_CHK:
		GetBool(IDC_SEARCH_REG_EXPR_CHK, m_bRegExpr);
		break;
	case IDC_SEARCH_CLOSE_BTN:
		Close();
		break;
	default:
		break;
	}
	return true;
}

Bool CSearchDialog::InitValues()
{
	if (!GeDialog::InitValues()) return false;

	SetBool(IDC_SEARCH_REG_EXPR_CHK, m_bRegExpr);
	SetString(IDC_SEARCH_EDIT, m_strText);

	return true;
}

Int32 CSearchDialog::Message(const BaseContainer &msg, BaseContainer &result)
{
/*
	switch (msg.GetId())
	{
	default:
		break;
	}
*/
	return GeDialog::Message(msg, result);
}

Bool CSearchDialog::AskClose()
{
	// return true, if the dialog should not be closed
	return false;
}
