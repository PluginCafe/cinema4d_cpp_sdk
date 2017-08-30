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

##GETSTATE##
Int32 ##CLASSNAME##::GetState(BaseDocument *doc)
{
	return CMD_ENABLED;
}

##.GETSTATE##
##EXECUTE##Bool ##CLASSNAME##::Execute(BaseDocument *doc)
{
	return true;
}

##.EXECUTE##
##RESTORELAYOUT##Bool ##CLASSNAME##::RestoreLayout(void *secret)
{
	return true;
}

##.RESTORELAYOUT##
##REGISTER##
Bool Register##CLASSNAME##()
{
	return RegisterCommandPlugin(##CLASSNAME##::ID, "My Command", 0, "MyCommand", NewObjClear(##CLASSNAME##));
}

##.REGISTER##
