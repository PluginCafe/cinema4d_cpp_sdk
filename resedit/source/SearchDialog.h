// SearchDialog.h :  Interface definition of class CSearchDialog
//
//////////////////////////////////////////////////////////////////////

#if !defined SEARCHDIALOG_H_3ff7db06
#define SEARCHDIALOG_H_3ff7db06

class CStringTranslateDialog;
class CSearchDialog : public GeDialog
{
public:
	CSearchDialog();
	virtual ~CSearchDialog();

	virtual Bool CreateLayout(void);
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
	virtual Bool InitValues(void);
	virtual Int32 Message(const BaseContainer &msg, BaseContainer &result);
	virtual Bool AskClose();

	CStringTranslateDialog* m_pDlg;
protected:
	String m_strText;
	RegularExprParser *m_pregExpr;
	Bool m_bRegExpr;
};

#endif // SEARCHDIALOG_H_3ff7db06
