// BrowserMenu.h: Schnittstelle für die Klasse CBrowserMenu.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BROWSERMENU_H__277056E2_40AA_11D6_9B3B_004095418E0F__INCLUDED_)
#define AFX_BROWSERMENU_H__277056E2_40AA_11D6_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBrowserMenu : public CommandData
{
public:
	CBrowserMenu();
	virtual ~CBrowserMenu();

	virtual Bool Execute(BaseDocument *doc);
	virtual Int32 GetState(BaseDocument *doc) { return CMD_ENABLED; }
	virtual Bool RestoreLayout(void *secret);
};

#endif // !defined(AFX_BROWSERMENU_H__277056E2_40AA_11D6_9B3B_004095418E0F__INCLUDED_)
