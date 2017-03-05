#ifndef _INPUT_MANAGER_H_
#define _INPUT_MANAGER_H_

namespace WinBoy
{
	struct Point2
	{
		int x, y;

		Point2() : x(0), y(0) { }
		Point2(int x, int y) : x(x), y(y) { }
	};

	class InputManager
	{

	public:
		static const uint32_t MAX_KEYS = 512;

		enum Keys
		{
			LEFT_MOUSE_BUTTON = 256
		};

	private:
		bool keys[MAX_KEYS];
		bool prevKeys[MAX_KEYS];
		
		Point2 mousePosition;

		static InputManager* instance;

	public:
		void Update();

		bool GetKey(uint16_t key) const;
		bool GetKeyDown(uint16_t key) const;
		bool GetKeyUp(uint16_t key) const;

		void KeyUp(uint16_t key);
		void KeyDown(uint16_t key);

		void SetMousePosition(const Point2& position);
		Point2 GetMousePosition() const;

		static InputManager& Instance();

	private:
		InputManager();

	};

}

#endif