#pragma once
#include "raylib.h"
#include "sand_sim.hpp"
#include <cstdint>

constexpr auto PLAYER_HORIZONTAL_SPEED = 700.0;
constexpr auto PLAYER_HORIZONTAL_SLOW_SPEED = 1200.0;
constexpr auto PLAYER_HORIZONTAL_SLOW_SPEED_THRESH = 0.03 * PLAYER_HORIZONTAL_SLOW_SPEED;
constexpr auto PLAYER_MAX_SPEED = 1500.0;
constexpr auto PLAYER_JUMP_SPEED = 300.0;
constexpr auto G = 900.0;

enum PlayerFireType {
	FIRE_TYPE_STREAM,
	FIRE_TYPE_BURST,
	FIRE_TYPE_MAX
};

struct Player {
	double x{};
	double y{};
	int w{};
	int h{};
	double x_speed{};
	double y_speed{};
	PlayerFireType fire_mode{};
};

void UpdatePlayer(
	Player* player,
	SandPit* world,
	bool left,
	bool right,
	bool jump,
	bool fire_held,
	bool fire_press,
	bool switch_fire_mode,
	int mouse_x,
	int mouse_y,
	float dt
);

void PlaceSandCircle(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r);
void VacuumSand(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r);
void BlowSand(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r, std::int32_t v_x, std::int32_t v_y);
