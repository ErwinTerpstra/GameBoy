#ifndef _MEMORY_POINTER_H_
#define _MEMORY_POINTER_H_

#include "environment.h"

namespace libdmg
{
	class Memory;
	class MemoryBank;

	class Pointer
	{
	public:
		virtual uint8_t Read() = 0;
		virtual void Write(uint8_t value) = 0;

		uint8_t operator *() { return Read(); }

		Pointer& operator=(int value)
		{
			Write(value);

			return *this;
		}
	};

	class NativePointer : public Pointer
	{
	private:
		uint8_t* ptr;

	public:
		NativePointer(uint8_t* ptr);

		uint8_t Read();
		void Write(uint8_t value);

		NativePointer& operator=(int value)
		{
			Pointer::operator=(value);
			return *this;
		}
	};

	class MemoryPointer : public Pointer
	{
	private:
		MemoryBank* memoryBank;
		uint16_t localAddress;

	public:
		MemoryPointer(Memory& memory, uint16_t address);
		
		uint8_t Read();
		void Write(uint8_t value);

		MemoryPointer& operator=(int value)
		{
			Pointer::operator=(value);
			return *this;
		}
	};
}

#endif