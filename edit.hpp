#pragma once
#include "game.hpp"

constexpr auto MAX_CURSOR_TEXT_SIZE = 20;

struct EditModeVariableDouble {
	const char* description = nullptr;			// We place descriptions here instead of EditModeVariable to 
	double* item = nullptr;						// make our lives easier in rendering code (we dont have to pass
};												// more than one object around to client routines)
struct EditModeVariableUnsignedInteger {
	const char* description = nullptr;
	std::uint32_t* item = nullptr;
};
struct EditModeVariableInteger {
	const char* description = nullptr;
	int* item = nullptr;
};
struct EditModeVariableColour {
	const char* description = nullptr;
	int rgba_idx = 0;						// 0-3 for r-a
	GameColour* item = nullptr;
};
std::uint8_t* EditModeVariableColour_GetSelectionAt(EditModeVariableColour* var, int rgba_index);
struct EditModeVariableBoolean {
	const char* description = nullptr;
	bool* item = nullptr;
};
struct EditModeVariableGraphicResource {
	const char* description = nullptr;
	GraphicResource* item = nullptr;
};
struct EditModeVariableAudioResource {
	const char* description = nullptr;
	AudioResource* item = nullptr;
};
enum EditModeVariableType {
	EDIT_MODE_VAR_DOUBLE,
	EDIT_MODE_VAR_U32,
	EDIT_MODE_VAR_INT,
	EDIT_MODE_VAR_COLOUR,
	EDIT_MODE_VAR_BOOLEAN,
	EDIT_MODE_VAR_GRAPHIC_RESOURCE,
	EDIT_MODE_VAR_AUDIO_RESOURCE,
	EDIT_MODE_VAR_STRING,
	EDIT_MODE_VAR_MAX
};

struct EditModeVariable {
	union {
		EditModeVariableDouble var_double;
		EditModeVariableUnsignedInteger var_unsigned_integer;
		EditModeVariableInteger var_integer;
		EditModeVariableColour var_colour;
		EditModeVariableBoolean var_boolean;
		EditModeVariableGraphicResource var_graphic_resource;
		EditModeVariableAudioResource var_audio_resource;
	} var;
	EditModeVariableType type;
	EditModeVariable* prev = nullptr;
	EditModeVariable* next = nullptr;
};

struct EditModeData {
	bool active{};
	GameActionFlags pressed{};
	GameActionFlags held{};
	int screen_cursor_x{};
	int screen_cursor_y{};
	int world_cursor_x{};
	int world_cursor_y{};
	char cursor_pos_text[MAX_CURSOR_TEXT_SIZE + 1]{};		// + 1 for null terminator
	int cursor_pos_text_size = 0;
	const char* toast_text = nullptr;
	Entity selection_ents[ENTITY_MAX]{};
	EditModeVariable* ent_variable_list_heads[ENTITY_MAX]{};			// Circular singly-linked list.
	EditModeVariable* ent_variable_list_selections[ENTITY_MAX]{};		// Circular singly-linked list.
	int selected_ent_index{};						// -1 for no selection
};

void EditMode_Create(EditModeData* data);
void EditMode_Destroy(EditModeData* data);
void EditMode_ReceiveInput(EditModeData* data, GameActionFlags* in_out_pressed,	GameActionFlags* in_out_held, int cursor_x,	int cursor_y);
void EditMode_Update(EditModeData* edit, SandGame* game, EngineTime dt);
bool EditMode_GetPrototypeEntity(EditModeData* edit, Entity** out_selected_entity);
bool EditMode_IsEntitySelected(EditModeData* edit);
bool EditMode_IsEnabled(EditModeData* data);
EditModeVariable* EditMode_GetSelectedEntityVariablesListHead(EditModeData* data);
EditModeVariable* EditMode_GetSelectedEntityVariablesSelectedVariable(EditModeData* data);


