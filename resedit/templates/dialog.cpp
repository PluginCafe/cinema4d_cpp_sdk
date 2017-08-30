// ##CLASSNAME##.cpp
//////////////////////////////////////////////////////////////////////

// TODO : Add your includes here
#include "##CLASSNAME##.h"

##CLASSNAME##::##CLASSNAME##()
{
}

##CLASSNAME##::~##CLASSNAME##()
{
}

##CREATELAYOUT##
Bool ##CLASSNAME##::CreateLayout(void)
{
	if (!##MODALDIALOG##GeModalDialog##.MODALDIALOG####NONMODALDIALOG##GeDialog##.NONMODALDIALOG####SUBDIALOG##SubDialog##.SUBDIALOG##::CreateLayout())
		return false;
	if (!LoadDialogResource(##DIALOGID##, nullptr, 0))
		return false;

	return true;
}

##.CREATELAYOUT##
##INITVALUES##
Bool ##CLASSNAME##::InitValues(void)
{
	return true;
}

##.INITVALUES##
##COMMAND##
Bool ##CLASSNAME##::Command(Int32 id, const BaseContainer &msg)
{
	switch (id)
	{
		default:
			break;
	}

	return true;
}

##.COMMAND##
##MESSAGE##
Int32 ##CLASSNAME##::Message(const BaseContainer &msg, BaseContainer &result)
{
	return ##MODALDIALOG##GeModalDialog##.MODALDIALOG####NONMODALDIALOG##GeDialog##.NONMODALDIALOG####SUBDIALOG##SubDialog##.SUBDIALOG##::Message(msg, result);
}

##.MESSAGE##
##ASKCLOSE##
Bool ##CLASSNAME##::AskClose()
{
	// return true, if the dialog should not be closed
	return false;
}

##.ASKCLOSE##
