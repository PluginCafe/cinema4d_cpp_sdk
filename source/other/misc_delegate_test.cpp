/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) MAXON Computer GmbH, all rights reserved            //
/////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_misc.h"
#include "main.h"

namespace maxon
{
	static Result<void> CallDelegates();
	static Result<void> CallLambdaDelegates();
	static Result<void> CallByReferenceDelegates();
}

void MiscDelegateTest()
{
	maxon::CallDelegates();
	maxon::CallLambdaDelegates();
	maxon::CallByReferenceDelegates();
}

namespace maxon
{

class SampleClass
{
public:
	virtual ~SampleClass()
	{
	}

	static void MethodByValue(const Char* str)
	{
		DiagnosticOutput("@", str);
	}

	static void MethodByLValueReference(const Char*& str)
	{
		DiagnosticOutput("@", str);
	}

	static void MethodByRValueReference(Char*&& str)
	{
		DiagnosticOutput("@", str);
	}

	static void TestWithoutError()
	{
		DiagnosticOutput("static Test()");
	};

	static Result<void> Test()
	{
		DiagnosticOutput("static Test()");
		return OK;
	};

	virtual Result<void> operator ()()
	{
		DiagnosticOutput("virtual operator ()()");
		return OK;
	}
	virtual const Char* GetName() const
	{
		DiagnosticOutput("virtual GetName()");
		return "SampleClass";
	}
};

static Result<void> CallDelegates()
{
	AutoPtr<SampleClass> sample(NewObj(SampleClass));
	iferr (sample)
		return OutOfMemoryError(CREATE);

	// Assign a virtual method pointer (to a const method).
	Delegate<const Char*()> fn(sample.Get(), &SampleClass::GetName);
	fn();

	fn = Delegate<const Char*()>::CreateByReference<SampleClass, &SampleClass::GetName>(sample);
	fn();

	// Assign a static function pointer
	Delegate<void()> fn2(&SampleClass::TestWithoutError);
	fn2();

	fn2 = Delegate<void()>::Create<&SampleClass::TestWithoutError>();
	fn2();

	fn2 = &SampleClass::TestWithoutError;
	fn2();

	Int x = 5;

	auto test3 = [x](Int a) -> Result<void>
	{
		if (a == 42)
			return OK;

		if (x == 0)
			return OK;

		// Just for the sake of returning an error ...
		return OutOfMemoryError(CREATE);
	};
	Delegate<Result<void>(Int)> fn3(test3);
	iferr (fn3(42))
		DebugStop("Method returned an error");

	// Assign a virtual method pointer (method is not const).
	Delegate<Result<void>()> fn4 = Delegate<Result<void>()>::CreateByReference<SampleClass, &SampleClass::operator()>(sample);
	iferr (fn4())
		DebugStop("Method returned an error");

	return OK;
}

static Result<void> CallLambdaDelegates()
{
	// Parameter by value.
	auto lambdaByValue =
		[](const Char* str)
		{
			DiagnosticOutput(str);
		};
	Delegate<void(const Char*)> fn5(lambdaByValue);
	if (fn5.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	fn5("Lambda ByValue constructor");

	if (Delegate<void(const Char*)>::Create(lambdaByValue, fn5) == FAILED)
		return OutOfMemoryError(CREATE);
	if (fn5.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	fn5("Lambda ByValue From");

	fn5 = lambdaByValue;
	if (fn5.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	fn5("Lambda ByValue assignment");

	// Parameter by lvalue reference.
	auto lambdaByLValueReference =
		[](const Char*& str)
		{
			DiagnosticOutput(str);
		};
	Delegate<void(const Char*&)> fn6(lambdaByLValueReference);
	if (fn6.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	const Char* sampleFunctionByReferenceStr = "Lambda ByLValueReference constructor";
	fn6(sampleFunctionByReferenceStr);

	iferr (Delegate<void(const Char*&)>::Create(lambdaByLValueReference, fn6))
		return OutOfMemoryError(CREATE);
	if (fn6.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	sampleFunctionByReferenceStr = "Lambda ByLValueReference From";
	fn6(sampleFunctionByReferenceStr);

	fn6 = lambdaByLValueReference;
	if (fn6.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	sampleFunctionByReferenceStr = "Lambda ByLValueReference assignment";
	fn6(sampleFunctionByReferenceStr);

	// Parameter by rvalue reference.
	auto lambdaByRValueReference =
		[](Char*&& str)
		{
			DiagnosticOutput("@", str);
		};
	Delegate<void(Char*&&)> fn7(lambdaByRValueReference);
	if (fn7.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	Char* sampleFunctionMoveStr = const_cast<Char*>("Lambda ByRValueReference constructor");
	fn7(std::move(sampleFunctionMoveStr));

	iferr (Delegate<void(Char*&&)>::Create(lambdaByRValueReference, fn7))
		return OutOfMemoryError(CREATE);
	if (fn7.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	sampleFunctionMoveStr = const_cast<Char*>("Lambda ByRValueReference From");
	fn7(std::move(sampleFunctionMoveStr));

	fn7 = lambdaByRValueReference;
	if (fn7.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	sampleFunctionMoveStr = const_cast<Char*>("Lambda ByRValueReference assignment");
	fn7(std::move(sampleFunctionMoveStr));


	return OK;
}

static Result<void> CallByReferenceDelegates()
{
	// Parameter by value.
	auto lambdaByValue =
		[](const Char* str)
		{
			DiagnosticOutput(str);
		};

	auto fn5 = Delegate<void(const Char*)>::CreateByReference(lambdaByValue);
	if (fn5.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	fn5("Lambda ByValue CreateByReference");

	// Parameter by lvalue reference.
	auto lambdaByLValueReference =
		[](const Char*& str)
		{
			DiagnosticOutput("@", str);
		};
	auto fn6 = Delegate<void(const Char*&)>::CreateByReference(lambdaByLValueReference);
	if (fn6.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	const Char* sampleFunctionByReferenceStr = "Lambda ByLValueReference CreateByReference";
	fn6(sampleFunctionByReferenceStr);

	// Parameter by rvalue reference.
	auto lambdaByRValueReference =
		[](Char*&& str)
		{
			DiagnosticOutput("@", str);
		};
	auto fn7 = Delegate<void(Char*&&)>::CreateByReference(lambdaByRValueReference);
	if (fn7.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	Char* sampleFunctionMoveStr = const_cast<Char*>("Lambda ByRValueReference CreateByReference");
	fn7(std::move(sampleFunctionMoveStr));


	// Assign a virtual method pointer (to a const method).
	AutoPtr<SampleClass> sample(NewObj(SampleClass));
	iferr (sample)
		return OutOfMemoryError(CREATE);
	auto fn8 = Delegate<const Char*()>::CreateByReference<SampleClass, &SampleClass::GetName>(sample);
	if (fn8.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	fn8();

	fn8 = Delegate<const Char*()>::CreateByReference(sample.Get(), &SampleClass::GetName);
	if (fn8.IsStaticFunctionPointer() == true)
		DebugStop("IllegalStateError(CREATE)");
	fn8();

	return OK;
}


}
