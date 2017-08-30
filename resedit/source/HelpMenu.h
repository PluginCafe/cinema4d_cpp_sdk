// HelpMenu.h: Schnittstelle für die Klasse CHelpMenu.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELPMENU_H__277056E4_40AA_11D6_9B3B_004095418E0F__INCLUDED_)
#define AFX_HELPMENU_H__277056E4_40AA_11D6_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CHelpMenu : public CommandData
{
public:
	CHelpMenu();
	virtual ~CHelpMenu();

	virtual Bool Execute(BaseDocument *doc);
	virtual Int32 GetState(BaseDocument *doc) { return CMD_ENABLED; }
};

#endif // !defined(AFX_HELPMENU_H__277056E4_40AA_11D6_9B3B_004095418E0F__INCLUDED_)
