// example code for a menu plugin and multiprocessing

#include "c4d.h"
#include "c4d_symbols.h"
#include "c4d_memory_mp.h"
#include "main.h"

class MPTest : public C4DThread
{
public:
	MPTest() : _start(0), _result(0.0), _duration(0.0) {}
	virtual void Main()
	{
		Float64 t1 = GeGetMilliSeconds();

		// calculate the 10,000,000 th random number
		_rnd.Init(_start);
		for (Int i = 0; i < 10000000; i++)
			_rnd.Get01();

		_result = _rnd.Get01();
		_duration = GeGetMilliSeconds() - t1;
	}
	virtual const Char* GetThreadName(void) { return "SDK MpTest"; }
	void SetStart(Int32 start) { _start = start; }
	Float GetResult() const { return _result; }
	Float GetDuration() const { return _duration; }

private:
	Random _rnd;
	Int32	 _start;
	Float	 _result;
	Float	 _duration;
};


class ControlThread : public C4DThread
{
public:
	virtual void Main()
	{
		Int32 cnt = GeGetCurrentThreadCount();
		MPThreadPool mp;
		maxon::BaseArray<C4DThread*> threadPointers;
		MPAlloc<MPTest> threads;

		if (!threadPointers.Resize(cnt) || !threads.Init(cnt))
		{
			_result = "Initialization failed";
			return;
		}

		for (Int32 i = 0; i < cnt; i++)
		{
			threads[i].SetStart(i);
			threadPointers[i] = &threads[i];
		}

		if (!mp.Init(*this, cnt, threadPointers.GetFirst()))
		{
			_result = "MP initialization failed";
			return;
		}

		Float64 start_time = GeGetMilliSeconds();
		if (!mp.Start(THREADPRIORITY_BELOW))
		{
			_result = "MP start failed";
			return;
		}

		Float64	start_duration = GeGetMilliSeconds() - start_time;
		mp.Wait();
		Float64 total_duration = GeGetMilliSeconds() - start_time;

		_result = "Multiprocessing Test on " + String::IntToString(cnt) + " CPUs returns:";
		for (Int32 i = 0; i < cnt; i++)
			_result += " " + String::FloatToString(threads[i].GetResult());

		_result += "| Duration per thread:";
		for (Int32 i = 0; i < cnt; i++)
		{
			_result += " " + String::FloatToString(threads[i].GetDuration()) + " ms";
			if (i + 1 < cnt)
				_result += ",";
		}

		_result += "|Thread start: " + String::FloatToString(start_duration) + " ms";
		_result += "|Total duration: " + String::FloatToString(total_duration) + " ms";
	}
	virtual const Char* GetThreadName(void) { return "SDK ControlThread"; }
	String GetResult() const { return _result; }

private:
	String _result;
};

class MenuTest : public CommandData
{
public:
	virtual Bool Execute(BaseDocument* doc);
};

Bool MenuTest::Execute(BaseDocument* doc)
{
	ControlThread ct;
	String msg;

	GeShowMouse(MOUSE_BUSY);
	if (ct.Start())
	{
		ct.Wait();
		msg = ct.GetResult();
	}
	else
		msg = "Thread start failed";
	GeShowMouse(MOUSE_NORMAL);

	MessageDialog(msg);

	return true;
}

Bool RegisterMenuTest(void)
{
	// be sure to use a unique ID obtained from www.plugincafe.com
	return RegisterCommandPlugin(1000956, GeLoadString(IDS_MENUTEST), 0, AutoBitmap("icon.tif"), String("C++ SDK Menu Test Plugin"), NewObjClear(MenuTest));
}

