#include "stdafx.h"

#include "gdibufferallocator.h"

#include "debug.h"

using namespace WinBoy;

GDIBufferAllocator::GDIBufferAllocator(HWND windowHandle) : BufferAllocator(), windowHandle(windowHandle)
{

}

GDIBufferAllocator::~GDIBufferAllocator()
{

}

uint8_t* GDIBufferAllocator::Allocate(const Buffer& buffer)
{
	// Only BGR24 encoding is supporte for the GDI buffer
	assert(buffer.encoding == Buffer::BGR24);

	bitmapInfo.bmiColors[0].rgbRed		= 255;
	bitmapInfo.bmiColors[0].rgbGreen	= 255;
	bitmapInfo.bmiColors[0].rgbBlue		= 255;

	BITMAPINFOHEADER& header = bitmapInfo.bmiHeader;
	header.biWidth = buffer.width;
	header.biHeight = buffer.height;
	header.biPlanes = 1;
	header.biBitCount = buffer.bpp;
	header.biCompression = BI_RGB;
	header.biSize = sizeof(BITMAPINFOHEADER);
	header.biSizeImage = buffer.size;
	header.biClrImportant = 0;
	header.biClrUsed = 0;
	
	HDC windowDC = GetDC(windowHandle);

	void* bufferBase = NULL;
	bitmap = CreateDIBSection(windowDC, &bitmapInfo, DIB_RGB_COLORS, &bufferBase, NULL, 0);

	uint8_t* data = static_cast<uint8_t*>(bufferBase);

	ReleaseDC(windowHandle, windowDC);

	return data;
}

void GDIBufferAllocator::Destroy()
{
	DeleteObject(bitmap);
}