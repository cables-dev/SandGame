// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "common.hpp"
#include "engine/engine_audio.hpp"
#include "engine/engine_input.hpp"
#include "game.hpp"
#include "render.hpp"
#include "serialise.hpp"
#include "deserialise.hpp"
#include "edit.hpp"
#include <cassert>
#include <time.h>

void DefaultKeyBindings(EngineInputConfig* bindings);

int main()
{
	SandGame game;
	DeserialiseError err;
	DeserialiseMetadata md;
	EngineInputConfig bindings;
	RenderData render_data;
	EngineAudioData audio_data;
	EditModeData edit;
	GameActionFlags pressed_actions;
	GameActionFlags held_actions;
	int cursor_x;
	int cursor_y;

	EditMode_Create(
		&edit,
		SAND_SIZE
	);
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

	Level_LoadFile(
		&audio_data, 
		&render_data, 
		&game, 
		&md, 
		"levels\\lvl_empty.sg", 
		&err
	);

	// Game loop
	while (!Render_ShouldGameClose(&render_data)) {
		if (SandGame_ShouldLoadNewLevel(&game)) {
			Level_LoadFile(&audio_data, &render_data, &game, &md, SandGame_GetNewLevelPath(&game), &err);
		}

		auto dt = Engine_GetFrameTime();
		EngineInput_FetchState(&bindings, &pressed_actions, &held_actions, &cursor_x, &cursor_y);
		EditMode_ReceiveInput(&edit, &pressed_actions, &held_actions, cursor_x, cursor_y);
		SandGame_ReceiveInput(&game, &pressed_actions, &held_actions, cursor_x, cursor_y);
		SandGame_Update(&game, dt);
		EditMode_Update(&edit, &game, &md, dt);

		Render_Begin(&render_data);
		Render_RenderGame(&render_data, &game, dt);
		Render_RenderEditMode(&render_data, &edit, dt);
		Render_End(&render_data, &game.fx_flags, dt);

		EngineAudio_Play(&audio_data, &game.sfx_flags);
	}

	SandGame_Destroy(&game);
	EngineAudio_Shutdown(&audio_data);
	Render_Shutdown(&render_data);
	EngineInput_Destroy(&bindings);
	EditMode_Destroy(&edit);

	return 0;
}

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
	EngineInput_SetBinding(bindings, ACTION_TOGGLE_EDIT_MODE, KEY_B);
	EngineInput_SetBinding(bindings, ACTION_EDIT_MODE_NEXT_ENTITY, KEY_RIGHT);
	EngineInput_SetBinding(bindings, ACTION_EDIT_MODE_PREV_ENTITY, KEY_LEFT);
	EngineInput_SetBinding(bindings, ACTION_EDIT_MODE_PLACE_ENTITY, KEY_MOUSE_LEFT_BUTTON);
	EngineInput_SetBinding(bindings, ACTION_EDIT_MODE_SELECT_NEXT, KEY_DOWN);
	EngineInput_SetBinding(bindings, ACTION_EDIT_MODE_SELECT_PREV, KEY_UP);
	EngineInput_SetBinding(bindings, ACTION_EDIT_MODE_INCREASE_FINE , KEY_L);
	EngineInput_SetBinding(bindings, ACTION_EDIT_MODE_INCREASE_COARSE, KEY_J);
	EngineInput_SetBinding(bindings, ACTION_EDIT_MODE_DECREASE_FINE , KEY_H);
	EngineInput_SetBinding(bindings, ACTION_EDIT_MODE_DECREASE_COARSE, KEY_K);
	EngineInput_SetBinding(bindings, ACTION_EDIT_MODE_DELETE, KEY_MOUSE_MIDDLE_BUTTON);
	EngineInput_SetBinding(bindings, ACTION_EDIT_MODE_TOGGLE_HELP, KEY_Q);
	EngineInput_SetBinding(bindings, ACTION_EDIT_MODE_SAVE_FILE, KEY_N);
	EngineInput_SetBinding(bindings, ACTION_RESET_LEVEL, KEY_DELETE);
}
//⠀⠀⠀⠀⠀⠸⣧⠀⢀⣴⣿⡛⠛⠛⠛⠛⠛⠛⠛⣛⣿⢷⣄⣼⠇⠀⠀⠀⠀
//⠀⠀⠀⠀⠀⠀⠙⣷⡟⠁⠈⠻⢦⣤⣤⣤⡤⠶⠟⠋⠁⣠⡿⠃⠀⠀⠀⠀⠀	- "I like games and gaming and consoles and controllers and Call of Duty and Mario.
//⠀⠀⠀⠀⠀⠀⠀⠈⠻⣶⣄⠀⠀⠾⠟⠛⠳⠆⠀⣠⣾⠋⠀⠀⠀⠀⠀⠀⠀    I also like Sematary and Haunted Mound and Ghost Mountain. I got hungry so I am
//⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⢷⣄⣀⣀⣀⣠⡶⠟⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀	   eating the function sorry." 
//⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠉⠉⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀


