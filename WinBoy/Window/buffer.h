#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "WinBoy.h"

namespace WinBoy
{
	class BufferAllocator;
	struct Color;

	// TODO: Make this a generic buffer (void*) and move the "byte" functionality to a ByteBufer subclass
	// Also split the color functionality to a ColorBuffer class
	class Buffer
	{

	public:
		enum Encoding
		{
			RGB24, RGBA32, BGR24, BGRA32, FLOAT32, FLOAT96, FLOAT128
		};

		uint32_t width, height, stride, size;
		uint8_t bpp, pixelStride, alignment;

		Encoding encoding;

		uint8_t* data;

	private:
		BufferAllocator* allocator;

	public:

		Buffer(BufferAllocator* allocator);
		virtual ~Buffer();

		const BufferAllocator& GetAllocator() const;

		void Allocate(uint32_t preferredWidth, uint32_t preferredHeight, Encoding encoding);
		void AllocateAligned(uint32_t preferredWidth, uint32_t preferredHeight, uint8_t alignment, Encoding encoding);

		uint32_t CalculateStride(uint32_t width, uint8_t bitDepth, uint8_t alignment);
		void Destroy();
		
		__inline void Clear() { memset(data, 0, size); }
		void Clear(const Color& color);
		
		void Blit(const Buffer& src);

		float GetPixel(uint32_t x, uint32_t y) const;
		void GetPixel(uint32_t x, uint32_t y, Color& color) const;

		void SetPixel(uint32_t x, uint32_t y, const Color& color);
		void SetPixel(uint32_t x, uint32_t y, const uint8_t* buffer);
		void SetPixel(uint32_t x, uint32_t y, float f);
		
		__inline uint8_t* GetBase(uint32_t x, uint32_t y) const
		{ 
			return data + y * stride + x * pixelStride;
		}

		static uint8_t GetEncodingDepth(Encoding encoding);

		static void EncodeColor(const Color& color, Encoding encoding, uchar* buffer);
		static void DecodeColor(const uchar* buffer, Encoding encoding, Color& color);

	};

}

#endif