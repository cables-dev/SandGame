#include "level.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <corecrt_malloc.h>
#include <string.h>
#include <cassert>

#define SOUND_DEFINITION_COMMAND "sound"
#define ENTITY_DEFINITION_COMMAND "ent"

struct SandGameLevelFile {
    char* buffer_start;
    char* buffer;
    int buffer_size;                // Signed to prevent wraparounds when decrementing.
    bool halted;
};

// Will return false if the file could not be opened.
bool SandGameLevelFile_OpenAndCreate(SandGameLevelFile* level, const char* file_path) {
    FILE* file;
    auto err = fopen_s(&file, file_path, "r");
    if (file == nullptr)
        return false;

    fseek(file, 0, SEEK_END); 
	auto file_size = ftell(file);
    file_size += 1;                     // +1 for null terminator.
	fseek(file, 0, SEEK_SET);

    level->buffer_start = (char*)(malloc(file_size));
    level->buffer = level->buffer_start;
    level->buffer_size = file_size;
    fread(level->buffer_start, 1, file_size, file);
    level->buffer_start[file_size - 1] = '\0';
    level->halted = false;

    fclose(file);
    return true;
}

void SandGameLevelFile_Destroy(SandGameLevelFile* level) {
    free(level->buffer_start);
}

bool SandGameLevelFile_IsEOF(SandGameLevelFile* file) {
    return file->buffer_size <= 0;
}

bool SandGameLevelFile_IsHalted(SandGameLevelFile* file) {
    return file->halted;
}

bool SandGameLevelFile_ReadStringLiteral(SandGameLevelFile* file, char** out_buffer, std::uint32_t* out_buffer_size) {
    if (SandGameLevelFile_IsEOF(file) || SandGameLevelFile_IsHalted(file)) {
        file->halted = true;
        return false;
    }

    auto* l = file->buffer;
    if (*l != '\"')
        return false;
    ++l;

    auto* r = l;
    while (*r != '\"' && *r != '\0') {
        ++r;
    }

    // End of file stream (We expected a terminator for our string...)
    if (*r == '\0') {
        file->halted = true;
        return false;
    }
    else
		*r = '\0';

    if (out_buffer)
		*out_buffer = l;
    if (out_buffer_size)
		*out_buffer_size = r  - l;

    ++r;        // Skip terminator "
    ++r;        // Skip next space 
    file->buffer = r;
    file->buffer_size -= r - l;
    return true;
}

// out_buffer will point into the file contents.
bool SandGameLevelFile_ReadString(SandGameLevelFile* file, char** out_buffer, std::uint32_t* out_buffer_size) {
    if (SandGameLevelFile_IsEOF(file) || SandGameLevelFile_IsHalted(file)) {
        file->halted = true;
        return false;
    }

    auto* l = file->buffer;
    auto* r = l;
    while (*r != ' ' && *r != '\n' && *r != '\0') {
        ++r;
    }

    // End of file stream
    if (*r == '\0')
        file->halted = true;
    else
		*r = '\0';

    if (out_buffer)
		*out_buffer = l;
    if (out_buffer_size)
		*out_buffer_size = r - l;

    ++r;
    file->buffer = r;
    file->buffer_size -= r - l;
    return true;
}

bool SandGameLevelFile_ReadInteger(SandGameLevelFile* file, long* out_num) {
    if (SandGameLevelFile_IsHalted(file))
        return false;

    char* number_string;
    auto read_success = SandGameLevelFile_ReadString(file, &number_string, nullptr);
    if (!read_success)
        return false;

    char* parse_ptr;
    auto result = strtol(number_string, &parse_ptr, 10);

    if (*parse_ptr != '\0') {
        file->halted = true;
        return false;
    }

    *out_num = result;
    return true;
}

bool SandGameLevelFile_ReadDouble(SandGameLevelFile* file, double* out_num) {
    if (SandGameLevelFile_IsHalted(file))
        return false;

    char* number_string;
    auto read_success = SandGameLevelFile_ReadString(file, &number_string, nullptr);
    if (!read_success)
        return false;

    char* parse_ptr;
    auto result = strtod(number_string, &parse_ptr);

    if (*parse_ptr != '\0') {
        file->halted = true;
        return false;
    }

    *out_num = result;
    return true;
}

bool Level_HandleSoundDefinition(AudioData* audio, SandGameLevelFile* level) {
    // [AUDIO_RSC] [FILE_PATH]
    AudioResource rsc{};
    auto rsc_result = SandGameLevelFile_ReadInteger(level, (long*)&rsc);
    if (!rsc_result)
        return false;

    char* file_path{};
    auto file_path_result = SandGameLevelFile_ReadString(level, &file_path, nullptr);
    if (!file_path_result)
        return false;

	Audio_LoadAndSetResource(audio, rsc, file_path);
}

bool Level_HandleRectDefinition(SandGame* game, SandGameLevelFile* level) {
    // [top_left_x:double] [top_left_y:double] [w:double] [h:double] [r:int] [g:int] [b:int] [a:int]
    double top_left_x;
    auto success = SandGameLevelFile_ReadDouble(level, &top_left_x);
    if (!success) return false;

    double top_left_y;
    success = SandGameLevelFile_ReadDouble(level, &top_left_y);
    if (!success) return false;

	double w;
    success = SandGameLevelFile_ReadDouble(level, &w);
    if (!success) return false;

	double h;
    success = SandGameLevelFile_ReadDouble(level, &h);
    if (!success) return false;

    long r;
    success = SandGameLevelFile_ReadInteger(level, &r);
    if (!success || r > 255 || r < 0) return false;

    long g;
    success = SandGameLevelFile_ReadInteger(level, &g);
    if (!success || g > 255 || g < 0) return false;

    long b;
    success = SandGameLevelFile_ReadInteger(level, &b);
    if (!success || b > 255 || b < 0) return false;

    long a;
    success = SandGameLevelFile_ReadInteger(level, &a);
    if (!success || a > 255 || a < 0) return false;

    GameColour c{};
    c.r = (std::uint8_t)r;
    c.g = (std::uint8_t)g;
    c.b = (std::uint8_t)b;
    c.a = (std::uint8_t)a;
    EntityRectangleObstacle rect;
    RectangleObstacle_Create(&rect, top_left_x, top_left_y, w, h, c);
    SandGame_AddEntity(game, &rect, ENTITY_RECTANGLE);

    return true;
}

bool Level_HandleHintBoxDefinition(SandGame* game, SandGameLevelFile* level) {
    double top_left_x;
    auto success = SandGameLevelFile_ReadDouble(level, &top_left_x);
    if (!success) return false;

    double top_left_y;
    success = SandGameLevelFile_ReadDouble(level, &top_left_y);
    if (!success) return false;

	double w;
    success = SandGameLevelFile_ReadDouble(level, &w);
    if (!success) return false;

	double h;
    success = SandGameLevelFile_ReadDouble(level, &h);
    if (!success) return false;

    char* message;
    success = SandGameLevelFile_ReadStringLiteral(level, &message, nullptr);
    if (!success) return false;

    long only_once;
    success = SandGameLevelFile_ReadInteger(level, &only_once);
    if (!success || (only_once != 1 && only_once != 0)) return false;

    EntityHintBox hint;
    HintBox_Create(&hint, message, only_once, top_left_x, top_left_y, w, h);
    SandGame_AddEntity(game, &hint, ENTITY_HINT_BOX);

    return true;
}

bool Level_HandleEntityDefinition(SandGame* game, SandGameLevelFile* level) {
    EntityType type;
    auto type_success = SandGameLevelFile_ReadInteger(level, (long*)&type);
    if (!type_success)
        return false;

    switch (type) {
		case ENTITY_RECTANGLE: { return Level_HandleRectDefinition(game, level); }
		case ENTITY_HINT_BOX: { return Level_HandleHintBoxDefinition(game, level); }
		default: { assert("Level_HandleEntityDefinition: Unaccounted entity type encountered." && false); }
    }
}

bool Level_ProcessCommand(
    AudioData* audio, RenderData* render, SandGame* game, SandGameLevelFile* level, const char* command
) {
    if (strcmp(command, SOUND_DEFINITION_COMMAND) == 0) {
        return Level_HandleSoundDefinition(audio, level);
    }
    else if (strcmp(command, ENTITY_DEFINITION_COMMAND) == 0) {
        return Level_HandleEntityDefinition(game, level);
    }
}

bool Level_ProcessFile(AudioData* audio, RenderData* render, SandGame* game, SandGameLevelFile* level) {
    while (!SandGameLevelFile_IsHalted(level)) {
        char* command_word;
        auto read_result = SandGameLevelFile_ReadString(level, &command_word, nullptr);
        if (!read_result)
            return false;

        auto process_success = Level_ProcessCommand(audio, render, game, level, command_word);
        if (!process_success)
            return false;
    }
    return true;
}

bool Level_LoadFromFile(AudioData* audio, RenderData* render, SandGame* game, const char* file_path) {
    if (game->level_buffer != nullptr)
        free((void*)game->level_buffer);

    SandGameLevelFile level;
    auto open_result = SandGameLevelFile_OpenAndCreate(&level, file_path);
    if (!open_result)
        return false;

    auto success = Level_ProcessFile(audio, render, game, &level);
    if (success)
        game->level_buffer = level.buffer_start;

    return success;
}
