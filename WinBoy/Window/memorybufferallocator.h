#ifndef _MEMORY_BUFFER_ALLOCATOR_H_
#define _MEMORY_BUFFER_ALLOCATOR_H_

#include "bufferallocator.h"

namespace WinBoy
{

	class MemoryBufferAllocator : public BufferAllocator
	{
		
	public:
		

	private:
		uint8_t* data;

	public:
		MemoryBufferAllocator();
		~MemoryBufferAllocator();
		
		virtual uint8_t* Allocate(const Buffer& buffer);
		virtual void Destroy();

	};

}


#endif