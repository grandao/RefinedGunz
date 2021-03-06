#pragma once
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <cassert>

#pragma warning(push)
#pragma warning(disable:4996)

template <bool b, size_t size1, size_t size2>
struct strcpy_literal_dummy
{
	static inline char* strcpy_literal_impl(char (&Dest)[size1], const char (&Source)[size2]);
};

template <size_t size1, size_t size2>
struct strcpy_literal_dummy<true, size1, size2>
{
	static inline char* strcpy_literal_impl(char(&Dest)[size1], const char(&Source)[size2])
	{
		memcpy(Dest, Source, size2);

		return Dest + size2 - 1;
	}
};

template <size_t size1, size_t size2>
inline char* strcpy_literal(char(&Dest)[size1], const char(&Source)[size2])
{
	return strcpy_literal_dummy<(size2 < size1), size1, size2>::strcpy_literal_impl(Dest, Source);
}

inline char* strcpy_trunc(char* Dest, size_t size, const char* Source)
{
	auto len = strlen(Source);

	if (size - 1 < len)
	{
		len = size - 1;
	}

	memcpy(Dest, Source, len);

	Dest[len] = 0;

	return Dest + len;
}

template <size_t size>
inline char* strcpy_trunc(char(&Dest)[size], const char *Source)
{
	return strcpy_trunc(Dest, size, Source);
}

inline char* strcpy_safe(char *Dest, size_t size, const char *Source)
{
	auto len = strlen(Source);

	if (size - 1 < len && size > 0)
	{
		len = size - 1;
		assert(false);
	}

	memcpy(Dest, Source, len);

	Dest[len] = 0;

	return Dest + len;
}

template <size_t size>
inline char* strcpy_safe(char(&Dest)[size], const char *Source)
{
	return strcpy_safe(Dest, size, Source);
}

inline char* strncpy_safe(char *Dest, size_t size, const char *Source, size_t Count)
{
	auto len = Count;

	if (size - 1 < len)
	{
		len = size - 1;
	}

	memcpy(Dest, Source, len);

	Dest[len] = 0;

	return Dest + len;
}

template <size_t size>
inline char* strncpy_safe(char(&Dest)[size], const char *Source, size_t Count)
{
	return strncpy_safe(Dest, size, Source, Count);
}

template <size_t size>
inline char* strcat_safe(char(&Dest)[size], const char *Source)
{
	auto DestLen = strlen(Dest);
	auto SourceLen = strlen(Source);

	auto BytesToCopy = SourceLen;
	if (DestLen + BytesToCopy + 1 > size)
	{
		BytesToCopy = size - 1 - DestLen;
	}

	memcpy(Dest + DestLen, Source, BytesToCopy);

	Dest[DestLen + BytesToCopy] = 0;

	return Dest + DestLen + BytesToCopy;
}

inline char* strcat_safe(char *Dest, size_t size, const char *Source)
{
	auto DestLen = strlen(Dest);
	auto SourceLen = strlen(Source);

	auto BytesToCopy = SourceLen;
	if (DestLen + BytesToCopy + 1 > size && size > 0)
	{
		BytesToCopy = size - 1 - DestLen;
	}

	memcpy(Dest + DestLen, Source, BytesToCopy);

	Dest[DestLen + BytesToCopy] = 0;

	return Dest + DestLen + BytesToCopy;
}

template <size_t size>
inline char* strncat_safe(char(&Dest)[size], const char* Source, size_t Count)
{
	auto DestLen = strlen(Dest);
	auto SourceLen = Count;

	auto BytesToCopy = SourceLen;
	if (DestLen + BytesToCopy + 1 > size)
	{
		BytesToCopy = size - 1 - DestLen;
	}

	memcpy(Dest + DestLen, Source, BytesToCopy);

	Dest[DestLen + BytesToCopy] = 0;

	return Dest + DestLen + BytesToCopy;
}

inline char* strncat_safe(char* Dest, size_t size, const char* Source, size_t Count)
{
	auto DestLen = strlen(Dest);
	auto SourceLen = Count;

	auto BytesToCopy = SourceLen;
	if (DestLen + BytesToCopy + 1 > size)
	{
		BytesToCopy = size - 1 - DestLen;
	}

	memcpy(Dest + DestLen, Source, BytesToCopy);

	Dest[DestLen + BytesToCopy] = 0;

	return Dest + DestLen + BytesToCopy;
}

template<size_t size>
inline int sprintf_safe(char(&Dest)[size], const char *Format, ...)
{
	va_list args;

	va_start(args, Format);
	int ret = vsnprintf(Dest, size, Format, args);
	va_end(args);

	Dest[size - 1] = 0;

	return ret;
}

inline int sprintf_safe(char *Dest, int size, const char *Format, ...)
{
	va_list args;

	va_start(args, Format);
	int ret = vsnprintf(Dest, size, Format, args);
	va_end(args);

	return ret;
}

template <size_t size>
inline int vsprintf_safe(char (&Dest)[size], const char* Format, va_list va)
{
	return vsnprintf(Dest, size, Format, va);
}

template <size_t size>
void itoa_safe(int val, char(&dest)[size], int radix)
{
	if (radix == 10)
		sprintf_safe(dest, "%d", val);
	else if (radix == 16)
		sprintf_safe(dest, "%x", val);
	else
		assert(false);
}

inline auto strcpy_unsafe(char* a, const char* b) {
	return strcpy(a, b);
}

#pragma warning(pop)