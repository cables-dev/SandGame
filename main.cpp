// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "common.hpp"
#include "engine/engine_audio.hpp"
#include "engine/engine_input.hpp"
#include "game.hpp"
#include "render.hpp"
#include "level.hpp"
#include <cassert>

//⠀⠀⠀⠀⠀⠀⢠⣤⣴⣶⣶⣶⣶⣶⣶⣶⣶⣦⣤⣤⣄⣀⣀⣀⡀⠀⠀⠀⠀
//⠀⠀⠀⢠⣶⣿⣿⡿⠿⠿⠿⠛⠛⠛⠛⠛⠛⠿⠿⠿⠿⢿⣿⣿⣿⣿⡆⠀⠀
//⠀⠀⠀⢸⣿⠟⠉⠀⠀⠀⠀⠀⢀⠀⠀⠀⠀⠀⠀⣀⠀⠀⠀⠀⢸⣿⣇⠀⠀
//⠀⠀⢠⣿⣿⠀⠀⠀⠀⠀⠀⠀⠹⣦⠀⠀⠀⠀⣰⡟⠀⠀⠀⠀⢸⣿⣿⣀⠀
//⠀⠀⢸⣿⡿⠀⢰⣶⣤⣤⣄⣀⡀⠙⣃⠀⠀⡀⠛⠀⣀⣠⣤⣤⣬⣹⡟⢩⡿
//⢸⣟⢻⣿⢷⣤⣴⡟⣿⣿⣿⣿⣿⣶⣋⣀⣀⣻⣿⣿⣿⣿⣿⣿⣿⢿⣧⡟⠁
//⠀⠙⢷⣿⡀⠀⢹⣄⡙⠛⠛⠛⣿⢹⡏⠉⠙⣿⠀⠛⠻⠿⠿⢿⡇⢸⡏⠀⠀
//⠀⠀⠀⠙⡧⠀⠀⠉⠉⠛⠛⠛⠋⢸⡇⠀⠀⣿⠛⠛⠛⠛⠛⠛⠃⢸⡇⠀⠀
//⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⣸⡇⠀⠀⢻⠀⠀⠀⠀⠀⠀⠀⣸⠃⠀⠀
//⠀⠀⠀⠀⣷⡀⠀⠀⠀⠀⠀⠀⣠⣿⣤⣀⣠⣾⡇⠀⠀⠀⠀⠀⠀⡟⠀⠀⠀
//⠀⠀⠀⠀⠘⣇⠀⠀⠀⠀⣠⠀⠛⠀⠀⠉⠁⠈⠛⠰⣦⠀⠀⠀⢰⡇⠀⠀⠀
//⠀⠀⠀⠀⠀⢿⡀⠀⠀⣾⢋⣤⡴⠞⠛⠳⠞⠛⠳⣦⣌⢷⡄⢀⡾⠁⠀⠀⠀
void DefaultKeyBindings(EngineInputConfig* bindings) {
	EngineInput_Create(bindings);
	EngineInput_SetBinding(bindings, ACTION_MOVE_LEFT, KEY_A);				
	EngineInput_SetBinding(bindings, ACTION_MOVE_RIGHT, KEY_D);
	EngineInput_SetBinding(bindings, ACTION_JUMP, KEY_SPACE);
	EngineInput_SetBinding(bindings, ACTION_FIRE, KEY_W);
	EngineInput_SetBinding(bindings, ACTION_FIRE, KEY_MOUSE_LEFT_BUTTON);
	EngineInput_SetBinding(bindings, ACTION_SWITCH_FIRE_MODE, KEY_LEFT_CONTROL);
	EngineInput_SetBinding(bindings, ACTION_DBG_RESET, KEY_R);
	EngineInput_SetBinding(bindings, ACTION_INTERACT, KEY_E);
}
//⠀⠀⠀⠀⠀⠸⣧⠀⢀⣴⣿⡛⠛⠛⠛⠛⠛⠛⠛⣛⣿⢷⣄⣼⠇⠀⠀⠀⠀
//⠀⠀⠀⠀⠀⠀⠙⣷⡟⠁⠈⠻⢦⣤⣤⣤⡤⠶⠟⠋⠁⣠⡿⠃⠀⠀⠀⠀⠀	- "I like games and gaming and consoles and controllers and Call of Duty and Mario.
//⠀⠀⠀⠀⠀⠀⠀⠈⠻⣶⣄⠀⠀⠾⠟⠛⠳⠆⠀⣠⣾⠋⠀⠀⠀⠀⠀⠀⠀    I also like Sematary and Haunted Mound and Ghost Mountain. I got hungry so I am
//⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⢷⣄⣀⣀⣀⣠⡶⠟⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀	   eating the function sorry." 
//⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠉⠉⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀

int main()
{
	EngineInputConfig bindings;
	RenderData render_data;
	EngineAudioData audio_data;
	SandGame game;

	DefaultKeyBindings(
		&bindings
	);
	Render_Init(
		&render_data, 
		WINDOW_WIDTH, 
		WINDOW_HEIGHT, 
		144, 
		"Sand",
		GRAPHIC_RSC_BACKGROUND
	);
	EngineAudio_Init(
		&audio_data
	);

	Level_LoadFromFile(
		&audio_data, 
		&render_data, 
		&game, 
		"levels\\lvl_empty.sg"
	);

	// Game loop
	while (!Render_ShouldGameClose(&render_data)) {
		if (SandGame_ShouldLoadNewLevel(&game)) {
			Level_LoadFromFile(&audio_data, &render_data, &game, SandGame_GetNewLevelPath(&game));
		}

		auto dt_s = Engine_GetFrameTime();
		EngineInput_FetchState(&bindings, &game.action_flags_pressed, &game.action_flags_held, &game.cursor_x, &game.cursor_y);
		SandGame_Update(&game, dt_s);
		Render_RenderGame(&render_data, &game, dt_s);
		EngineAudio_Play(&audio_data, &game.sfx_flags);
	}

	SandGame_Destroy(&game);
	EngineAudio_Shutdown(&audio_data);
	Render_Shutdown(&render_data);
	EngineInput_Destroy(&bindings);

	return 0;
}

