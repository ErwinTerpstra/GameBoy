#ifndef _WINDOW_H_
#define _WINDOW_H_

namespace WinBoy
{
	class Buffer;
	class GDIBufferAllocator;
	class InputManager;

	class Window
	{

	public:
		
		enum Extensions
		{
			WINDOW_GL
		};

		HWND handle;
		
		bool closed;

		int width;
		int height;

	private:
		HINSTANCE instance;

		InputManager& inputManager;

		const char* className;

	public:

		Window(HINSTANCE instance, const char* className, bool createClass = true);
		~Window();
		
		void Create(const char* title, int width, int height);
		void Show(int command) const;
		void SetClientSize(int width, int height);
		
		void ProcessMessages() const;
		
		void DrawBuffer(const Buffer& buffer, const GDIBufferAllocator& allocator) const;

		LRESULT CALLBACK MessageCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		
		void RegisterClass() const;



	};

}


#endif