#include "edit.hpp"
#include <cassert>

void EditModeVariableDouble_Create(EditModeVariableDouble* var, const char* description, double* ptr) {
	assert(ptr);				// So we don't have to do a nullptr check on every funtion call
	var->item = ptr;
	var->description = description;
}
void EditModeVariableDouble_Destroy(EditModeVariableDouble* var) { /*pass*/ }
void EditModeVariableDouble_IncreaseFine(EditModeVariableDouble* var) {	*var->item += 0.1; }
void EditModeVariableDouble_IncreaseCoarse(EditModeVariableDouble* var) { *var->item += 1.0; }
void EditModeVariableDouble_DecreaseFine(EditModeVariableDouble* var) { *var->item -= 0.1; }
void EditModeVariableDouble_DecreaseCoarse(EditModeVariableDouble* var) { *var->item -= 1.0; }
bool EditModeVariableDouble_OnSelectNext(EditModeVariableDouble* var) { return true; }			// Return true if we permit the parent to cycle to the next variable
bool EditModeVariableDouble_OnSelectPrev(EditModeVariableDouble* var) { return true; }			// Return true if we permit the parent to cycle to the next variable

void EditModeVariableUnsignedInteger_Create(EditModeVariableUnsignedInteger* var, const char* description, std::uint32_t* ptr) {
	assert(ptr);				// So we don't have to do a nullptr check on every funtion call
	var->item = ptr;
	var->description = description;
}
void EditModeVariableUnsignedInteger_Destroy(EditModeVariableUnsignedInteger* var) { /*pass*/ }
void EditModeVariableUnsignedInteger_IncreaseFine(EditModeVariableUnsignedInteger* var) {	*var->item += 1; }
void EditModeVariableUnsignedInteger_IncreaseCoarse(EditModeVariableUnsignedInteger* var) { *var->item += 1; }
void EditModeVariableUnsignedInteger_DecreaseFine(EditModeVariableUnsignedInteger* var) { *var->item -= 1; }
void EditModeVariableUnsignedInteger_DecreaseCoarse(EditModeVariableUnsignedInteger* var) { *var->item -= 1; }
bool EditModeVariableUnsignedInteger_OnSelectNext(EditModeVariableUnsignedInteger* var) { return true; }			// Return true if we permit the parent to cycle to the next variable
bool EditModeVariableUnsignedInteger_OnSelectPrev(EditModeVariableUnsignedInteger* var) { return true; }			// Return true if we permit the parent to cycle to the next variable

void EditModeVariableInteger_Create(EditModeVariableInteger* var, const char* description, int* ptr) {
	assert(ptr);				// So we don't have to do a nullptr check on every funtion call
	var->item = ptr;
	var->description = description;
}
void EditModeVariableInteger_Destroy(EditModeVariableInteger* var) { /*pass*/ }
void EditModeVariableInteger_IncreaseFine(EditModeVariableInteger* var) {	*var->item += 1; }
void EditModeVariableInteger_IncreaseCoarse(EditModeVariableInteger* var) { *var->item += 1; }
void EditModeVariableInteger_DecreaseFine(EditModeVariableInteger* var) { *var->item -= 1; }
void EditModeVariableInteger_DecreaseCoarse(EditModeVariableInteger* var) { *var->item -= 1; }
bool EditModeVariableInteger_OnSelectNext(EditModeVariableInteger* var) { return true; }			// Return true if we permit the parent to cycle to the next variable
bool EditModeVariableInteger_OnSelectPrev(EditModeVariableInteger* var) { return true; }			// Return true if we permit the parent to cycle to the next variable

void EditModeVariableColour_Create(EditModeVariableColour* var, const char* description, GameColour* ptr) {
	assert(ptr);				// So we don't have to do a nullptr check on every funtion call
	var->item = ptr;
	var->rgba_idx = 0;
	var->description = description;
}
void EditModeVariableColour_Destroy(EditModeVariableColour* var) { /*pass*/ }
std::uint8_t* EditModeVariableColour_GetSelection(EditModeVariableColour* var) {
	switch (var->rgba_idx) {
	case 0: { return &(var->item->r); break; }
	case 1: { return &(var->item->g); break; }
	case 2: { return &(var->item->b); break; }
	case 3: { return &(var->item->a); break; }
	}
}
void EditModeVariableColour_IncreaseFine(EditModeVariableColour* var) { *EditModeVariableColour_GetSelection(var) += 1; }
void EditModeVariableColour_IncreaseCoarse(EditModeVariableColour* var) { *EditModeVariableColour_GetSelection(var) += 1; }
void EditModeVariableColour_DecreaseFine(EditModeVariableColour* var) { *EditModeVariableColour_GetSelection(var) -= 1; }
void EditModeVariableColour_DecreaseCoarse(EditModeVariableColour* var) { *EditModeVariableColour_GetSelection(var) -= 1; }
bool EditModeVariableColour_OnSelectNext(EditModeVariableColour* var) { 
	auto result = var->rgba_idx == 3;
	if (!result)
		var->rgba_idx = (var->rgba_idx + 1) & 0b11;			// values 0-3
	return result;
}			// Return true if we permit the parent to cycle to the next variable
bool EditModeVariableColour_OnSelectPrev(EditModeVariableColour* var) { 
	auto result = var->rgba_idx == 0;
	if (!result)
		var->rgba_idx = (var->rgba_idx - 1) & 0b11;			// values 0-3
	return result;
}			// Return true if we permit the parent to cycle to the next variable

void EditModeVariableBoolean_Create(EditModeVariableBoolean* var, const char* description, bool* ptr) {
	assert(ptr);				// So we don't have to do a nullptr check on every funtion call
	var->item = ptr;
	var->description = description;
}
void EditModeVariableBoolean_Destroy(EditModeVariableBoolean* var) { /*pass*/ }
void EditModeVariableBoolean_IncreaseFine(EditModeVariableBoolean* var) { *var->item = !*var->item; }
void EditModeVariableBoolean_IncreaseCoarse(EditModeVariableBoolean* var) { *var->item = !*var->item; }
void EditModeVariableBoolean_DecreaseFine(EditModeVariableBoolean* var) { *var->item = !*var->item; }
void EditModeVariableBoolean_DecreaseCoarse(EditModeVariableBoolean* var) { *var->item = !*var->item; }
bool EditModeVariableBoolean_OnSelectNext(EditModeVariableBoolean* var) { return true; }			// Return true if we permit the parent to cycle to the next variable
bool EditModeVariableBoolean_OnSelectPrev(EditModeVariableBoolean* var) { return true; }			// Return true if we permit the parent to cycle to the next variable

void EditModeVariableGraphicResource_Create(EditModeVariableGraphicResource* var, const char* description, GraphicResource* ptr) {
	assert(ptr);				// So we don't have to do a nullptr check on every funtion call
	var->item = ptr;
	var->description = description;
}
void EditModeVariableGraphicResource_Destroy(EditModeVariableGraphicResource* var) { /*pass*/ }
void EditModeVariableGraphicResource_IncreaseFine(EditModeVariableGraphicResource* var) { *var->item = Bounds(*var->item + 1, -1, MAX_GRAPHIC_RESOURCES); }
void EditModeVariableGraphicResource_IncreaseCoarse(EditModeVariableGraphicResource* var) { *var->item = Bounds(*var->item + 1, -1, MAX_GRAPHIC_RESOURCES); }
void EditModeVariableGraphicResource_DecreaseFine(EditModeVariableGraphicResource* var) { *var->item = Bounds(*var->item - 1, -1, MAX_GRAPHIC_RESOURCES); }
void EditModeVariableGraphicResource_DecreaseCoarse(EditModeVariableGraphicResource* var) { *var->item = Bounds(*var->item - 1, -1, MAX_GRAPHIC_RESOURCES); }
bool EditModeVariableGraphicResource_OnSelectNext(EditModeVariableGraphicResource* var) { return true; }			// Return true if we permit the parent to cycle to the next variable
bool EditModeVariableGraphicResource_OnSelectPrev(EditModeVariableGraphicResource* var) { return true; }			// Return true if we permit the parent to cycle to the next variable

void EditModeVariableAudioResource_Create(EditModeVariableAudioResource* var, const char* description, AudioResource* ptr) {
	assert(ptr);				// So we don't have to do a nullptr check on every funtion call
	var->item = ptr;
	var->description = description;
}
void EditModeVariableAudioResource_Destroy(EditModeVariableAudioResource* var) { /*pass*/ }
void EditModeVariableAudioResource_IncreaseFine(EditModeVariableAudioResource* var) { *var->item = Bounds(*var->item + 1, -1, MAX_GRAPHIC_RESOURCES); }
void EditModeVariableAudioResource_IncreaseCoarse(EditModeVariableAudioResource* var) { *var->item = Bounds(*var->item + 1, -1, MAX_GRAPHIC_RESOURCES); }
void EditModeVariableAudioResource_DecreaseCoarse(EditModeVariableAudioResource* var) { *var->item = Bounds(*var->item - 1, -1, MAX_GRAPHIC_RESOURCES); }
void EditModeVariableAudioResource_DecreaseFine(EditModeVariableAudioResource* var) { *var->item = Bounds(*var->item - 1, -1, MAX_GRAPHIC_RESOURCES); }
bool EditModeVariableAudioResource_OnSelectNext(EditModeVariableAudioResource* var) { return true; }			// Return true if we permit the parent to cycle to the next variable
bool EditModeVariableAudioResource_OnSelectPrev(EditModeVariableAudioResource* var) { return true; }			// Return true if we permit the parent to cycle to the next variable


// Does not initialise list pointer!
void EditModeVariable_CreateFrom(EditModeVariable* edit_var, void* var, EditModeVariableType type) {
	switch(type) {
	case EDIT_MODE_VAR_DOUBLE: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableDouble)); break; }
	case EDIT_MODE_VAR_U32: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableUnsignedInteger)); break; }
	case EDIT_MODE_VAR_INT: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableInteger)); break; }
	case EDIT_MODE_VAR_COLOUR: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableColour)); break; } 
	case EDIT_MODE_VAR_BOOLEAN: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableBoolean)); break; }
	case EDIT_MODE_VAR_GRAPHIC_RESOURCE: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableGraphicResource)); break; }
	case EDIT_MODE_VAR_AUDIO_RESOURCE: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableAudioResource)); break; } 
	//case EDIT_MODE_VAR_STRING: {}
	default: { assert(false && "EditModeVariable_CreateFrom: Unaccounted variable type encountered!"); }
	}
	edit_var->type = type;
	edit_var->next = nullptr;
	edit_var->prev = nullptr;
}

bool EditModeVariable_OnSelectNext(EditModeVariable* var) {
	switch(var->type) {
	case EDIT_MODE_VAR_DOUBLE: { return EditModeVariableDouble_OnSelectNext(&var->var.var_double); }
	case EDIT_MODE_VAR_U32: { return EditModeVariableUnsignedInteger_OnSelectNext(&var->var.var_unsigned_integer); }
	case EDIT_MODE_VAR_INT: { return EditModeVariableInteger_OnSelectNext(&var->var.var_integer); }
	case EDIT_MODE_VAR_COLOUR: { return EditModeVariableColour_OnSelectNext(&var->var.var_colour); }
	case EDIT_MODE_VAR_BOOLEAN: { return EditModeVariableBoolean_OnSelectNext(&var->var.var_boolean); }
	case EDIT_MODE_VAR_GRAPHIC_RESOURCE: { return EditModeVariableGraphicResource_OnSelectNext(&var->var.var_graphic_resource); }
	case EDIT_MODE_VAR_AUDIO_RESOURCE: { return EditModeVariableAudioResource_OnSelectNext(&var->var.var_audio_resource); }
	//case EDIT_MODE_VAR_STRING: {}
	default: { assert(false && "EditModeVariable_OnSelectNext: Unaccounted variable type encountered!"); }
	}
}
bool EditModeVariable_OnSelectPrev(EditModeVariable* var) {
	switch(var->type) {
	case EDIT_MODE_VAR_DOUBLE: { return EditModeVariableDouble_OnSelectPrev(&var->var.var_double); }
	case EDIT_MODE_VAR_U32: { return EditModeVariableUnsignedInteger_OnSelectPrev(&var->var.var_unsigned_integer); }
	case EDIT_MODE_VAR_INT: { return EditModeVariableInteger_OnSelectPrev(&var->var.var_integer); }
	case EDIT_MODE_VAR_COLOUR: { return EditModeVariableColour_OnSelectPrev(&var->var.var_colour); }
	case EDIT_MODE_VAR_BOOLEAN: { return EditModeVariableBoolean_OnSelectPrev(&var->var.var_boolean); }
	case EDIT_MODE_VAR_GRAPHIC_RESOURCE: { return EditModeVariableGraphicResource_OnSelectPrev(&var->var.var_graphic_resource); }
	case EDIT_MODE_VAR_AUDIO_RESOURCE: { return EditModeVariableAudioResource_OnSelectPrev(&var->var.var_audio_resource); }
	//case EDIT_MODE_VAR_STRING: {}
	default: { assert(false && "EditModeVariable_OnSelectPrev: Unaccounted variable type encountered!"); }
	}
}
void EditModeVariable_IncreaseFine(EditModeVariable* var) {
	switch(var->type) {
	case EDIT_MODE_VAR_DOUBLE: { EditModeVariableDouble_IncreaseFine(&var->var.var_double); break; }
	case EDIT_MODE_VAR_U32: { EditModeVariableUnsignedInteger_IncreaseFine(&var->var.var_unsigned_integer); break; }
	case EDIT_MODE_VAR_INT: { EditModeVariableInteger_IncreaseFine(&var->var.var_integer); break; }
	case EDIT_MODE_VAR_COLOUR: { EditModeVariableColour_IncreaseFine(&var->var.var_colour); break; }
	case EDIT_MODE_VAR_BOOLEAN: { EditModeVariableBoolean_IncreaseFine(&var->var.var_boolean); break; } 
	case EDIT_MODE_VAR_GRAPHIC_RESOURCE: { EditModeVariableGraphicResource_IncreaseFine(&var->var.var_graphic_resource); break; } 
	case EDIT_MODE_VAR_AUDIO_RESOURCE: { EditModeVariableAudioResource_IncreaseFine(&var->var.var_audio_resource); break; }
	//case EDIT_MODE_VAR_STRING: {}
	default: { assert(false && "EditModeVariable_IncreaseFine: Unaccounted variable type encountered!"); }
	}
}
void EditModeVariable_IncreaseCoarse(EditModeVariable* var) {
	switch(var->type) {
	case EDIT_MODE_VAR_DOUBLE: { EditModeVariableDouble_IncreaseCoarse(&var->var.var_double); break; }
	case EDIT_MODE_VAR_U32: { EditModeVariableUnsignedInteger_IncreaseCoarse(&var->var.var_unsigned_integer); break; }
	case EDIT_MODE_VAR_INT: { EditModeVariableInteger_IncreaseCoarse(&var->var.var_integer); break; }
	case EDIT_MODE_VAR_COLOUR: { EditModeVariableColour_IncreaseCoarse(&var->var.var_colour); break; }
	case EDIT_MODE_VAR_BOOLEAN: { EditModeVariableBoolean_IncreaseCoarse(&var->var.var_boolean); break; }
	case EDIT_MODE_VAR_GRAPHIC_RESOURCE: { EditModeVariableGraphicResource_IncreaseCoarse(&var->var.var_graphic_resource); break; }
	case EDIT_MODE_VAR_AUDIO_RESOURCE: { EditModeVariableAudioResource_IncreaseCoarse(&var->var.var_audio_resource); break; }
	//case EDIT_MODE_VAR_STRING: {}
	default: { assert(false && "EditModeVariable_IncreaseCoarse: Unaccounted variable type encountered!"); }
	}
}
void EditModeVariable_DecreaseFine(EditModeVariable* var) {
	switch(var->type) {
	case EDIT_MODE_VAR_DOUBLE: { EditModeVariableDouble_DecreaseFine(&var->var.var_double); break; }
	case EDIT_MODE_VAR_U32: { EditModeVariableUnsignedInteger_DecreaseFine(&var->var.var_unsigned_integer); break; }
	case EDIT_MODE_VAR_INT: { EditModeVariableInteger_DecreaseFine(&var->var.var_integer); break; }
	case EDIT_MODE_VAR_COLOUR: { EditModeVariableColour_DecreaseFine(&var->var.var_colour); break; }
	case EDIT_MODE_VAR_BOOLEAN: { EditModeVariableBoolean_DecreaseFine(&var->var.var_boolean); break; }
	case EDIT_MODE_VAR_GRAPHIC_RESOURCE: { EditModeVariableGraphicResource_DecreaseFine(&var->var.var_graphic_resource); break; }
	case EDIT_MODE_VAR_AUDIO_RESOURCE: { EditModeVariableAudioResource_DecreaseFine(&var->var.var_audio_resource); break; } 
	//case EDIT_MODE_VAR_STRING: {}
	default: { assert(false && "EditModeVariable_DecreaseFine: Unaccounted variable type encountered!"); }
	}
}
void EditModeVariable_DecreaseCoarse(EditModeVariable* var) {
	switch(var->type) {
	case EDIT_MODE_VAR_DOUBLE: { EditModeVariableDouble_DecreaseCoarse(&var->var.var_double); break; }
	case EDIT_MODE_VAR_U32: { EditModeVariableUnsignedInteger_DecreaseCoarse(&var->var.var_unsigned_integer); break; }
	case EDIT_MODE_VAR_INT: { EditModeVariableInteger_DecreaseCoarse(&var->var.var_integer); break; }
	case EDIT_MODE_VAR_COLOUR: { EditModeVariableColour_DecreaseCoarse(&var->var.var_colour); break; }
	case EDIT_MODE_VAR_BOOLEAN: { EditModeVariableBoolean_DecreaseCoarse(&var->var.var_boolean); break; }
	case EDIT_MODE_VAR_GRAPHIC_RESOURCE: { EditModeVariableGraphicResource_DecreaseCoarse(&var->var.var_graphic_resource); break; }
	case EDIT_MODE_VAR_AUDIO_RESOURCE: { EditModeVariableAudioResource_DecreaseCoarse(&var->var.var_audio_resource); break; }
	//case EDIT_MODE_VAR_STRING: {}
	default: { assert(false && "EditModeVariable_DecreaseCoarse: Unaccounted variable type encountered!"); }
	}
}

EditModeVariable* EditMode_GetEntityVariablesListHead(EditModeData* data, int index) {
	return data->ent_variable_list_heads[index];
}

EditModeVariable* EditMode_GetEntityVariablesSelectedVariable(EditModeData* data, int index) {
	return data->ent_variable_list_selections[index];
}


void EditMode_InitVariableList(EditModeData* data, EditModeVariable* var, int index) {
	var->next = var;
	var->prev = var;
	data->ent_variable_list_heads[index] = var;
	data->ent_variable_list_selections[index] = var;
}

void EditMode_PushVariableToList(EditModeData* data, EditModeVariable* var, int index) {
	auto* node = EditMode_GetEntityVariablesListHead(data, index);			// Insert to back of list, point back at head.
	if (node == nullptr) {
		EditMode_InitVariableList(data, var, index);
		return;
	}

	auto* head = node;
	while (node->next != node)
		node = node->next;

	node->next = var;
	var->prev = node;
	var->next = var;
}

void EditMode_CreateEntitySelection(EditModeData* data) {
	auto* selections = data->selection_ents;

	EntityRectangleObstacle rect;
	RectangleObstacle_Create(&rect, 0.0, 0.0, 50.0, 50.0, GameColour{ 0xffffffff });
	selections[ENTITY_RECTANGLE] = Entity_CreateFrom(&rect, ENTITY_RECTANGLE);

	EntityHintBox hint;
	HintBox_Create(&hint, "Edit me in file!", false, 0.0, 0.0, 50.0, 50.0);
	selections[ENTITY_HINT_BOX] = Entity_CreateFrom(&hint, ENTITY_HINT_BOX);

	EntityBarrel barrel;
	Barrel_Create(&barrel, 0.0, 0.0, 80.0, 100.0, GRAPHIC_RSC_BARREL_KURT_IDLE, GRAPHIC_RSC_BARREL_KURT_EXPLODE);
	selections[ENTITY_BARREL] = Entity_CreateFrom(&barrel, ENTITY_BARREL);
}

Entity* EditMode_GetPrototypeEntityFromIndex(EditModeData* edit, int index) {
	return &edit->selection_ents[index];
}

void EditMode_SetupRectangleVariables(EditModeData* data) {
	auto rect_index = ENTITY_RECTANGLE;
	auto* rect_ent = &EditMode_GetPrototypeEntityFromIndex(data, rect_index)->entity.rect;
	EditModeVariableDouble width_var_inner;
	EditModeVariableDouble height_var_inner;
	EditModeVariableColour colour_var_inner;
	EditModeVariable* width_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* height_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* colour_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariableDouble_Create(&width_var_inner, "Width: ", &rect_ent->aabb.w);
	EditModeVariableDouble_Create(&height_var_inner, "Height : ", &rect_ent->aabb.h);
	EditModeVariableColour_Create(&colour_var_inner, "Colour : ", &rect_ent->colour);
	EditModeVariable_CreateFrom(width_var, &width_var_inner, EDIT_MODE_VAR_DOUBLE);
	EditModeVariable_CreateFrom(height_var, &height_var_inner, EDIT_MODE_VAR_DOUBLE);
	EditModeVariable_CreateFrom(colour_var, &colour_var_inner, EDIT_MODE_VAR_COLOUR);

	EditMode_InitVariableList(data, width_var, rect_index);
	EditMode_PushVariableToList(data, height_var, rect_index);
	EditMode_PushVariableToList(data, colour_var, rect_index);
}

void EditMode_SetupHintBoxVariables(EditModeData* data) {
	auto hint_index = ENTITY_HINT_BOX;
	auto* hint_ent = &EditMode_GetPrototypeEntityFromIndex(data, hint_index)->entity.hint_box;
	EditModeVariableDouble width_var_inner;
	EditModeVariableDouble height_var_inner;
	EditModeVariableBoolean only_once_inner;
	EditModeVariableAudioResource audio_rsc_inner;
	EditModeVariable* width_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* height_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* only_once_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* audio_rsc_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariableDouble_Create(&width_var_inner, "Width: ", &hint_ent->aabb.w);
	EditModeVariableDouble_Create(&height_var_inner, "Height : ", &hint_ent->aabb.h);
	EditModeVariableBoolean_Create(&only_once_inner, "Trigger Only Once: ", &hint_ent->only_once);
	EditModeVariableAudioResource_Create(&audio_rsc_inner, "Sound Effect: ", &hint_ent->audio_rsc);
	EditModeVariable_CreateFrom(width_var, &width_var_inner, EDIT_MODE_VAR_DOUBLE);
	EditModeVariable_CreateFrom(height_var, &height_var_inner, EDIT_MODE_VAR_DOUBLE);
	EditModeVariable_CreateFrom(only_once_var, &only_once_inner, EDIT_MODE_VAR_BOOLEAN);
	EditModeVariable_CreateFrom(audio_rsc_var, &audio_rsc_inner, EDIT_MODE_VAR_AUDIO_RESOURCE);

	EditMode_InitVariableList(data, width_var, hint_index);
	EditMode_PushVariableToList(data, height_var, hint_index);
	EditMode_PushVariableToList(data, only_once_var, hint_index);
	EditMode_PushVariableToList(data, audio_rsc_var, hint_index);
}

void EditMode_SetupBarrelVariables(EditModeData* data) {
	auto barrel_index = ENTITY_BARREL;
	auto* barrel_ent = &EditMode_GetPrototypeEntityFromIndex(data, barrel_index)->entity.barrel;
	EditModeVariableDouble width_var_inner;
	EditModeVariableGraphicResource idle_rsc_inner;
	EditModeVariableGraphicResource explode_rsc_inner;
	EditModeVariable* width_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* idle_rsc_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* explode_rsc_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariableDouble_Create(&width_var_inner, "Width: ", &barrel_ent->aabb.w);
	EditModeVariableGraphicResource_Create(&idle_rsc_inner, "Idle Sprite: ", &barrel_ent->idle_sprite);
	EditModeVariableGraphicResource_Create(&explode_rsc_inner, "Explode Sprite: ", &barrel_ent->explode_sprite);
	EditModeVariable_CreateFrom(width_var, &width_var_inner, EDIT_MODE_VAR_DOUBLE);
	EditModeVariable_CreateFrom(idle_rsc_var, &idle_rsc_inner, EDIT_MODE_VAR_GRAPHIC_RESOURCE);
	EditModeVariable_CreateFrom(explode_rsc_var, &explode_rsc_inner, EDIT_MODE_VAR_GRAPHIC_RESOURCE);

	EditMode_InitVariableList(data, width_var, barrel_index);
	EditMode_PushVariableToList(data, idle_rsc_var, barrel_index);
	EditMode_PushVariableToList(data, explode_rsc_var, barrel_index);
}

void EditMode_SetupLevelDoorVariables(EditModeData* data) {
	auto barrel_index = ENTITY_LEVEL_DOOR;
	auto* barrel_ent = &EditMode_GetPrototypeEntityFromIndex(data, barrel_index)->entity.door;
}

void EditMode_SetupVariables(EditModeData* data) {
	EditMode_SetupRectangleVariables(data);
	EditMode_SetupHintBoxVariables(data);
	EditMode_SetupBarrelVariables(data);
	EditMode_SetupLevelDoorVariables(data);
}

std::uint8_t* EditModeVariableColour_GetSelectionAt(EditModeVariableColour* var, int rgba_index) {
	switch (rgba_index) {
	case 0: { 
		return &var->item->r; 
	}
	case 1: { 
		return &var->item->g; 
	}
	case 2: { 
		return &var->item->b; 
	}
	case 3: { 
		return &var->item->a; 
	}
	default: {
		return nullptr;
	}

	}
}

void EditMode_Create(EditModeData* data){
	data->active = false;
	data->pressed = NULL_ACTION_FLAGS;
	data->held = NULL_ACTION_FLAGS;
	data->screen_cursor_x = 0;
	data->screen_cursor_y = 0;
	data->toast_text = nullptr;
	data->selected_ent_index = -1;
	memset(data->cursor_pos_text, 0, sizeof(data->cursor_pos_text + 1) / sizeof(char));	// + 1 for null terminator
	EditMode_CreateEntitySelection(data);
	EditMode_SetupVariables(data);
}

void EditMode_Destroy(EditModeData* data){
	data->active = false;
}

const static GameActionFlag STEAL_INPUTS[]{ ACTION_FIRE };
const static auto NUM_STEAL_INPUTS{ sizeof(STEAL_INPUTS) / sizeof(GameActionFlag) };
void EditMode_StealInputs(GameActionFlags* in_out_pressed, GameActionFlags* in_out_held) {
	for (int i = 0; i < NUM_STEAL_INPUTS; i++) {
		GameActionFlags_Set(in_out_pressed, STEAL_INPUTS[i], false);
		GameActionFlags_Set(in_out_held, STEAL_INPUTS[i], false);
	}
}

bool EditMode_IsEnabled(EditModeData* data) {
	return data->active;
}

EditModeVariable* EditMode_GetSelectedEntityVariablesListHead(EditModeData* data) {
	auto ent_index = data->selected_ent_index;
	return EditMode_GetEntityVariablesListHead(data, ent_index);
}

EditModeVariable* EditMode_GetSelectedEntityVariablesSelectedVariable(EditModeData* data) {
	auto ent_index = data->selected_ent_index;
	return EditMode_GetEntityVariablesSelectedVariable(data, ent_index);
}

void EditMode_Toggle(EditModeData* data) {
	data->active = !data->active;
	if (EditMode_IsEnabled(data)) {
		data->toast_text = "EDIT MODE ENABLED. Press B to disable.";
	}
	else {
		data->toast_text = "Edit Mode Disabled";
	}
}

void EditMode_NoToggle(EditModeData* data) {
	data->toast_text = nullptr;
}

void EditMode_ReceiveInput(EditModeData* data, GameActionFlags* in_out_pressed, GameActionFlags* in_out_held, int cursor_x, int cursor_y) {
	data->pressed = *in_out_pressed;
	data->held = *in_out_held;
	data->screen_cursor_x = cursor_x;
	data->screen_cursor_y = cursor_y;

	if (GameActionFlags_Get(data->pressed, ACTION_TOGGLE_EDIT_MODE))
		EditMode_Toggle(data);
	else
		EditMode_NoToggle(data);

	if (EditMode_IsEnabled(data))
		EditMode_StealInputs(in_out_pressed, in_out_held);
}

constexpr auto MAX_AVAILABLE_ENT = 1;				// max entity id + 1
void EditMode_NextEntity(EditModeData* data) {
	++data->selected_ent_index;
	if (data->selected_ent_index >= MAX_AVAILABLE_ENT)
		data->selected_ent_index = -1;
}

void EditMode_PrevEntity(EditModeData* data) {
	--data->selected_ent_index;
	if (data->selected_ent_index <= -2)
		data->selected_ent_index = MAX_AVAILABLE_ENT - 1;
}

void EditMode_OnSelectNextVariable(EditModeData* data) {
	if (!EditMode_IsEntitySelected(data))
		return;

	auto ent_index = data->selected_ent_index;
	auto* current_var = EditMode_GetSelectedEntityVariablesSelectedVariable(data);
	if (EditModeVariable_OnSelectNext(current_var)) {			// Permission to cycle?
		data->ent_variable_list_selections[ent_index] = current_var->next;
	}
}

void EditMode_OnSelectPrevVariable(EditModeData* data) {
	if (!EditMode_IsEntitySelected(data))
		return;

	auto ent_index = data->selected_ent_index;
	auto* current_var = data->ent_variable_list_selections[ent_index];
	if (EditModeVariable_OnSelectPrev(current_var)) {			// Permission to cycle?
		data->ent_variable_list_selections[ent_index] = current_var->prev;
	}
}

void EditMode_VariableIncreaseFine(EditModeData* data) {
	if (!EditMode_IsEntitySelected(data))
		return;

	auto ent_index = data->selected_ent_index;
	auto* current_var = EditMode_GetEntityVariablesSelectedVariable(data, ent_index);
	EditModeVariable_IncreaseFine(current_var);
}

void EditMode_VariableIncreaseCoarse(EditModeData* data) {
	if (!EditMode_IsEntitySelected(data))
		return;
	auto ent_index = data->selected_ent_index;
	auto* current_var = EditMode_GetEntityVariablesSelectedVariable(data, ent_index);
	EditModeVariable_IncreaseCoarse(current_var);
}

void EditMode_VariableDecreaseFine(EditModeData* data) {
	if (!EditMode_IsEntitySelected(data))
		return;
	auto ent_index = data->selected_ent_index;
	auto* current_var = EditMode_GetEntityVariablesSelectedVariable(data, ent_index);
	EditModeVariable_DecreaseFine(current_var);
}

void EditMode_VariableDecreaseCoarse(EditModeData* data) {
	if (!EditMode_IsEntitySelected(data))
		return;
	auto ent_index = data->selected_ent_index;
	auto* current_var = EditMode_GetEntityVariablesSelectedVariable(data, ent_index);
	EditModeVariable_DecreaseCoarse(current_var);
}

void EditMode_HandleInput(
	EditModeData* data, 
	SandGame* game
) {
	// Convert mouse screen coordinates to world coordinates 
	SandGame_ScreenCoordsToWorldCoords(game, data->screen_cursor_x, data->screen_cursor_y, &data->world_cursor_x, &data->world_cursor_y);

	// Buttons
	if (GameActionFlags_Get(data->pressed, ACTION_EDIT_MODE_NEXT_ENTITY))
		EditMode_NextEntity(data);
	if (GameActionFlags_Get(data->pressed, ACTION_EDIT_MODE_PREV_ENTITY))
		EditMode_PrevEntity(data);
	if (GameActionFlags_Get(data->pressed, ACTION_EDIT_MODE_SELECT_NEXT))
		EditMode_OnSelectNextVariable(data);
	if (GameActionFlags_Get(data->pressed, ACTION_EDIT_MODE_SELECT_PREV))
		EditMode_OnSelectPrevVariable(data);

	if (GameActionFlags_Get(data->pressed, ACTION_EDIT_MODE_INCREASE_FINE))
		EditMode_VariableIncreaseFine(data);
	if (GameActionFlags_Get(data->pressed, ACTION_EDIT_MODE_DECREASE_FINE))
		EditMode_VariableDecreaseFine(data);
	if (GameActionFlags_Get(data->held, ACTION_EDIT_MODE_INCREASE_COARSE))
		EditMode_VariableIncreaseFine(data);
	if (GameActionFlags_Get(data->held, ACTION_EDIT_MODE_DECREASE_COARSE))
		EditMode_VariableDecreaseFine(data);
}

void EditMode_PrintMouseCoordinateText(EditModeData* data) {
	data->cursor_pos_text_size = snprintf(data->cursor_pos_text, MAX_CURSOR_TEXT_SIZE, "(%d, %d)", data->world_cursor_x, data->world_cursor_y);
}

bool EditMode_IsEntitySelected(EditModeData* edit) {
	return edit->selected_ent_index >= 0;
}

// Returns false if there is no selection.
bool EditMode_GetPrototypeEntity(EditModeData* edit, Entity** out_selected_entity) {
	if (!EditMode_IsEntitySelected(edit))
		return false;

	if (out_selected_entity) {
		*out_selected_entity = &edit->selection_ents[edit->selected_ent_index];
	}
	return true;
}

void EditMode_MoveSelectedEntityTo(EditModeData* edit, double x, double y) {
	Entity* selection;
	AABB* aabb;
	if (!EditMode_GetPrototypeEntity(edit, &selection))
		return;	
	aabb = Entity_GetAABB(selection);
	aabb->top_left_x = x;
	aabb->top_left_y = y;
}

void EditMode_MoveSelectedEntityToCursorIfSelected(EditModeData* edit) {
	if (!EditMode_IsEntitySelected(edit))
		return;

	auto cursor_x = edit->world_cursor_x;
	auto cursor_y = edit->world_cursor_y;
	auto w2s_cursor_x = 0.0;
	auto w2s_cursor_y = 0.0;

	ScreenToWorld(cursor_x, cursor_y, &w2s_cursor_x, &w2s_cursor_y);
	EditMode_MoveSelectedEntityTo(edit, w2s_cursor_x, w2s_cursor_y);
}

void EditMode_PlaceEntity(EditModeData* edit, SandGame* game) {
	Entity* ent;
	if (EditMode_GetPrototypeEntity(edit, &ent)) {
		SandGame_AddEntity(game, ent, ent->type);
	}
}

void EditMode_CheckPlace(EditModeData* edit, SandGame* game) {
	if (GameActionFlags_Get(edit->pressed, ACTION_EDIT_MODE_PLACE_ENTITY))
		EditMode_PlaceEntity(edit, game);
}

void EditMode_Update(
	EditModeData* edit, 
	SandGame* game, 
	EngineTime dt
) {
	if (!EditMode_IsEnabled(edit))		// Set directly in ReceiveInput.
		return;

	EditMode_HandleInput(edit, game);
	EditMode_MoveSelectedEntityToCursorIfSelected(edit);
	EditMode_PrintMouseCoordinateText(edit);
	EditMode_CheckPlace(edit, game);
}
