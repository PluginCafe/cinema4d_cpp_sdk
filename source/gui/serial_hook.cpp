#include "c4d_resource.h"
#include "lib_sn.h"
#include "c4d_symbols.h"
#include "main.h"

class ExampleSNHookClass : public SNHookClass
{
public:
	String name;

	ExampleSNHookClass()
	{
		name = GeLoadString(IDS_SERIAL_HOOK);
		if (!name.Content())
			name = "C++ SDK - Example Serial Hook";
	}

	virtual ~ExampleSNHookClass() { }

	virtual Int32 SNCheck(const String& c4dsn, const String& sn, Int32 regdate, Int32 curdate)
	{
		return sn == String("123456789-abcdef") ? SN_OKAY : SN_WRONGNUMBER;
	}

	virtual const String& GetTitle()
	{
		return name;
	}
};

ExampleSNHookClass* snhook = nullptr;

Bool RegisterExampleSNHook()
{
	snhook = NewObjClear(ExampleSNHookClass);
	if (!snhook->Register(450000241, SNFLAG_OWN))
		return false;
	return true;
}

void FreeExampleSNHook()
{
	if (snhook)
		DeleteObj(snhook);
}
