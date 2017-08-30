#ifndef _CRC_H_
#define _CRC_H_

#include "ge_math.h"
#ifdef __API_INTERN__
	#include "ge_string.h"
#else
	#include "c4d_string.h"
#endif

class BaseList2D;

#define CRC32_NEGL 0xffffffff

// this table implements the generator polynom X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
// hex: 0xedb88320
// binary (leftmost bit represents x^0, rightmost bit is x^31, x^32 is implicitly 1), 1110 1101 1011 1000 1000 0011 0010 0000 (impl. 1)

class Crc32
{
protected:
	static const UInt32 crc32tab[256];

protected:
	UInt32 crc;

public:
	Crc32(void)
	{
		Reset();
	}

	void Set(UInt32 u)
	{
		crc = u ^ CRC32_NEGL;
	}

	void Reset(void)
	{
		crc = CRC32_NEGL;
	}

	void Update(const void *s, Int n)
	{
		register UInt32 c = crc;
		UChar* d = (UChar*)s;
		while (n--)
			c = crc32tab[UChar(c ^ *d++)] ^ (c >> 8);
		crc = c;
	}

	inline void UpdateByte(UChar b)
	{
		crc = crc32tab[UChar(crc ^ b)] ^ (crc >> 8);
	}

	inline void UpdateUInt32(UInt32 u)
	{
		register UInt32 c = crc;
		c = crc32tab[UChar(c ^ (u & 0xff))] ^ (c >> 8); u >>= 8;
		c = crc32tab[UChar(c ^ (u & 0xff))] ^ (c >> 8); u >>= 8;
		c = crc32tab[UChar(c ^ (u & 0xff))] ^ (c >> 8); u >>= 8;
		c = crc32tab[UChar(c ^ (u & 0xff))] ^ (c >> 8);
		crc = c;
	}

	inline void UpdateUInt64(UInt64 u)
	{
		register UInt32 c = crc;
		c = crc32tab[UChar(c ^ (u & 0xff))] ^ (c >> 8); u >>= 8;
		c = crc32tab[UChar(c ^ (u & 0xff))] ^ (c >> 8); u >>= 8;
		c = crc32tab[UChar(c ^ (u & 0xff))] ^ (c >> 8); u >>= 8;
		c = crc32tab[UChar(c ^ (u & 0xff))] ^ (c >> 8); u >>= 8;
		c = crc32tab[UChar(c ^ (u & 0xff))] ^ (c >> 8); u >>= 8;
		c = crc32tab[UChar(c ^ (u & 0xff))] ^ (c >> 8); u >>= 8;
		c = crc32tab[UChar(c ^ (u & 0xff))] ^ (c >> 8); u >>= 8;
		c = crc32tab[UChar(c ^ (u & 0xff))] ^ (c >> 8);
		crc = c;
	}

	inline void UpdateInt32(Int32 u)
	{
		register UInt32 n = (UInt32)u;
		register UInt32 c = crc;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8);
		crc = c;
	}

	inline void UpdateBool(Bool b)
	{
		register UInt32 n = (UInt32)b;
		register UInt32 c = crc;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8);
		crc = c;
	}

	inline void UpdateFloat32(Float32 r)
	{
		register UInt32 n = *((UInt32*)&r);
		register UInt32 c = crc;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8);
		crc = c;
	}

	inline void UpdateFloat64(Float64 r)
	{
		register UInt64 n = *((UInt64*)&r);
		register UInt32 c = crc;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8);
		crc = c;
	}

	inline void UpdateFloat(Float r)
	{
#ifdef __FLOAT_32_BIT
		register UInt32 n = *((UInt32*)&r);
		register UInt32 c = crc;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8);
		crc = c;
#else
		register UInt64 n = *((UInt64*)&r);
		register UInt32 c = crc;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8);
		crc = c;
#endif
	}

	inline void UpdatePointer(const void* p)
	{
#ifdef __C4D_64BIT
		register UInt64 n = (UInt64)p;
		register UInt32 c = crc;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8);
		crc = c;
#else
		register UInt32 n = (UInt32)p;
		register UInt32 c = crc;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8);
		crc = c;
#endif
	}

	inline void UpdateInt(Int p)
	{
#ifdef __C4D_64BIT
		register UInt64 n = (UInt64)p;
		register UInt32 c = crc;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8);
		crc = c;
#else
		register UInt32 n = (UInt32)p;
		register UInt32 c = crc;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8); n >>= 8;
		c = crc32tab[UChar(c ^ (n & 0xff))] ^ (c >> 8);
		crc = c;
#endif
	}

	void UpdateString(const String& str)
	{
		Int32 lLen = str.GetCStringLen(STRINGENCODING_UTF8);
		Char* pchString = str.GetCStringCopy(STRINGENCODING_UTF8);
		if (pchString)
		{
			Update(pchString, lLen);
			DeleteMem(pchString);
		}
	}

	UInt32 GetCrc(void)
	{
		return crc ^ CRC32_NEGL;
	}

	void UpdateObject(BaseList2D *bl);
};

#ifdef __PC
#define CRC64_NEGL 0xffffffffffffffff
#else
#define CRC64_NEGL 0xffffffffffffffffLLU
#endif

class Crc64
{
protected:
	static const UInt64 crc64tab[256];

protected:
	UInt64 crc;

public:
	Crc64(void);
	virtual void Reset(void);
	virtual void Update(const void *input, Int length);
	virtual void UpdateByte(UChar input);
	virtual UInt64 GetCrc(void);
};

#endif // _CRC_H_
