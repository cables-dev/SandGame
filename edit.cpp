#include "edit.hpp"
#include "engine/engine_input.hpp"
#include <cassert>
#include <limits>

constexpr auto MAX_AVAILABLE_ENT = ENTITY_MAX;				// for debugging 

void EditModeVariableDouble_Create(
	EditModeVariableDouble* var, 
	const char* description, 
	double* ptr, 
	bool is_signed, 
	double increment
) {
	assert(ptr);				// So we don't have to do a nullptr check on every funtion call
	var->item = ptr;
	var->description = description;
	var->is_signed = is_signed;
	var->increment = increment;
}
void EditModeVariableDouble_Increment(EditModeVariableDouble* var, double coeff = 1.0) {
	if (!var->is_signed) {
		*var->item = Bounds(*var->item + var->increment * coeff, DBL_MAX, 0.0);
	}
	else {
		*var->item += var->increment * coeff;
	}
}
void EditModeVariableDouble_Destroy(EditModeVariableDouble* var) { /*pass*/ }
void EditModeVariableDouble_IncreaseFine(EditModeVariableDouble* var) {	EditModeVariableDouble_Increment(var); }
void EditModeVariableDouble_IncreaseCoarse(EditModeVariableDouble* var) { EditModeVariableDouble_Increment(var, 5); }
void EditModeVariableDouble_DecreaseFine(EditModeVariableDouble* var) { EditModeVariableDouble_Increment(var, -1.0); }
void EditModeVariableDouble_DecreaseCoarse(EditModeVariableDouble* var) { EditModeVariableDouble_Increment(var, -5.0); }
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
void EditModeVariableGraphicResource_IncreaseFine(EditModeVariableGraphicResource* var) { *var->item = Bounds(*var->item + 1, MAX_GRAPHIC_RESOURCES, -1); }
void EditModeVariableGraphicResource_IncreaseCoarse(EditModeVariableGraphicResource* var) { *var->item = Bounds(*var->item + 1, MAX_GRAPHIC_RESOURCES, -1); }
void EditModeVariableGraphicResource_DecreaseFine(EditModeVariableGraphicResource* var) { *var->item = Bounds(*var->item - 1, MAX_GRAPHIC_RESOURCES, -1); }
void EditModeVariableGraphicResource_DecreaseCoarse(EditModeVariableGraphicResource* var) { *var->item = Bounds(*var->item - 1, MAX_GRAPHIC_RESOURCES, -1); }
bool EditModeVariableGraphicResource_OnSelectNext(EditModeVariableGraphicResource* var) { return true; }			// Return true if we permit the parent to cycle to the next variable
bool EditModeVariableGraphicResource_OnSelectPrev(EditModeVariableGraphicResource* var) { return true; }			// Return true if we permit the parent to cycle to the next variable

void EditModeVariableAudioResource_Create(EditModeVariableAudioResource* var, const char* description, AudioResource* ptr) {
	assert(ptr);				// So we don't have to do a nullptr check on every funtion call
	var->item = ptr;
	var->description = description;
}
void EditModeVariableAudioResource_Destroy(EditModeVariableAudioResource* var) { /*pass*/ }
void EditModeVariableAudioResource_IncreaseFine(EditModeVariableAudioResource* var) { *var->item = Bounds(*var->item + 1, MAX_GRAPHIC_RESOURCES, -1); }
void EditModeVariableAudioResource_IncreaseCoarse(EditModeVariableAudioResource* var) { *var->item = Bounds(*var->item + 1, MAX_GRAPHIC_RESOURCES, -1); }
void EditModeVariableAudioResource_DecreaseCoarse(EditModeVariableAudioResource* var) { *var->item = Bounds(*var->item - 1, MAX_GRAPHIC_RESOURCES, -1); }
void EditModeVariableAudioResource_DecreaseFine(EditModeVariableAudioResource* var) { *var->item = Bounds(*var->item - 1, MAX_GRAPHIC_RESOURCES, -1); }
bool EditModeVariableAudioResource_OnSelectNext(EditModeVariableAudioResource* var) { return true; }			// Return true if we permit the parent to cycle to the next variable
bool EditModeVariableAudioResource_OnSelectPrev(EditModeVariableAudioResource* var) { return true; }			// Return true if we permit the parent to cycle to the next variable

void EditModeVariableBitwise_Create(EditModeVariableBitwise* var, const char* description, int* ptr, int msb_pos) {
	assert(ptr);				// So we don't have to do a nullptr check on every funtion call
	var->item = ptr;
	var->msb_pos = msb_pos;
	var->description = description;
}
void EditModeVariableBitwise_Destroy(EditModeVariableBitwise* var) { /*pass*/ }
bool EditModeVariableBitwise_GetBitValue(EditModeVariableBitwise* var, int bit_pos) {
	if (bit_pos >= var->msb_pos)
		return false;
	return *var->item & (1 << bit_pos);
}
void EditModeVariableBitwise_SetBitValue(EditModeVariableBitwise* var, int bit_pos, bool to) {
	if (bit_pos >= var->msb_pos)
		return;
	if (to)
		*var->item |= (1 << bit_pos);
	else
		*var->item &= ~(1 << bit_pos);
}
bool EditModeVariableBitwise_GetSelectedBit(EditModeVariableBitwise* var) { return EditModeVariableBitwise_GetBitValue(var, var->cursor_idx); }
void EditModeVariableBitwise_SetSelectedBit(EditModeVariableBitwise* var, bool to) { EditModeVariableBitwise_SetBitValue(var, var->cursor_idx, to); }
void EditModeVariableBitwise_ToggleSelectedBit(EditModeVariableBitwise* var) { EditModeVariableBitwise_SetBitValue(var, var->cursor_idx, !EditModeVariableBitwise_GetSelectedBit(var)); }
bool EditModeVariableBitwise_IsBitSelected(EditModeVariableBitwise* var, int bit_idx) {
	return bit_idx == var->cursor_idx;
}
void EditModeVariableBitwise_IncreaseFine(EditModeVariableBitwise* var) { EditModeVariableBitwise_ToggleSelectedBit(var); }
void EditModeVariableBitwise_IncreaseCoarse(EditModeVariableBitwise* var) { EditModeVariableBitwise_ToggleSelectedBit(var); }
void EditModeVariableBitwise_DecreaseFine(EditModeVariableBitwise* var) { EditModeVariableBitwise_ToggleSelectedBit(var); }
void EditModeVariableBitwise_DecreaseCoarse(EditModeVariableBitwise* var) { EditModeVariableBitwise_ToggleSelectedBit(var); }
bool EditModeVariableBitwise_OnSelectNext(EditModeVariableBitwise* var) { 
	auto result = var->cursor_idx == var->msb_pos - 1;
	if (!result)
		var->cursor_idx = (var->cursor_idx + 1) % var->msb_pos;		
	return result;
}			// Return true if we permit the parent to cycle to the next variable
bool EditModeVariableBitwise_OnSelectPrev(EditModeVariableBitwise* var) { 
	auto result = var->cursor_idx == 0;
	if (!result)
		var->cursor_idx = (var->cursor_idx - 1) % var->msb_pos;
	return result;
}			// Return true if we permit the parent to cycle to the next variable

void EditModeVariableCString_Create(EditModeVariableCString* var, const char* description, char** ptr) {
	assert(ptr);				// So we don't have to do a nullptr check on every funtion call
	var->item = ptr;
	var->description = description;
	var->needs_free = false;
}
void EditModeVariableCString_Destroy(EditModeVariableCString* var) { 
	if (var->needs_free) {
		free(*var->item);
	}
}
void EditModeVariableCString_UpdateValue(EditModeVariableCString* var) {
	auto* new_value = EngineInput_ShowEntryBox(var->description, "Enter a new value:", *var->item);
	if (new_value != nullptr)
		*var->item = new_value;
}
void EditModeVariableCString_IncreaseFine(EditModeVariableCString* var) { EditModeVariableCString_UpdateValue(var); }
void EditModeVariableCString_IncreaseCoarse(EditModeVariableCString* var) { EditModeVariableCString_UpdateValue(var); }
void EditModeVariableCString_DecreaseCoarse(EditModeVariableCString* var) { EditModeVariableCString_UpdateValue(var); }
void EditModeVariableCString_DecreaseFine(EditModeVariableCString* var) { EditModeVariableCString_UpdateValue(var); }
bool EditModeVariableCString_OnSelectNext(EditModeVariableCString* var) { return true; }			// Return true if we permit the parent to cycle to the next variable
bool EditModeVariableCString_OnSelectPrev(EditModeVariableCString* var) { return true; }			// Return true if we permit the parent to cycle to the next variable


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
	case EDIT_MODE_VAR_STRING: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableCString)); break; } 	
	case EDIT_MODE_VAR_BITWISE: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableBitwise)); break; } 	
	default: { assert(false && "EditModeVariable_CreateFrom: Unaccounted variable type encountered!"); }
	}
	edit_var->type = type;
	edit_var->next = nullptr;
	edit_var->prev = nullptr;
}

void EditModeVariable_CreateFrom(EditModeVariable* edit_var, void* var, EditModeVariableType type, GraphicResource* display_resource_ptr) {
	switch(type) {
	case EDIT_MODE_VAR_DOUBLE: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableDouble)); break; }
	case EDIT_MODE_VAR_U32: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableUnsignedInteger)); break; }
	case EDIT_MODE_VAR_INT: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableInteger)); break; }
	case EDIT_MODE_VAR_COLOUR: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableColour)); break; } 
	case EDIT_MODE_VAR_BOOLEAN: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableBoolean)); break; }
	case EDIT_MODE_VAR_GRAPHIC_RESOURCE: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableGraphicResource)); break; }
	case EDIT_MODE_VAR_AUDIO_RESOURCE: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableAudioResource)); break; } 
	case EDIT_MODE_VAR_STRING: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableCString)); break; } 	
	case EDIT_MODE_VAR_BITWISE: { memcpy_s(&edit_var->var, sizeof(edit_var->var), var, sizeof(EditModeVariableBitwise)); break; } 	
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
	case EDIT_MODE_VAR_STRING: { return EditModeVariableCString_OnSelectNext(&var->var.var_c_string); }
	case EDIT_MODE_VAR_BITWISE: { return EditModeVariableBitwise_OnSelectNext(&var->var.var_bitwise); }
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
	case EDIT_MODE_VAR_STRING: { return EditModeVariableCString_OnSelectPrev(&var->var.var_c_string); }
	case EDIT_MODE_VAR_BITWISE: { return EditModeVariableBitwise_OnSelectPrev(&var->var.var_bitwise); }
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
	case EDIT_MODE_VAR_STRING: { EditModeVariableCString_IncreaseFine(&var->var.var_c_string); break; }
	case EDIT_MODE_VAR_BITWISE: { EditModeVariableBitwise_IncreaseFine(&var->var.var_bitwise); break; }
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
	case EDIT_MODE_VAR_STRING: { EditModeVariableCString_IncreaseCoarse(&var->var.var_c_string); break; }
	case EDIT_MODE_VAR_BITWISE: { EditModeVariableBitwise_IncreaseCoarse(&var->var.var_bitwise); break; }
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
	case EDIT_MODE_VAR_STRING: { EditModeVariableCString_DecreaseFine(&var->var.var_c_string); break; }
	case EDIT_MODE_VAR_BITWISE: { EditModeVariableBitwise_DecreaseFine(&var->var.var_bitwise); break; }
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
	case EDIT_MODE_VAR_STRING: { EditModeVariableCString_DecreaseCoarse(&var->var.var_c_string); break; }
	case EDIT_MODE_VAR_BITWISE: { EditModeVariableBitwise_DecreaseCoarse(&var->var.var_bitwise); break; }
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
	selections[ENTITY_RECTANGLE].ent = Entity_CreateFrom(&rect, ENTITY_RECTANGLE);

	EntityHintBox hint;
	HintBox_Create(&hint, "Edit me!", false, 0.0, 0.0, 50.0, 50.0);
	selections[ENTITY_HINT_BOX].ent = Entity_CreateFrom(&hint, ENTITY_HINT_BOX);

	EntityBarrel barrel;
	Barrel_Create(&barrel, 0.0, 0.0, 80.0, 100.0, GRAPHIC_RSC_BARREL_KURT_IDLE, GRAPHIC_RSC_BARREL_KURT_EXPLODE);
	selections[ENTITY_BARREL].ent = Entity_CreateFrom(&barrel, ENTITY_BARREL);

	EntityLevelDoor door;
	LevelDoor_Create(&door, 0.0, 0.0, 50, 100, "nowhere.sg", GRAPHIC_RSC_DOOR, 0, 0);
	selections[ENTITY_LEVEL_DOOR].ent = Entity_CreateFrom(&door, ENTITY_LEVEL_DOOR);

	EntityLadybird ladybird;
	Ladybird_Create(&ladybird, 0.0, 0.0);
	selections[ENTITY_LADYBIRD].ent = Entity_CreateFrom(&ladybird, ENTITY_LADYBIRD);
}

PrototypeEntity* EditMode_GetPrototypeEntityPrototypeFromIndex(EditModeData* edit, int index) {
	return &edit->selection_ents[index];
}

Entity* EditMode_GetPrototypeEntityFromIndex(EditModeData* edit, int index) {
	return &edit->selection_ents[index].ent;
}

void EditMode_SetupRectangleVariables(EditModeData* data) {
	auto rect_index = ENTITY_RECTANGLE;
	auto* rect_ent = &EditMode_GetPrototypeEntityFromIndex(data, rect_index)->entity.rect;
	EditModeVariableDouble width_var_inner;
	EditModeVariableDouble height_var_inner;
	EditModeVariableColour colour_var_inner;
	EditModeVariableGraphicResource rsc_var_inner;
	EditModeVariable* width_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* height_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* colour_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* rsc_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariableDouble_Create(&width_var_inner, "Width: ", &rect_ent->aabb.w, false, SAND_SIZE);
	EditModeVariableDouble_Create(&height_var_inner, "Height : ", &rect_ent->aabb.h, false, SAND_SIZE);
	EditModeVariableColour_Create(&colour_var_inner, "Colour : ", &rect_ent->colour);
	EditModeVariableGraphicResource_Create(&rsc_var_inner, "Sprite: ", &rect_ent->graphic);
	EditModeVariable_CreateFrom(width_var, &width_var_inner, EDIT_MODE_VAR_DOUBLE);
	EditModeVariable_CreateFrom(height_var, &height_var_inner, EDIT_MODE_VAR_DOUBLE);
	EditModeVariable_CreateFrom(colour_var, &colour_var_inner, EDIT_MODE_VAR_COLOUR);
	EditModeVariable_CreateFrom(rsc_var, &rsc_var_inner, EDIT_MODE_VAR_GRAPHIC_RESOURCE);

	EditMode_InitVariableList(data, width_var, rect_index);
	EditMode_PushVariableToList(data, height_var, rect_index);
	EditMode_PushVariableToList(data, colour_var, rect_index);
	EditMode_PushVariableToList(data, rsc_var, rect_index);
}

void EditMode_SetupHintBoxVariables(EditModeData* data) {
	auto hint_index = ENTITY_HINT_BOX;
	auto* hint_ent = &EditMode_GetPrototypeEntityFromIndex(data, hint_index)->entity.hint_box;
	EditModeVariableDouble width_var_inner;
	EditModeVariableDouble height_var_inner;
	EditModeVariableBoolean only_once_inner;
	EditModeVariableAudioResource audio_rsc_inner;
	EditModeVariableCString popup_text_inner;
	EditModeVariable* width_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* height_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* only_once_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* audio_rsc_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* popup_text_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariableDouble_Create(&width_var_inner, "Width: ", &hint_ent->aabb.w, false, SAND_SIZE);
	EditModeVariableDouble_Create(&height_var_inner, "Height : ", &hint_ent->aabb.h, false, SAND_SIZE);
	EditModeVariableBoolean_Create(&only_once_inner, "Trigger Only Once: ", &hint_ent->only_once);
	EditModeVariableAudioResource_Create(&audio_rsc_inner, "Sound Effect: ", &hint_ent->audio_rsc);
	EditModeVariableCString_Create(&popup_text_inner, "Hint: ", (char**)&hint_ent->message);
	EditModeVariable_CreateFrom(width_var, &width_var_inner, EDIT_MODE_VAR_DOUBLE);
	EditModeVariable_CreateFrom(height_var, &height_var_inner, EDIT_MODE_VAR_DOUBLE);
	EditModeVariable_CreateFrom(only_once_var, &only_once_inner, EDIT_MODE_VAR_BOOLEAN);
	EditModeVariable_CreateFrom(audio_rsc_var, &audio_rsc_inner, EDIT_MODE_VAR_AUDIO_RESOURCE);
	EditModeVariable_CreateFrom(popup_text_var, &popup_text_inner, EDIT_MODE_VAR_STRING);

	EditMode_InitVariableList(data, width_var, hint_index);
	EditMode_PushVariableToList(data, height_var, hint_index);
	EditMode_PushVariableToList(data, only_once_var, hint_index);
	EditMode_PushVariableToList(data, audio_rsc_var, hint_index);
	EditMode_PushVariableToList(data, popup_text_var, hint_index);
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
	EditModeVariableDouble_Create(&width_var_inner, "Width: ", &barrel_ent->aabb.w, false, SAND_SIZE);
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
	auto door_index = ENTITY_LEVEL_DOOR;
	auto* door_ent = &EditMode_GetPrototypeEntityFromIndex(data, door_index)->entity.door;
	EditModeVariableDouble width_var_inner;
	EditModeVariableDouble height_var_inner;
	EditModeVariableCString next_level_path_inner;
	EditModeVariableBitwise lock_flags_inner;
	EditModeVariableBitwise unlock_flags_inner;
	EditModeVariable* width_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* height_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* next_level_path_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* lock_flags_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariable* unlock_flags_var = (EditModeVariable*)malloc(sizeof(EditModeVariable));
	EditModeVariableDouble_Create(&width_var_inner, "Width: ", &door_ent->aabb.w, false, SAND_SIZE);
	EditModeVariableDouble_Create(&height_var_inner, "Height: ", &door_ent->aabb.h, false, SAND_SIZE);
	EditModeVariableCString_Create(&next_level_path_inner, "Next Level File: ", (char**)&door_ent->next_level_path);
	EditModeVariableBitwise_Create(&lock_flags_inner, "Lock Flags: ", &door_ent->lock_flag, 32);
	EditModeVariableBitwise_Create(&unlock_flags_inner, "Unlock Flags: ", &door_ent->unlock_flag, 32);
	EditModeVariable_CreateFrom(width_var, &width_var_inner, EDIT_MODE_VAR_DOUBLE);
	EditModeVariable_CreateFrom(height_var, &height_var_inner, EDIT_MODE_VAR_DOUBLE);
	EditModeVariable_CreateFrom(next_level_path_var, &next_level_path_inner, EDIT_MODE_VAR_STRING);
	EditModeVariable_CreateFrom(lock_flags_var, &lock_flags_inner, EDIT_MODE_VAR_BITWISE);
	EditModeVariable_CreateFrom(unlock_flags_var, &unlock_flags_inner, EDIT_MODE_VAR_BITWISE);

	EditMode_InitVariableList(data, width_var, door_index);
	EditMode_PushVariableToList(data, height_var, door_index);
	EditMode_PushVariableToList(data, next_level_path_var, door_index);
	EditMode_PushVariableToList(data, lock_flags_var, door_index);
	EditMode_PushVariableToList(data, unlock_flags_var, door_index);
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

void EditMode_SetToast(EditModeData* data, const char* toast, bool needs_free) {
	if (data->toast_needs_free)
		free((char*)data->toast_text);
	data->toast_text = toast;
	data->toast_needs_free = needs_free;
}

void EditMode_Create(EditModeData* data, int grain_size) {
	data->active = false;
	data->grain_size = grain_size;
	data->pressed = NULL_ACTION_FLAGS;
	data->held = NULL_ACTION_FLAGS;
	data->screen_cursor_x = 0;
	data->toast_needs_free = false;
	data->screen_cursor_y = 0;
	data->toast_text = nullptr;
	data->selected_ent_index = -1;
	memset(data->cursor_pos_text, 0, sizeof(data->cursor_pos_text + 1) / sizeof(char));	// + 1 for null terminator
	EditMode_CreateEntitySelection(data);
	EditMode_SetupVariables(data);
}

void EditMode_Destroy(EditModeData* data){
	data->active = false;
	EditMode_SetToast(data, "", false);
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
		EditMode_SetToast(data, "EDIT MODE ENABLED. Press B to disable. Q for help.", false);
	}
	else {
		EditMode_SetToast(data, "Edit mode disabled.", false);
	}
}

void EditMode_NoToggle(EditModeData* data) {
	EditMode_SetToast(data, nullptr, false);
}

void EditMode_ReceiveInput(EditModeData* data, GameActionFlags* in_out_pressed, GameActionFlags* in_out_held, int cursor_x, int cursor_y) {
	data->pressed = *in_out_pressed;
	data->held = *in_out_held;
	data->screen_cursor_x = cursor_x;
	data->screen_cursor_y = cursor_y;

	if (GameActionFlags_Get(data->pressed, ACTION_TOGGLE_EDIT_MODE))		// We need to do our enabled check in here
		EditMode_Toggle(data);												// instead of update so we can decide to 
	else																	// steal inputs or not
		EditMode_NoToggle(data);

	if (EditMode_IsEnabled(data))
		EditMode_StealInputs(in_out_pressed, in_out_held);
}

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
	if (current_var && EditModeVariable_OnSelectNext(current_var)) {			// Permission to cycle?
		data->ent_variable_list_selections[ent_index] = current_var->next;
		//current_var = 
	}
}

void EditMode_OnSelectPrevVariable(EditModeData* data) {
	if (!EditMode_IsEntitySelected(data))
		return;

	auto ent_index = data->selected_ent_index;
	auto* current_var = data->ent_variable_list_selections[ent_index];
	if (current_var && EditModeVariable_OnSelectPrev(current_var)) {			// Permission to cycle?
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

	// Snap to grid
	data->world_cursor_x = (data->world_cursor_x / data->grain_size) * data->grain_size;
	data->world_cursor_y = (data->world_cursor_y / data->grain_size) * data->grain_size;

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
		*out_selected_entity = EditMode_GetPrototypeEntityFromIndex(edit, edit->selected_ent_index);
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

// Reset entity variables so that it appears as expected in the world
void EditMode_PrepareEntityForPlace(Entity* e) {
	auto* ent = &e->entity;
	switch (e->type) {
	case ENTITY_BARREL: {
		auto* barrel = &ent->barrel;
		barrel->active_sprite = barrel->idle_sprite;
		barrel->last_sprite = -1;
		break;
	}
	}
}

void EditMode_PlaceEntity(EditModeData* edit, SandGame* game) {
	Entity* ent;
	if (EditMode_GetPrototypeEntity(edit, &ent)) {
		EditMode_PrepareEntityForPlace(ent);
		SandGame_AddEntity(game, ent, ent->type);
	}
}

void EditMode_CheckPlace(EditModeData* edit, SandGame* game) {
	if (GameActionFlags_Get(edit->pressed, ACTION_EDIT_MODE_PLACE_ENTITY))
		EditMode_PlaceEntity(edit, game);
}

void EditMode_CheckToggleHelpText(EditModeData* edit) {
	if (GameActionFlags_Get(edit->pressed, ACTION_EDIT_MODE_TOGGLE_HELP)) {
		edit->show_help = !edit->show_help;
	}
}

void EditMode_DeleteEntityUnderCursor(EditModeData* edit, SandGame* game) {
	static SandGame* _game;
	static double _cursor_x;
	static double _cursor_y;
	static bool _finished;
	_game = game;
	_finished = false;							// Only remove the first entity.

	ScreenToWorld(edit->world_cursor_x, edit->world_cursor_y, &_cursor_x, &_cursor_y);
	auto cb = [](Entity* ent) {
		auto* aabb = Entity_GetAABB(ent);
		if (AABB_ContainsPoint(aabb, _cursor_x, _cursor_y) && !_finished) {		// Contains mouse cursor?
			SandGame_RemoveEntity(_game, ent);
			_finished = true;
		}
	};
	SandGame_ForEachEntity(game, cb);
}

void EditMode_CheckRemoveEntity(EditModeData* edit, SandGame* game) {
	if (GameActionFlags_Get(edit->pressed, ACTION_EDIT_MODE_DELETE)) {
		EditMode_DeleteEntityUnderCursor(edit, game);
	}
}

void EditMode_DumpToFile(EditModeData* edit, SandGame* game, DeserialiseMetadata* md) {
	auto* filename = CreateRandomFileName("levels\\", ".sg");
	char* buffer;
	int snprintf_chars;
	Serialise_SerialiseGameAndDumpToFile(game, filename, md);

	snprintf_chars = snprintf(nullptr, 0, "Saved to file: %s", filename);
	buffer = (char*)(malloc(snprintf_chars + 2));
	snprintf_chars = snprintf(buffer, snprintf_chars + 2, "Saved to file: %s", filename);
	EditMode_SetToast(edit, buffer, true);
	free(filename);
}

void EditMode_CheckDumpFile(EditModeData* edit, SandGame* game, DeserialiseMetadata* md) {
	if (GameActionFlags_Get(edit->pressed, ACTION_EDIT_MODE_SAVE_FILE) && md != nullptr) {
		EditMode_DumpToFile(edit, game, md);
	}
}

void EditMode_Update(
	EditModeData* edit, 
	SandGame* game, 
	DeserialiseMetadata* resource_info,
	EngineTime dt
) {
	if (!EditMode_IsEnabled(edit))		// Set directly in ReceiveInput.
		return;

	EditMode_HandleInput(edit, game);
	EditMode_CheckToggleHelpText(edit);
	EditMode_CheckRemoveEntity(edit, game);
	EditMode_PrintMouseCoordinateText(edit);
	EditMode_MoveSelectedEntityToCursorIfSelected(edit);
	EditMode_CheckPlace(edit, game);
	EditMode_CheckDumpFile(edit, game, resource_info);
}
