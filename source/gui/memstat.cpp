// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_MEMSTAT 200000072

#include "c4d.h"
#include "gradientuserarea.h"
#include "c4d_symbols.h"
#include "main.h"

class MemStatDialog : public GeDialog
{
private:
	void CheckMaxMemory(Int32 mbblocks);

public:
	MemStatDialog();
	virtual Bool CreateLayout();
	virtual Bool InitValues();
	virtual Bool Command(Int32 id, const BaseContainer& msg);
	virtual Int32 Message(const BaseContainer& msg, BaseContainer& result);
	virtual Bool CoreMessage  (Int32 id, const BaseContainer& msg);
	virtual void Timer(const BaseContainer& msg);
};

enum
{
	IDC_MEMORY_STAT_MEMORY_INUSE = 1000,
	IDC_MEMORY_STAT_MEMORY_PEAK,
	IDC_MEMORY_STAT_NO_OF_ALLOCATIONS_TOTAL,
	IDC_MEMORY_STAT_NO_OF_ALLOCATIONS_CURRENT,
	IDC_MEMORY_STAT_OGL_MEMORY,
	IDC_MEMORY_STAT_EOGL_TEXBUFFER,
	IDC_MEMORY_STAT_EOGL_VERTEXBUFFER,

	IDC_MEMORY_TEST_1MB,
	IDC_MEMORY_TEST_10MB,
	IDC_MEMORY_TEST_100MB,

	IDC_MEMORY_TEST_1MB_RES,
	IDC_MEMORY_TEST_10MB_RES,
	IDC_MEMORY_TEST_100MB_RES,

	IDC_MEMORY_STAT_
};

MemStatDialog::MemStatDialog()
{
}

Bool MemStatDialog::CreateLayout()
{
	// first call the parent instance
	Bool res = GeDialog::CreateLayout();

	SetTitle("Memory Statistics");

	GroupBegin(0, BFH_SCALEFIT, 2, 0, "", 0);
	{
		GroupBorderSpace(4, 4, 4, 4);
		AddStaticText(0, BFH_FIT, 0, 0, "Memory In Use", 0); AddStaticText(IDC_MEMORY_STAT_MEMORY_INUSE, BFH_SCALEFIT, SizeChr(140), 0, String(), 0);
		AddStaticText(0, BFH_FIT, 0, 0, "Memory Peak", 0); AddStaticText(IDC_MEMORY_STAT_MEMORY_PEAK, BFH_SCALEFIT, SizeChr(140), 0, String(), 0);
		AddStaticText(0, BFH_FIT, 0, 0, "Number of Allocations (Current)", 0); AddStaticText(IDC_MEMORY_STAT_NO_OF_ALLOCATIONS_CURRENT, BFH_SCALEFIT, SizeChr(100), 0, String(), 0);
		AddStaticText(0, BFH_FIT, 0, 0, "Number of Allocations (Total)", 0); AddStaticText(IDC_MEMORY_STAT_NO_OF_ALLOCATIONS_TOTAL, BFH_SCALEFIT, SizeChr(100), 0, String(), 0);
		AddStaticText(0, BFH_FIT, 0, 0, "OpenGL memory (total/free) MiB", 0); AddStaticText(IDC_MEMORY_STAT_OGL_MEMORY, BFH_SCALEFIT, SizeChr(100), 0, String(), 0);
		AddStaticText(0, BFH_FIT, 0, 0, "EOGL Texture Cache", 0); AddStaticText(IDC_MEMORY_STAT_EOGL_TEXBUFFER, BFH_SCALEFIT, SizeChr(100), 0, String(), 0);
		AddStaticText(0, BFH_FIT, 0, 0, "EOGL VBO Cache", 0); AddStaticText(IDC_MEMORY_STAT_EOGL_VERTEXBUFFER, BFH_SCALEFIT, SizeChr(100), 0, String(), 0);
	}
	GroupEnd();

	GroupBegin(0, BFH_LEFT, 2, 0, "", 0);
	{
		AddButton(IDC_MEMORY_TEST_1MB, BFH_FIT, 0, 0, "Test Max Memory Alloc (  1 MB)");
		AddStaticText(IDC_MEMORY_TEST_1MB_RES, BFH_FIT, SizeChr(140), 0, String(), 0);

		AddButton(IDC_MEMORY_TEST_10MB, BFH_FIT, 0, 0, "Test Max Memory Alloc ( 10 MB)");
		AddStaticText(IDC_MEMORY_TEST_10MB_RES, BFH_FIT, SizeChr(140), 0, String(), 0);

		AddButton(IDC_MEMORY_TEST_100MB, BFH_FIT, 0, 0, "Test Max Memory Alloc (100 MB)");
		AddStaticText(IDC_MEMORY_TEST_100MB_RES, BFH_FIT, SizeChr(140), 0, String(), 0);
	}
	GroupEnd();

	SetTimer(500);

	return res;
}

static String GetOGLMemoryString(Int32 l)
{
	return l <= 0 ? "unknown" : String::IntToString(l >> 10);
}

static String GetNoOfAllocationsString(Int64 amount)
{
	return amount <= 0 ? "unknown" : String::IntToString(amount);
}

Bool MemStatDialog::InitValues()
{
	// first call the parent instance
	if (!GeDialog::InitValues())
		return false;

	BaseContainer stat;
	// since this function is slow we have to tell Cinema that we need this information by setting 1
	stat.SetInt32(C4D_MEMORY_STAT_OPENGL_TOTAL, 1);
	GeGetMemoryStat(stat);

	SetString(IDC_MEMORY_STAT_MEMORY_INUSE, String::MemoryToString(stat.GetInt64(C4D_MEMORY_STAT_MEMORY_INUSE)));
	SetString(IDC_MEMORY_STAT_MEMORY_PEAK, String::MemoryToString(stat.GetInt64(C4D_MEMORY_STAT_MEMORY_PEAK)));
	SetString(IDC_MEMORY_STAT_NO_OF_ALLOCATIONS_TOTAL, GetNoOfAllocationsString(stat.GetInt64(C4D_MEMORY_STAT_NO_OF_ALLOCATIONS_TOTAL)));
	SetString(IDC_MEMORY_STAT_NO_OF_ALLOCATIONS_CURRENT, GetNoOfAllocationsString(stat.GetInt64(C4D_MEMORY_STAT_NO_OF_ALLOCATIONS_CURRENT)));
	SetString(IDC_MEMORY_STAT_OGL_MEMORY, GetOGLMemoryString(stat.GetInt32(C4D_MEMORY_STAT_OPENGL_TOTAL)) + " / " + GetOGLMemoryString(stat.GetInt32(C4D_MEMORY_STAT_OPENGL_FREE)));
	SetString(IDC_MEMORY_STAT_EOGL_TEXBUFFER, String::MemoryToString(stat.GetInt64(C4D_MEMORY_STAT_EOGL_TEXBUFFER)) + String(" (") + String::IntToString(stat.GetInt32(C4D_MEMORY_STAT_EOGL_TEXTUREBUFFER_CNT)) + String(")"));
	SetString(IDC_MEMORY_STAT_EOGL_VERTEXBUFFER, String::MemoryToString(stat.GetInt64(C4D_MEMORY_STAT_EOGL_VERTEXBUFFER)) + String(" (") + String::IntToString(stat.GetInt32(C4D_MEMORY_STAT_EOGL_VERTEXBUFFER_CNT)) + String(")"));

	return true;
}

void MemStatDialog::Timer(const BaseContainer& msg)
{
	InitValues();
}

void MemStatDialog::CheckMaxMemory(Int32 mbblocks)
{
	maxon::BaseArray<void*> blocks;
	Int32 i;

	for (i = 0; true; i++)
	{
		void* block = nullptr;
		if (mbblocks > 0)
			block = NewMem(UChar, mbblocks * 1024 * 1024);
		if (!block)
			break;
		InitValues();
		if (!blocks.Append(block))
		{
			DeleteMem(block);
			break;
		}
		InitValues();
	}

	BaseContainer stat;
	GeGetMemoryStat(stat);

	for (i = 0; i < blocks.GetCount(); i++)
	{
		void* block = blocks[i];
		if (block)
			DeleteMem(block);
	}

	InitValues();

	String memstr = String::MemoryToString(stat.GetInt64(C4D_MEMORY_STAT_MEMORY_INUSE));

	switch (mbblocks)
	{
		case 1:			SetString(IDC_MEMORY_TEST_1MB_RES, memstr); break;
		case 10:		SetString(IDC_MEMORY_TEST_10MB_RES, memstr); break;
		case 100:		SetString(IDC_MEMORY_TEST_100MB_RES, memstr); break;
	}

	GeOutString("Max memory allocation: " + memstr, GEMB_OK);
}

Bool MemStatDialog::Command(Int32 id, const BaseContainer& msg)
{
	switch (id)
	{
		case IDC_MEMORY_TEST_1MB:			CheckMaxMemory(1); break;
		case IDC_MEMORY_TEST_10MB:		CheckMaxMemory(10); break;
		case IDC_MEMORY_TEST_100MB:		CheckMaxMemory(100); break;
	}
	return true;
}

Bool MemStatDialog::CoreMessage(Int32 id, const BaseContainer& msg)
{
	switch (id)
	{
		case EVMSG_CHANGE:
			if (CheckCoreMessage(msg))
			{
			}
			break;
	}
	return GeDialog::CoreMessage(id, msg);
}

Int32 MemStatDialog::Message(const BaseContainer& msg, BaseContainer& result)
{
	//switch (msg.GetId())
	{
	}
	return GeDialog::Message(msg, result);
}

class MemStatCommand : public CommandData
{
private:
	MemStatDialog dlg;

public:
	virtual Bool Execute(BaseDocument* doc);
	virtual Int32 GetState(BaseDocument* doc);
	virtual Bool RestoreLayout(void* secret);
};

Int32 MemStatCommand::GetState(BaseDocument* doc)
{
	return CMD_ENABLED;
}

Bool MemStatCommand::Execute(BaseDocument* doc)
{
	return dlg.Open(DLG_TYPE_ASYNC, ID_MEMSTAT, -1, -1);
}

Bool MemStatCommand::RestoreLayout(void* secret)
{
	return dlg.RestoreLayout(ID_MEMSTAT, 0, secret);
}

Bool RegisterMemoryStat()
{
	return RegisterCommandPlugin(ID_MEMSTAT, String("C++ SDK - Memory Statistics"), 0, nullptr, String(), NewObjClear(MemStatCommand));
}

