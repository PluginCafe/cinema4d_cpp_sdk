// CompareMenu.h: Schnittstelle für die Klasse CCompareMenu.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPAREMENU_H__277056E3_40AA_11D6_9B3B_004095418E0F__INCLUDED_)
#define AFX_COMPAREMENU_H__277056E3_40AA_11D6_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCompareMenu : public CommandData
{
public:
	CCompareMenu();
	virtual ~CCompareMenu();

	virtual Bool Execute(BaseDocument *doc);
	virtual Int32 GetState(BaseDocument *doc) { return CMD_ENABLED; }
};

#endif // !defined(AFX_COMPAREMENU_H__277056E3_40AA_11D6_9B3B_004095418E0F__INCLUDED_)
