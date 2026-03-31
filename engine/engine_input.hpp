#pragma once
#include "engine_common.hpp"
#include "raylib.h"
#include <unordered_map>			// TODO: USE C OBJECTSSS
#include <vector>					// 

using GameActionFlags = std::uint32_t;
using GameActionFlag = std::uint32_t;
using GameAction = std::uint8_t;
constexpr GameActionFlags NULL_ACTION_FLAGS = 0;

GameActionFlag GameActionFlag_FromAction(GameAction act);
bool GameActionFlags_Get(GameActionFlags flags, GameActionFlag flag);
void GameActionFlags_Set(GameActionFlags* flags, GameActionFlag flag, bool to=true);

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

struct EngineInputConfig {
	std::unordered_map<GameActionFlag, std::vector<InputKey>> bindings;
};
void EngineInput_Create(EngineInputConfig* bindings);
void EngineInput_SetBinding(EngineInputConfig* bindings, GameActionFlag action, InputKey bind_to);
void EngineInput_FetchState(const EngineInputConfig* bindings, GameActionFlags* out_flags_pressed, GameActionFlags* out_flags_held, int* out_mouse_x, int* out_mouse_y);
void EngineInput_Destroy(EngineInputConfig* bindings);
