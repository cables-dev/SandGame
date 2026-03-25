// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "common.hpp"
#include "player.hpp"
#include "sand_sim.hpp"
#include "raylib.h"
#include <cstdint>
#include <iostream>

constexpr std::uint32_t kWindowWidth{ 1800 };
constexpr std::uint32_t kWindowHeight{ 900 };
constexpr std::uint32_t kSandStubbornness{ 8 };
constexpr std::uint32_t kPitWidth{ kWindowWidth / kSandS };
constexpr std::uint32_t kPitHeight{ kWindowHeight / kSandS };

Texture2D sand_texture;
Color* sand_pixel_buffer;

void Init(SandPit* p) {
	SandPit_Create(p, kPitWidth, kPitHeight, 2, 1, kSandStubbornness, kSandS);

	auto img = GenImageColor(kPitWidth, kPitHeight, BLANK);
	sand_pixel_buffer = (Color*)malloc(kPitWidth * kPitHeight * sizeof(Color));
	sand_texture = LoadTextureFromImage(img);
	UnloadImage(img);
}

void Shutdown(SandPit* p) {
	SandPit_Destroy(p);
}

constexpr static Color particleColours[]{
	Color{243, 227, 124, 255},
	Color{232, 225, 74, 255},
	Color{234, 202, 74, 255},
	Color{253, 221, 88, 255}
};

void DrawSandPit(const SandPit* pit, std::uint32_t player_x, std::uint32_t player_y) {
	// Annoying lambda capture workaround
	static auto start_x = ((player_x / kSandS) / kPitWidth) * kPitWidth;
	static auto start_y = ((player_y / kSandS) / kPitHeight) * kPitHeight;

    memset(sand_pixel_buffer, 0, kPitWidth * kPitHeight * sizeof(Color));
    auto cb = [](TWO_BIT_ID id, uint32_t x, uint32_t y) {
        sand_pixel_buffer[(kPitHeight-y-1) * kPitWidth + x] = particleColours[id];
    };
    SandPit_ForEachGrain(pit, cb);

    UpdateTexture(sand_texture, sand_pixel_buffer);
    DrawTextureEx(sand_texture, {0, 0}, 0.0, (float)kSandS, WHITE);
}

void UpdateSandPit(SandPit* pit, bool place_button, bool action_button, bool clear_pit) {
	auto mouse_x = GetMouseX();
	auto mouse_y = GetMouseY();
	if (place_button) {
		PlaceSandCircle(pit, mouse_x / kSandS, kPitHeight - (mouse_y / kSandS), 3);
	}
	if (action_button) {
		VacuumSand(pit, mouse_x / kSandS, kPitHeight - (mouse_y / kSandS), 32);
		SandPit_ClearRegion(pit, mouse_x / kSandS - 2, kPitHeight - mouse_y / kSandS - 2, 10, 14);
	}
	if (clear_pit) {
		SandPit_Clear(pit);
	}

	SandPit_SimulateStep(pit);
}

int main()
{
	Player player{ kPitWidth / 2.0, kPitHeight / 2.0, 12, 21, 0.0, false };
	SandPit pit;
	InitWindow(kWindowWidth, kWindowHeight, "Sand");
	SetTargetFPS(144);

	Init(&pit);

	while (!WindowShouldClose()) {
		auto dt = GetFrameTime();

		auto player_x = (int)player.x;
		auto player_y = (int)player.y;
		auto start_x = ((int)player_x / kWindowWidth) * kWindowWidth;
		auto start_y = ((int)player_y / kWindowHeight) * kWindowHeight;

		UpdateSandPit(&pit, IsMouseButtonDown(MOUSE_BUTTON_LEFT), IsMouseButtonDown(MOUSE_BUTTON_RIGHT), IsKeyPressed(KEY_R));
		UpdatePlayer(
			&player,
			&pit,
			IsKeyDown(KEY_A),
			IsKeyDown(KEY_D),
			IsKeyDown(KEY_SPACE),
			IsKeyDown(KEY_W), 
			IsKeyPressed(KEY_W), 
			IsKeyPressed(KEY_LEFT_CONTROL),
			start_x + GetMouseX(),
			kWindowHeight - GetMouseY() + start_y,
			dt
		);

		BeginDrawing();
		ClearBackground(BLACK);
		DrawFPS(20, 20);

		// Draw Player
		Rectangle player_rect = { 
			(player_x - start_x) - (player.w / 2), 
			kWindowHeight - ((player_y - start_y) + player.h), 
			player.w, player.h 
		};
		DrawRectangleRec(player_rect, RED);

		DrawSandPit(&pit, player.x, player.y);
		EndDrawing();
	}

	Shutdown(&pit);
	CloseWindow();

	return 0;
}

