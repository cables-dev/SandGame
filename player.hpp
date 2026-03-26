#pragma once
#include "raylib.h"
#include "sand_sim.hpp"
#include <cstdint>

constexpr auto PLAYER_HORIZONTAL_SPEED = 700.0;
constexpr auto PLAYER_HORIZONTAL_SLOW_SPEED = 1200.0;
constexpr auto PLAYER_HORIZONTAL_SLOW_SPEED_THRESH = 0.03 * PLAYER_HORIZONTAL_SLOW_SPEED;
constexpr auto PLAYER_MAX_SPEED = 1500.0;
constexpr auto PLAYER_AUTO_JUMP_HEIGHT = 3.0;
constexpr auto PLAYER_JUMP_SPEED = 300.0;
constexpr auto PLAYER_JUMP_EPSILON = SAND_SIZE / 2.0;
constexpr auto PLAYER_JUMP_SPEED_BOOST_X = 40.0;
constexpr auto G = 900.0;

enum PlayerFireType {
	FIRE_TYPE_STREAM,
	FIRE_TYPE_BURST,
	FIRE_TYPE_MAX
};

struct Player {
	AABB bbox{};
	double x_speed{};
	double y_speed{};
	bool do_jump{};
	PlayerFireType fire_mode{};
};

void Player_Create(
	Player* player,
	double player_x,
	double player_y,
	double player_w,
	double player_h
);
void Player_UpdatePlayer(
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

