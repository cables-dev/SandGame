#include "engine_input.hpp"
#include "../tinyfiledialogs/tinyfiledialogs.h"
#include <string.h>

// shouts out tinyfd
// NEEDS FREE
// nullptr if the window was closed...
NEEDS_FREE char* EngineInput_ShowEntryBox(const char* title, const char* message, const char* placeholder_text) {
	if (!placeholder_text)
		placeholder_text = "";
	const char* input = tinyfd_inputBox(title, message, placeholder_text);
	if (!input)
		return nullptr;
	auto input_size = strlen(input);
	auto* result = (char*)malloc(input_size + 1);
	memcpy_s(result, input_size + 1, input, input_size + 1);
	return result;
}
