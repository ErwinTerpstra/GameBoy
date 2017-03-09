#include "stdafx.h"
#include "window.h"

#include "inputmanager.h"
#include "gdibufferallocator.h"

#include <Windowsx.h>
#include <assert.h>

using namespace WinBoy;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

Window::Window(HINSTANCE instance, const char* className, bool createClass) : instance(instance), className(className), inputManager(InputManager::Instance())
{
	if (createClass)
		RegisterClass();
}

Window::~Window()
{
	
}

void Window::RegisterClass() const
{
	WNDCLASSEX wc;

	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = instance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = className;
	wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wc);
}

void Window::Create(const char* title, int width, int height)
{
	int borderWidth = GetSystemMetrics(SM_CXBORDER);
	int borderHeight = GetSystemMetrics(SM_CYBORDER);
	int frameWidth = GetSystemMetrics(SM_CXFRAME);
	int frameHeight = GetSystemMetrics(SM_CYFRAME);
	int captionHeight = GetSystemMetrics(SM_CYSIZE);

	handle = CreateWindowEx(WS_EX_CLIENTEDGE, className, title, (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX),
			 			    CW_USEDEFAULT, CW_USEDEFAULT, width , height, NULL, NULL, instance, NULL);
	assert(handle != NULL);

	SetWindowLongPtrW(handle, GWLP_USERDATA, (long)this);

	SetClientSize(width, height);

	closed = false;
}

void Window::SetClientSize(int width, int height)
{
	RECT clientRect, windowRect;
	POINT delta;

	GetClientRect(handle, &clientRect);
	GetWindowRect(handle, &windowRect);

	delta.x = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
	delta.y = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);

	MoveWindow(handle, windowRect.left, windowRect.top, width + delta.x, height + delta.y, TRUE);

	this->width = width;
	this->height = height;
}

void Window::Show(int command) const
{
	ShowWindow(handle, command);
	UpdateWindow(handle);
}

void Window::ProcessMessages() const
{
	inputManager.Update();

	MSG msg;

	while (PeekMessage(&msg, handle, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK Window::MessageCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch(msg)
	{
		case WM_CLOSE:
			DestroyWindow(hWnd);
			closed = true;
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_MOUSEMOVE:
		{
			short x = GET_X_LPARAM(lParam);
			short y = GET_Y_LPARAM(lParam);
			inputManager.SetMousePosition(Point2(x, y));
			break;
		}
		
		case WM_LBUTTONDOWN:
			inputManager.KeyDown(InputManager::LEFT_MOUSE_BUTTON);
			break;

		case WM_LBUTTONUP:
			inputManager.KeyUp(InputManager::LEFT_MOUSE_BUTTON);
			break;

		case WM_KEYDOWN:
			inputManager.KeyDown(wParam);
			break;

		case WM_KEYUP:
			inputManager.KeyUp(wParam);
			break;

		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

void Window::DrawBuffer(const Buffer& buffer, const GDIBufferAllocator& allocator) const
{
	HDC windowDC = GetDC(handle);
	HDC bufferDC = CreateCompatibleDC(windowDC);
	HGDIOBJ oldObj = SelectObject(bufferDC, allocator.bitmap);
			
	BOOL result = BitBlt(windowDC, 0, 0, buffer.width, buffer.height, bufferDC, 0, 0, SRCCOPY);
	assert(result);
			
	SelectObject(bufferDC, oldObj);
	DeleteDC(bufferDC);
	ReleaseDC(handle, windowDC);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    long userData = GetWindowLongW(hWnd, GWLP_USERDATA);
	Window* window = reinterpret_cast<Window*>(userData);

    return window->MessageCallback(hWnd, msg, wParam, lParam);
}