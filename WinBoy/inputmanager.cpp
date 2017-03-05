#include "stdafx.h"
#include "inputmanager.h"
#include "debug.h"

using namespace WinBoy;

InputManager* InputManager::instance = NULL;

InputManager::InputManager()
{
	for (uint32_t i = 0; i < MAX_KEYS; ++i)
	{
		keys[i] = false;
		prevKeys[i] = false;
	}
}

void InputManager::Update()
{
	for (uint32_t i = 0; i < MAX_KEYS; ++i)
		prevKeys[i] = keys[i];
}

bool InputManager::GetKey(uint16_t key) const
{
	assert(key >= 0 && key < MAX_KEYS);
	return keys[key];
}

bool InputManager::GetKeyDown(uint16_t key) const
{
	assert(key >= 0 && key < MAX_KEYS);
	return keys[key] && !prevKeys[key];
}

bool InputManager::GetKeyUp(uint16_t key) const
{
	assert(key >= 0 && key < MAX_KEYS);
	return !keys[key] && prevKeys[key];
}

void InputManager::KeyDown(uint16_t key)
{
	keys[key] = true;
}

void InputManager::KeyUp(uint16_t key)
{
	keys[key] = false;
}

void InputManager::SetMousePosition(const Point2& position)
{
	mousePosition = position;
}

Point2 InputManager::GetMousePosition() const
{
	return mousePosition;
}

InputManager& InputManager::Instance()	
{
	if (instance == NULL)
		instance = new InputManager();

	return *instance;
}