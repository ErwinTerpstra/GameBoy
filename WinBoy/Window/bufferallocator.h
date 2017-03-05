#ifndef _BUFFER_ALLOCATOR_H_
#define _BUFFER_ALLOCATOR_H_

#include "buffer.h"

namespace WinBoy
{
	class BufferAllocator
	{

	public:
		BufferAllocator();

		virtual uchar* Allocate(const Buffer& buffer) = 0;
		virtual void Destroy() = 0;
	};
}

#endif