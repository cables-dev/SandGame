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

void Init(SandPit* p) {
	SandPit_Create(*p, kPitWidth, kPitHeight, kSandStubbornness, kSandS);
}

void Shutdown(SandPit* p) {
	SandPit_Destroy(*p);
}

void DrawSandPit(const SandPit& pit) {
	constexpr static Color particleColours[]{
		Color{243, 227, 124, 255},
		Color{232, 225, 74, 255},
		Color{234, 202, 74, 255},
		Color{253, 221, 88, 255}
	};
	for (int x = 0; x < kPitWidth; x++) {
		for (int y = 0; y < kPitHeight; y++) {
			auto id = SandPit_GetIdAt(pit, x, y);

			if (id != PARTICLE_NOT_PRESENT) 
				DrawRectangle(x * kSandS, kWindowHeight - (y * kSandS + kSandS), kSandS, kSandS, particleColours[id]);
		}
	}
}

void UpdateSandPit(SandPit* pit, bool place_button, bool action_button, bool clear_pit) {
	auto mouse_x = GetMouseX();
	auto mouse_y = GetMouseY();
	if (place_button) {
		PlaceSandCircle(pit, mouse_x / kSandS, kPitHeight - (mouse_y / kSandS), 3);
	}
	if (action_button) {
		VacuumSand(pit, mouse_x / kSandS, kPitHeight - (mouse_y / kSandS), 32);
		SandPit_ClearRegion(*pit, mouse_x / kSandS - 2, kPitHeight - mouse_y / kSandS - 2, 10, 14);
	}
	if (clear_pit) {
		SandPit_Clear(*pit);
	}
}

int main()
{
	Player player{ kPitWidth / 2.0, kPitHeight / 2.0, 12, 21, 0.0, false };

	SandPit pit;
	Init(&pit);

	InitWindow(kWindowWidth, kWindowHeight, "Sand");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		auto dt = GetFrameTime();

		UpdateSandPit(&pit, IsMouseButtonDown(MOUSE_BUTTON_LEFT), IsMouseButtonDown(MOUSE_BUTTON_RIGHT), IsKeyPressed(KEY_R));
		SandPit_SimulateStep(pit);
		UpdatePlayer(
			&player,
			&pit,
			IsKeyDown(KEY_A),
			IsKeyDown(KEY_D),
			IsKeyDown(KEY_SPACE),
			IsKeyDown(KEY_W), 
			IsKeyPressed(KEY_W), 
			IsKeyPressed(KEY_LEFT_CONTROL),
			GetMouseX(),
			kWindowHeight - GetMouseY(),
			dt
		);

		BeginDrawing();
		ClearBackground(BLACK);
		DrawFPS(20, 20);

		// Draw Player
		Rectangle player_rect = { 
			player.x - (player.w / 2), 
			kWindowHeight - (player.y + player.h), 
			player.w, player.h 
		};
		DrawRectangleRec(player_rect, RED);

		DrawSandPit(pit);
		EndDrawing();
	}

	Shutdown(&pit);
	CloseWindow();

	return 0;
}

