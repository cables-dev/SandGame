#include "engine_input.hpp"

GameActionFlag GameActionFlag_FromAction(GameAction act) {
	return (GameActionFlag)(1ull << act);
}

bool GameActionFlags_Get(GameActionFlags flags, GameActionFlag flag) {
	return flags & flag;
}

void GameActionFlags_Set(GameActionFlags* flags, GameActionFlag flag, bool to) {
	if (to) {
		*flags |= flag;
	}
	else {
		*flags &= ~flag;
	}
}

void EngineInput_Create(EngineInputConfig* bindings) {
	// nop
}

void EngineInput_SetBinding(EngineInputConfig* bindings, GameActionFlag action, InputKey bind_to) {
	if (bindings->bindings.find(action) != bindings->bindings.end())
		bindings->bindings[action].push_back(bind_to);
	bindings->bindings.insert({ action, {bind_to} });
}

bool Input_IsMouseButton(InputKey key) {
	return key & KEY_MOUSE_BUTTON_FLAG;
}

InputKey Input_GetRaylibKeyId(InputKey key) {
	return (InputKey)((int)key & ~KEY_MOUSE_BUTTON_FLAG);
}

bool Input_GetInputKeyPressed(InputKey key) {
	auto raylib = Input_GetRaylibKeyId(key);
	if (Input_IsMouseButton(key)) {
		return IsMouseButtonPressed(raylib);
	}
	else {
		return IsKeyPressed(raylib);
	}
}

bool Input_GetInputKeyHeld(InputKey key) {
	auto raylib = Input_GetRaylibKeyId(key);
	if (Input_IsMouseButton(key)) {
		return IsMouseButtonDown(raylib);
	}
	else {
		return IsKeyDown(raylib);
	}
}

void EngineInput_FetchState(
	const EngineInputConfig* bindings, 
	GameActionFlags* out_flags_pressed, 
	GameActionFlags* out_flags_held, 
	int* out_mouse_x, 
	int* out_mouse_y
) {
	for (auto& [action_flag, keys] : bindings->bindings) {
		auto pressed = false;
		auto held = false;
		for (auto& key : keys) {
			pressed |= Input_GetInputKeyPressed(key);
			held |= Input_GetInputKeyHeld(key);
		}
		GameActionFlags_Set(out_flags_pressed, action_flag, pressed);
		GameActionFlags_Set(out_flags_held, action_flag, held);
	}
	*out_mouse_x = GetMouseX();
	*out_mouse_y = GetMouseY();
}

void EngineInput_Destroy(EngineInputConfig* bindings) {
	// nop
}

