// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "common.hpp"
#include "game.hpp"
#include "render.hpp"
#include "audio.hpp"
#include "level.hpp"

int main()
{
	RenderData render_data;
	AudioData audio_data;
	SandGame game;

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
	SandGame_Create(
		&game, 
		PIT_WIDTH / 2.0, 
		PIT_HEIGHT / 2.0, 
		18, 
		30, 
		PIT_WIDTH, 
		PIT_HEIGHT, 
		4, 
		1, 
		SAND_STUBBORNNESS, 
		3, 
		100
	);

	Level_LoadFromFile(&audio_data, &render_data, &game, "levels\\lvl0.sg");
	Render_LoadAndSetAnimationResource(&render_data, GRAPHIC_RSC_BARREL_TERRORIST_IDLE, 0.05, "assets\\barrel_terrorist_idle.gif");
	Render_LoadAndSetAnimationResource(&render_data, GRAPHIC_RSC_BARREL_TERRORIST_EXPLODE, 0.01, "assets\\barrel_terrorist_explode.gif");

	EntityBarrel barrel;
	Barrel_Create(&barrel, 750, 100, 20, 20, GRAPHIC_RSC_BARREL_TERRORIST_IDLE, GRAPHIC_RSC_BARREL_TERRORIST_EXPLODE);
	SandGame_AddEntity(&game, &barrel, ENTITY_BARREL);

	int current_frame = 0;
	double last_update = 0;
	while (!Render_ShouldGameClose(&render_data)) {
		auto dt_s = GetFrameTime();
		auto dt_ms = dt_s * 1000;

		SandGame_Update(&game, dt_ms);
		Render_RenderGame(&render_data, &game, dt_s);
		Audio_PlayFor(&audio_data, &game);
	}

	SandGame_Destroy(&game);
	Audio_Shutdown(&audio_data);
	Render_Shutdown(&render_data);

	return 0;
}

