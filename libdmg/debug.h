#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "environment.h"
#include <cstdio>
#include <stdarg.h>

namespace libdmg
{

	class Debug
	{

	private:

		
	public:
		

		static void Print(const char* msg, ...)
		{
			va_list args;
			va_start(args, msg);

			vprintf(msg, args);
			va_end(args);
		}

		static bool AssertHandler(const char* code, const char* file, const uint32_t line)
		{
			Print("Assert failed!\n%s at %s:%d\n", code, file, line);
			return true;
		}

		static bool Halt()
		{
			__debugbreak();
			return true;
		}

		static void Break()
		{
			__debugbreak();
		}
	};

#ifdef DMG_DEBUG
#define assert(x) ((void)(!(x) && libdmg::Debug::AssertHandler(#x, __FILE__, __LINE__) && libdmg::Debug::Halt()))
#else
#define assert(x)
#endif

}

#endif