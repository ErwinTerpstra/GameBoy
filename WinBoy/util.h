#ifndef _UTIL_H_
#define _UTIL_H_

#include "WinBoy.h"

namespace WinBoy
{

	class Util
	{

	public:
		template <typename T>
		WB_FORCE_INLINE static void Sort(T& a, T& b)
		{
			if (a > b)
				Swap(a, b);
		}

		template <typename T>
		WB_FORCE_INLINE static void Swap(T& a, T& b)
		{
			T tmp = a;
			a = b;
			b = tmp;
		}


		template <typename T>
		WB_FORCE_INLINE static void Swap(T** a, T** b)
		{
			T* tmp = *a;
			*a = *b;
			*b = tmp;
		}

		template <typename T>
		WB_FORCE_INLINE static int32_t Sign(T val)
		{
			return (T(0) < val) - (val < T(0));
		}

		template <typename T>
		WB_FORCE_INLINE static T Clamp(T val, T min, T max)
		{
			return std::min(std::max(val, min), max);
		}

		WB_FORCE_INLINE static float Clamp01(float val)
		{
			return Clamp(val, 0.0f, 1.0f);
		}
	};
}

#endif