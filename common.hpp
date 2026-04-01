#pragma once
#include "engine/engine.hpp"
#include "resource.hpp"
#include <cstdint>

#define NEEDS_FREE

constexpr std::uint32_t LOWEST_Y_COORDINATE{ 0 };
constexpr std::uint32_t WINDOW_WIDTH{ 1920 };
constexpr std::uint32_t WINDOW_HEIGHT{ 1080 };
constexpr std::uint32_t SAND_STUBBORNNESS{ 4 };
constexpr std::uint32_t NUM_SCREENS_HORIZONTAL{ 4 };
constexpr auto NUM_SCREENS_VERTICAL{ 1 };

enum GameActionDefs : GameAction {
	ACTION_MOVE_LEFT = 1 << 0,
	ACTION_MOVE_RIGHT = 1 << 1,
	ACTION_JUMP = 1 << 2,
	ACTION_INTERACT = 1 << 3,
	ACTION_DBG_RESET = 1 << 4,
	ACTION_FIRE = 1 << 5,
	ACTION_SWITCH_FIRE_MODE = 1 << 6,
	ACTION_DBG_PLACE_SAND = 1 << 7,
	ACTION_MAX = 1 << 8// Remeber to update me!
};

struct StringChomper {
    char* ptr;
};

void StringChomper_Create(StringChomper* chomper, char* buffer);
char* StringChomper_GetPointer(StringChomper* chomper);
void StringChomper_InsertTerminator(StringChomper* chomper);
void StringChomper_Step(StringChomper* chomp);
char StringChomper_Peek(const StringChomper* chomp);
void StringChomper_SkipWhitespace(StringChomper* chomp);
bool StringChomper_ReadStringLiteral(
    StringChomper* chomp,
    char** out_ptr,
    std::uint32_t* out_literal_size
);
void StringChomper_ReadString(
	StringChomper* chomp,
	char** out_ptr,
	std::uint32_t* out_string_size
);
bool StringChomper_ReadInteger(
	StringChomper* chomp,
	int* out_num
);
bool StringChomper_ReadUnsignedInteger(
	StringChomper* chomp,
	std::uint32_t* out_num
);
bool StringChomper_ReadBoolean(
	StringChomper* chomp,
	bool* out_b
);
bool StringChomper_ReadColour(
	StringChomper* chomp,
	GameColour* out_colour
);
bool StringChomper_ReadDouble(
	StringChomper* chomp,
	double* out_num
);