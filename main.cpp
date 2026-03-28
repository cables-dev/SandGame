// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "common.hpp"
#include "input.hpp"
#include "game.hpp"
#include "render.hpp"
#include "audio.hpp"
#include "level.hpp"
#include <cassert>

void DefaultKeyBindings(KeyBindings* bindings) {
	Input_Create(bindings);
	Input_SetBinding(bindings, ACTION_MOVE_LEFT, KEY_A);				
	Input_SetBinding(bindings, ACTION_MOVE_RIGHT, KEY_D);
	Input_SetBinding(bindings, ACTION_JUMP, KEY_SPACE);
	Input_SetBinding(bindings, ACTION_FIRE, KEY_W);
	Input_SetBinding(bindings, ACTION_SWITCH_FIRE_MODE, KEY_LEFT_CONTROL);
	Input_SetBinding(bindings, ACTION_DBG_RESET, KEY_R);
	Input_SetBinding(bindings, ACTION_DBG_PLACE_SAND, KEY_MOUSE_LEFT_BUTTON);
	Input_SetBinding(bindings, ACTION_INTERACT, KEY_E);
}

int main()
{
	KeyBindings bindings;
	RenderData render_data;
	AudioData audio_data;
	SandGame game;

	DefaultKeyBindings(
		&bindings
	);
	Render_Init(
		&render_data, 
		WINDOW_WIDTH, 
		WINDOW_HEIGHT, 
		144, 
		"Sand"
	);
	Audio_Init(
		&audio_data
	);

	Level_LoadFromFile(&audio_data, &render_data, &game, "levels\\lvl0.sg");

	// Game loop
	while (!Render_ShouldGameClose(&render_data)) {
		if (SandGame_ShouldLoadNewLevel(&game)) {
			Level_LoadFromFile(&audio_data, &render_data, &game, SandGame_GetNewLevelPath(&game));
		}

		auto dt_s = GetFrameTime();
		Input_FetchState(&bindings, &game.action_flags_pressed, &game.action_flags_held, &game.cursor_x, &game.cursor_y);
		SandGame_Update(&game, dt_s);
		Render_RenderGame(&render_data, &game, dt_s);
		Audio_PlayFor(&audio_data, &game);
	}

	SandGame_Destroy(&game);
	Audio_Shutdown(&audio_data);
	Render_Shutdown(&render_data);
	Input_Destroy(&bindings);

	return 0;
}

