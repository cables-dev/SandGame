#pragma once
#include "engine/engine.hpp"
#include "resource.hpp"
#include <cstdint>

constexpr std::uint32_t LOWEST_Y_COORDINATE{ 0 };
constexpr std::uint32_t WINDOW_WIDTH{ 1920 };
constexpr std::uint32_t WINDOW_HEIGHT{ 1080 };
constexpr std::uint32_t SAND_STUBBORNNESS{ 4 };
constexpr std::uint32_t NUM_SCREENS_HORIZONTAL{ 4 };
constexpr auto NUM_SCREENS_VERTICAL{ 1 };

enum GameActionDefs : GameAction {
	ACTION_MOVE_LEFT_DEF,
	ACTION_MOVE_RIGHT_DEF,
	ACTION_JUMP_DEF,
	ACTION_INTERACT_DEF,
	ACTION_DBG_RESET_DEF,
	ACTION_FIRE_DEF,
	ACTION_SWITCH_FIRE_MODE_DEF,
	ACTION_DBG_PLACE_SAND_DEF,
	ACTION_TOGGLE_EDIT_MODE_DEF,
	ACTION_EDIT_MODE_NEXT_ENTITY_DEF,
	ACTION_EDIT_MODE_PREV_ENTITY_DEF,
	ACTION_EDIT_MODE_PLACE_ENTITY_DEF,
	ACTION_EDIT_MODE_SELECT_NEXT_DEF,
	ACTION_EDIT_MODE_SELECT_PREV_DEF,
	ACTION_EDIT_MODE_INCREASE_FINE_DEF,
	ACTION_EDIT_MODE_INCREASE_COARSE_DEF,
	ACTION_EDIT_MODE_DECREASE_FINE_DEF,
	ACTION_EDIT_MODE_DECREASE_COARSE_DEF,
	ACTION_EDIT_MODE_DELETE_DEF,
	ACTION_EDIT_MODE_TOGGLE_HELP_DEF,
	ACTION_EDIT_MODE_SAVE_FILE_DEF,
	ACTION_MAX
};

const auto ACTION_MOVE_LEFT = GameActionFlag_FromAction(ACTION_MOVE_LEFT_DEF);
const auto ACTION_MOVE_RIGHT = GameActionFlag_FromAction(ACTION_MOVE_RIGHT_DEF);
const auto ACTION_JUMP = GameActionFlag_FromAction(ACTION_JUMP_DEF);
const auto ACTION_INTERACT = GameActionFlag_FromAction(ACTION_INTERACT_DEF);
const auto ACTION_DBG_RESET = GameActionFlag_FromAction(ACTION_DBG_RESET_DEF);
const auto ACTION_FIRE = GameActionFlag_FromAction(ACTION_FIRE_DEF);
const auto ACTION_SWITCH_FIRE_MODE = GameActionFlag_FromAction(ACTION_SWITCH_FIRE_MODE_DEF);
const auto ACTION_DBG_PLACE_SAND = GameActionFlag_FromAction(ACTION_DBG_PLACE_SAND_DEF);
const auto ACTION_TOGGLE_EDIT_MODE = GameActionFlag_FromAction(ACTION_TOGGLE_EDIT_MODE_DEF);
const auto ACTION_EDIT_MODE_NEXT_ENTITY = GameActionFlag_FromAction(ACTION_EDIT_MODE_NEXT_ENTITY_DEF);
const auto ACTION_EDIT_MODE_PREV_ENTITY = GameActionFlag_FromAction(ACTION_EDIT_MODE_PREV_ENTITY_DEF);
const auto ACTION_EDIT_MODE_PLACE_ENTITY = GameActionFlag_FromAction(ACTION_EDIT_MODE_PLACE_ENTITY_DEF);
const auto ACTION_EDIT_MODE_SELECT_NEXT = GameActionFlag_FromAction(ACTION_EDIT_MODE_SELECT_NEXT_DEF);
const auto ACTION_EDIT_MODE_SELECT_PREV = GameActionFlag_FromAction(ACTION_EDIT_MODE_SELECT_PREV_DEF);
const auto ACTION_EDIT_MODE_INCREASE_FINE = GameActionFlag_FromAction(ACTION_EDIT_MODE_INCREASE_FINE_DEF);
const auto ACTION_EDIT_MODE_INCREASE_COARSE = GameActionFlag_FromAction(ACTION_EDIT_MODE_INCREASE_COARSE_DEF);
const auto ACTION_EDIT_MODE_DECREASE_FINE  = GameActionFlag_FromAction(ACTION_EDIT_MODE_DECREASE_FINE_DEF);
const auto ACTION_EDIT_MODE_DECREASE_COARSE = GameActionFlag_FromAction(ACTION_EDIT_MODE_DECREASE_COARSE_DEF);
const auto ACTION_EDIT_MODE_DELETE = GameActionFlag_FromAction(ACTION_EDIT_MODE_DELETE_DEF);
const auto ACTION_EDIT_MODE_TOGGLE_HELP = GameActionFlag_FromAction(ACTION_EDIT_MODE_TOGGLE_HELP_DEF);
const auto ACTION_EDIT_MODE_SAVE_FILE = GameActionFlag_FromAction(ACTION_EDIT_MODE_SAVE_FILE_DEF);

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

void WorldToScreen(double x, double y, double* out_x, double* out_y);
void WorldToScreen(const AABB* aabb, AABB* out);
void ScreenToWorld(double x, double y, double* out_x, double* out_y);
NEEDS_FREE char* CreateRandomFileName(const char* file_prefix, const char* file_extension);
