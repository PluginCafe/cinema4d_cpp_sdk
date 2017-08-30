// WizardDialog.h :  Interface definition of class WizardDialog
//
//////////////////////////////////////////////////////////////////////

#if !defined WIZARDDIALOG_H_418b1f53
#define WIZARDDIALOG_H_418b1f53

class ScriptData;

class WizardDialog : public GeModalDialog
{
public:
	WizardDialog();
	virtual ~WizardDialog();

	virtual Bool CreateLayout(void);
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
	virtual Bool InitValues(void);
	virtual Int32 Message(const BaseContainer &msg, BaseContainer &result);
	virtual Bool AskClose();

protected:
	void DoIt();
	Bool ModifyFile(const Filename &fn, ScriptData* pScript, const Filename &fnDir);

	FilenameCustomGui* m_FileGUI;
};

#endif // WIZARDDIALOG_H_418b1f53
