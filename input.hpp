#pragma once
#include "common.hpp"
#include "raylib.h"
#include <unordered_map>			// TODO: migrate to c object

constexpr static auto KEY_MOUSE_BUTTON_FLAG = 1 << 16;

using InputKey = int; 	// Defer to raylib definitions in KeyboardKey,
enum InputMouseKey {						
	// except for mouse buttons which use different functions...
	KEY_MOUSE_LEFT_BUTTON = KEY_MOUSE_BUTTON_FLAG | MOUSE_BUTTON_LEFT,	
	KEY_MOUSE_RIGHT_BUTTON = KEY_MOUSE_BUTTON_FLAG | MOUSE_BUTTON_RIGHT,
	KEY_MOUSE_MIDDLE_BUTTON = KEY_MOUSE_BUTTON_FLAG | MOUSE_BUTTON_MIDDLE,
	KEY_MOUSE_SIDE_BUTTON = KEY_MOUSE_BUTTON_FLAG | MOUSE_BUTTON_SIDE,
	KEY_MOUSE_EXTRA_BUTTON = KEY_MOUSE_BUTTON_FLAG | MOUSE_BUTTON_EXTRA,
	KEY_MOUSE_FORWARD_BUTTON = KEY_MOUSE_BUTTON_FLAG | MOUSE_BUTTON_FORWARD,
	KEY_MOUSE_BACK_BUTTON = KEY_MOUSE_BUTTON_FLAG | MOUSE_BUTTON_BACK,
};			


struct KeyBindings {
	std::unordered_map<GameAction, InputKey> bindings;
};

void Input_Create(KeyBindings* bindings);
void Input_SetBinding(KeyBindings* bindings, GameAction action, InputKey bind_to);
void Input_FetchState(const KeyBindings* bindings, GameActionFlags* out_flags_pressed, GameActionFlags* out_flags_held, int* out_mouse_x, int* out_mouse_y);
void Input_Destroy(KeyBindings* bindings);
