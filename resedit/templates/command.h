// ##CLASSNAME##.h
//////////////////////////////////////////////////////////////////////

#ifndef COMMAND_H__
#define COMMAND_H__

#include "c4d_commanddata.h"

class ##CLASSNAME## : public CommandData
{
public:
	##CLASSNAME##();
	virtual ~##CLASSNAME##();

	enum ##CLASSNAME##_ID { ID = ##PLUGINID## };

##GETSTATE##	virtual Int32 GetState(BaseDocument *doc);
##.GETSTATE##
##EXECUTE##	virtual Bool Execute(BaseDocument *doc);
##.EXECUTE##
##RESTORELAYOUT##	virtual Bool RestoreLayout(void *secret);
##.RESTORELAYOUT##
};

#endif	// COMMAND_H__
