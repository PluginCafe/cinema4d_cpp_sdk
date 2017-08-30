// ##CLASSNAME##.h
//////////////////////////////////////////////////////////////////////

#ifndef DIALOG_H__
#define DIALOG_H__

#include "c4d_gui.h"

##MODALDIALOG##
class ##CLASSNAME## : public GeModalDialog
##.MODALDIALOG##
##NONMODALDIALOG##
class ##CLASSNAME## : public GeDialog
##.NONMODALDIALOG##
##SUBDIALOG##
class ##CLASSNAME## : public SubDialog
##.SUBDIALOG##
{
public:
	##CLASSNAME##();
	virtual ~##CLASSNAME##();

##CREATELAYOUT##	virtual Bool CreateLayout(void);
##.CREATELAYOUT##
##INITVALUES##	virtual Bool InitValues(void);
##.INITVALUES##
##COMMAND##	virtual Bool Command(Int32 id, const BaseContainer &msg);
##.COMMAND##
##MESSAGE##	virtual Int32 Message(const BaseContainer &msg, BaseContainer &result);
##.MESSAGE##
##ASKCLOSE##	virtual Bool AskClose();
##.ASKCLOSE##
};

#endif	// DIALOG_H__
