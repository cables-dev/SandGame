// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "common.hpp"
#include "game.hpp"
#include "render.hpp"
#include "entity.hpp"

int main()
{
	RenderData render_data;
	SandGame game;

	Render_Init(
		&render_data, 
		WINDOW_WIDTH, 
		WINDOW_HEIGHT, 
		144, 
		"Sand"
	);
	SandGame_Create(
		&game, 
		PIT_WIDTH / 2.0, 
		PIT_HEIGHT / 2.0, 
		12, 
		30, 
		PIT_WIDTH, 
		PIT_HEIGHT, 
		4, 
		1, 
		SAND_STUBBORNNESS, 
		3, 
		100
	);

	EntityRectangleObstacle rect;
	auto rect_colour = GameColour{0x2ff2f2ff};
	RectangleObstacle_Create(&rect, 250 * SAND_SIZE, 250 * SAND_SIZE, 300, 300, rect_colour);

	auto* rect_ent = SandGame_AddEntity(&game, &rect, ENTITY_RECTANGLE);
	while (!Render_ShouldGameClose(&render_data)) {
		SandGame_Simulate(&game, GetFrameTime());
		Render_RenderGame(&render_data, &game);
	}

	SandGame_Destroy(&game);
	Render_Shutdown(&render_data);

	return 0;
}

