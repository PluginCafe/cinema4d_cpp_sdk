

#include "c4d_library.h"

// INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF
// INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF
// INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF

#define LIBRARY_COFFEE 1000477


struct PrivateCoffeeLib : public C4DLibrary
{
	Bool (*CompileCoffeeFile)(const Filename &file,Filename *res);
};

// INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF
// INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF
// INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF -- INTERNAL STUFF


static PrivateCoffeeLib *lib = nullptr;


static PrivateCoffeeLib *CheckLib(Int32 offset)
{
	return (PrivateCoffeeLib*)CheckLib(LIBRARY_COFFEE,offset,(C4DLibrary**)&lib);
}


Bool CompileCoffeeFile(const Filename &file,Filename *res)
{
	PrivateCoffeeLib *lib = CheckLib(LIBOFFSET(PrivateCoffeeLib,CompileCoffeeFile));
	if (!lib || !lib->CompileCoffeeFile) return false;

	return (*lib->CompileCoffeeFile)(file,res);
}




