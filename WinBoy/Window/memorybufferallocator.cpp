#include "stdafx.h"
#include "memorybufferallocator.h"

using namespace WinBoy;

MemoryBufferAllocator::MemoryBufferAllocator() :  BufferAllocator(), data(NULL)
{

}

MemoryBufferAllocator::~MemoryBufferAllocator()
{

}

uint8_t* MemoryBufferAllocator::Allocate(const Buffer& buffer)
{	
	data = new uint8_t[buffer.size];
	
	return data;
}

void MemoryBufferAllocator::Destroy()
{
	if (data != NULL)
	{
		delete[] data;
		data = NULL;
	}
}