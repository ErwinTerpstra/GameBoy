#include "stdafx.h"
#include <memory.h>

#include "debug.h"

#include "buffer.h"
#include "bufferallocator.h"

#include "color.h"
#include "util.h"

using namespace WinBoy;

Buffer::Buffer(BufferAllocator* allocator) : allocator(allocator), data(NULL)
{
	
}

Buffer::~Buffer()
{
	Destroy();
}

const BufferAllocator& Buffer::GetAllocator() const
{
	return *allocator;
}

void Buffer::Allocate(uint32_t preferredWidth, uint32_t preferredHeight, Encoding encoding)
{
	AllocateAligned(preferredWidth, preferredHeight, 1, encoding);
}

void Buffer::AllocateAligned(uint32_t preferredWidth, uint32_t preferredHeight, uint8_t alignment, Encoding encoding)
{
	this->encoding = encoding;
	this->alignment = alignment;
	
	this->bpp = GetEncodingDepth(encoding);
	this->pixelStride = this->bpp / 8;

	this->width = preferredWidth;
	this->height = preferredHeight;

	this->stride = CalculateStride(preferredWidth, bpp, alignment);
	this->size = height * stride;

	data = allocator->Allocate(*this);
}

uint32_t Buffer::CalculateStride(uint32_t width, uint8_t bitDepth, uint8_t alignment)
{
	uint32_t stride = width * (bitDepth / 8);
	return (uint32_t) (ceil(stride / (float)alignment) * alignment);
}

void Buffer::Destroy()
{
	if (data != NULL)
	{
		allocator->Destroy();

		data = NULL;
	}
}

void Buffer::Clear(const Color& color)
{
	for (uint8_t y = 0; y < height; ++y)
		for (uint8_t x = 0; x < width; ++x)
			SetPixel(x, y, color);
}

void Buffer::Blit(const Buffer& src)
{
	Color color;

	for (uint32_t y = 0; y < std::min(height, src.height); ++y)
	{
		for (uint32_t x = 0; x < std::min(width, src.width); ++x)
		{
			src.GetPixel(x, y, color);
			SetPixel(x, y, color);
		}
	}

}

float Buffer::GetPixel(uint32_t x, uint32_t y) const
{
	assert(bpp == sizeof(float) * 8);

	return *reinterpret_cast<float*>(GetBase(x, y));
}

void Buffer::GetPixel(uint32_t x, uint32_t y, Color& color) const
{
	uint8_t* pixelBase = GetBase(x, y);
	DecodeColor(pixelBase, encoding, color);
}

void Buffer::SetPixel(uint32_t x, uint32_t y, float f)
{
	assert(bpp == sizeof(float) * 8);

	memcpy(GetBase(x, y), reinterpret_cast<uint8_t*>(&f), sizeof(float));
}

void Buffer::SetPixel(uint32_t x, uint32_t y, const Color& color)
{
	uint8_t* pixelBase = GetBase(x, y);
	EncodeColor(color, encoding, pixelBase);
}

void Buffer::SetPixel(uint32_t x, uint32_t y, const uint8_t* buffer)
{
	uint8_t* pixelBase = GetBase(x, y);
	
	// Copy data from the input to the buffer
	memcpy(pixelBase, buffer, bpp / 8);
}


uint8_t Buffer::GetEncodingDepth(Encoding encoding)
{
	switch (encoding)
	{
	case RGB24:			return 24;
	case RGBA32:		return 32;
	case BGR24:			return 24;
	case BGRA32:		return 32;
	case FLOAT32:		return 32;
	case FLOAT96:		return 96;
	case FLOAT128:		return 128;
	}

	assert(false && "Encoding not supported.");

	return 0;
}

void Buffer::EncodeColor(const Color& color, Encoding encoding, uchar* buffer)
{
	switch (encoding)
	{
	case RGB24:
		buffer[0] = (uchar)(Util::Clamp01(color[0]) * 255.0f);
		buffer[1] = (uchar)(Util::Clamp01(color[1]) * 255.0f);
		buffer[2] = (uchar)(Util::Clamp01(color[2]) * 255.0f);
		break;

	case RGBA32:
		buffer[0] = (uchar)(Util::Clamp01(color[0]) * 255.0f);
		buffer[1] = (uchar)(Util::Clamp01(color[1]) * 255.0f);
		buffer[2] = (uchar)(Util::Clamp01(color[2]) * 255.0f);
		buffer[3] = (uchar)(Util::Clamp01(color[3]) * 255.0f);
		break;

	case BGR24:
		buffer[0] = (uchar)(Util::Clamp01(color[2]) * 255.0f);
		buffer[1] = (uchar)(Util::Clamp01(color[1]) * 255.0f);
		buffer[2] = (uchar)(Util::Clamp01(color[0]) * 255.0f);
		break;

	case BGRA32:
		buffer[0] = (uchar)(Util::Clamp01(color[2]) * 255.0f);
		buffer[1] = (uchar)(Util::Clamp01(color[1]) * 255.0f);
		buffer[2] = (uchar)(Util::Clamp01(color[0]) * 255.0f);
		buffer[3] = (uchar)(Util::Clamp01(color[3]) * 255.0f);
		break;

	case FLOAT96:
	{
		float* floatBuffer = reinterpret_cast<float*>(buffer);
		floatBuffer[0] = color[0];
		floatBuffer[1] = color[1];
		floatBuffer[2] = color[2];
		break;
	}

	case FLOAT128:
	{
		float* floatBuffer = reinterpret_cast<float*>(buffer);
		floatBuffer[0] = color[0];
		floatBuffer[1] = color[1];
		floatBuffer[2] = color[2];
		floatBuffer[3] = color[3];
		break;
	}

	default:
		assert(false && "Encoding does not support color writing.");
		break;
	}

}

void Buffer::DecodeColor(const uchar* buffer, Encoding encoding, Color& color)
{
	switch (encoding)
	{
	case RGB24:
		color[0] = buffer[0] / 255.0f;
		color[1] = buffer[1] / 255.0f;
		color[2] = buffer[2] / 255.0f;
		color[3] = 1.0f;
		break;

	case RGBA32:
		color[0] = buffer[0] / 255.0f;
		color[1] = buffer[1] / 255.0f;
		color[2] = buffer[2] / 255.0f;
		color[3] = buffer[3] / 255.0f;
		break;

	case BGR24:
		color[0] = buffer[2] / 255.0f;
		color[1] = buffer[1] / 255.0f;
		color[2] = buffer[0] / 255.0f;
		color[3] = 1.0f;
		break;

	case BGRA32:
		color[0] = buffer[2] / 255.0f;
		color[1] = buffer[1] / 255.0f;
		color[2] = buffer[0] / 255.0f;
		color[3] = buffer[3] / 255.0f;
		break;

	case FLOAT96:
	{
		const float* floatBuffer = reinterpret_cast<const float*>(buffer);
		color[0] = floatBuffer[0];
		color[1] = floatBuffer[1];
		color[2] = floatBuffer[2];
		break;
	}

	case FLOAT128:
	{
		const float* floatBuffer = reinterpret_cast<const float*>(buffer);
		color[0] = floatBuffer[0];
		color[1] = floatBuffer[1];
		color[2] = floatBuffer[2];
		color[3] = floatBuffer[3];
		break;
	}

	default:
		assert(false && "Encoding does not support color reading.");
		break;
	}

}