#include "level.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <corecrt_malloc.h>
#include <string.h>
#include <cassert>
#include <cstdlib>

// TODO: add error messages

#define HEADER_MAGIC "level"
#define SOUND_DEFINITION_COMMAND "sound"
#define STREAM_DEFINITION_COMMAND "soundstream"
#define GRAPHIC_DEFINITION_COMMAND "graphic"
#define ENTITY_DEFINITION_COMMAND "ent"
#define SAND_DEFINITION_COMMAND "sand"
#define PNG_FILE_EXTENSION ".png"
#define JPG_FILE_EXTENSION ".jpg"
#define JPEG_FILE_EXTENSION ".jpeg"
#define GIF_FILE_EXTENSION ".gif"

constexpr const char* IMAGE_FILE_EXTENSIONS[]{
	PNG_FILE_EXTENSION,
	JPG_FILE_EXTENSION,
	JPEG_FILE_EXTENSION
};
constexpr const char* ANIMATION_FILE_EXTENSIONS[]{
    GIF_FILE_EXTENSION
};

struct SandGameLevelFile {
    char* buffer_start;
    char* buffer;
    int buffer_size;                // Signed to prevent wraparounds when decrementing.
    bool halted;
};

// Will return false if the file could not be opened.
bool SandGameLevelFile_OpenAndCreate(SandGameLevelFile* level, const char* file_path) {
    FILE* file;
    auto err = fopen_s(&file, file_path, "rb");                 // Note: "r" -> "rb" guarantees the value returned by ftell is accurate.
    if (file == nullptr)
        return false;

    fseek(file, 0, SEEK_END); 
	auto file_size = ftell(file);
    file_size += 1;                     // +1 for null terminator.
	fseek(file, 0, SEEK_SET);

    level->buffer_start = (char*)(malloc(file_size));
    level->buffer = level->buffer_start;
    level->buffer_size = file_size;                         // TODO: fix
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
    return file->halted || SandGameLevelFile_IsEOF(file);
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
    if (*r == '\0') { // Skip next space, unless we are at EOF
        file->halted = true;
    }
    else {
        ++r;
    }
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

bool SandGameLevelFile_ReadUnsignedInteger(SandGameLevelFile* file, std::uint32_t* out_num) {
    if (SandGameLevelFile_IsHalted(file))
        return false;

    char* number_string;
    auto read_success = SandGameLevelFile_ReadString(file, &number_string, nullptr);
    if (!read_success)
        return false;

    char* parse_ptr;
    auto result = strtoul(number_string, &parse_ptr, 10);

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
    auto file_path_result = SandGameLevelFile_ReadStringLiteral(level, &file_path, nullptr);
    if (!file_path_result)
        return false;

	Audio_LoadAndSetSoundResource(audio, rsc, file_path);
}

bool Level_HandleStreamDefinition(AudioData* audio, SandGameLevelFile* level) {
    // [AUDIO_RSC] [FILE_PATH]
    AudioResource rsc{};
    auto rsc_result = SandGameLevelFile_ReadInteger(level, (long*)&rsc);
    if (!rsc_result)
        return false;

    char* file_path{};
    auto file_path_result = SandGameLevelFile_ReadStringLiteral(level, &file_path, nullptr);
    if (!file_path_result)
        return false;

	Audio_LoadAndSetStreamResource(audio, rsc, file_path);
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

    GraphicResource rsc;
    success = SandGameLevelFile_ReadInteger(level, (long*)&rsc);
    if (!success || a > 255 || a < 0) return false;

    GameColour c{};
    c.r = (std::uint8_t)r;
    c.g = (std::uint8_t)g;
    c.b = (std::uint8_t)b;
    c.a = (std::uint8_t)a;
    EntityRectangleObstacle rect;
    if (rsc >= 0)
		RectangleObstacle_Create(&rect, top_left_x, top_left_y, w, h, c, rsc);
    else
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

bool Level_HandleBarrelDefinition(SandGame* game, SandGameLevelFile* level) {
    double top_left_x;
    auto success = SandGameLevelFile_ReadDouble(level, &top_left_x);
    if (!success) return false;

    double bottom_y;
    success = SandGameLevelFile_ReadDouble(level, &bottom_y);
    if (!success) return false;

	double w;
    success = SandGameLevelFile_ReadDouble(level, &w);
    if (!success) return false;

	double h;
    success = SandGameLevelFile_ReadDouble(level, &h);
    if (!success) return false;

    GraphicResource idle_rsc;
    success = SandGameLevelFile_ReadUnsignedInteger(level, (std::uint32_t*)&idle_rsc);
    if (!success) return false;

    GraphicResource explode_rsc;
    success = SandGameLevelFile_ReadUnsignedInteger(level, (std::uint32_t*)&explode_rsc);
    if (!success) return false;

	EntityBarrel barrel;
	Barrel_Create(&barrel, top_left_x, bottom_y, w, h, idle_rsc, explode_rsc);  
    SandGame_AddEntity(game, &barrel, ENTITY_BARREL);

    return true;
}

bool Level_HandleLevelDoorDefinition(SandGame* game, SandGameLevelFile* level) {
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

    char* next_level_path;
    success = SandGameLevelFile_ReadStringLiteral(level, &next_level_path, nullptr);
    if (!success) return false;

	EntityLevelDoor door;
	LevelDoor_Create(&door, 1200, 100, 50, 100, next_level_path);
    SandGame_AddEntity(game, &door, ENTITY_LEVEL_DOOR);

    return success;
}

bool Level_HandleEntityDefinition(SandGame* game, SandGameLevelFile* level) {
    EntityType type;
    auto type_success = SandGameLevelFile_ReadInteger(level, (long*)&type);
    if (!type_success)
        return false;

    switch (type) {
		case ENTITY_RECTANGLE: { return Level_HandleRectDefinition(game, level); }
		case ENTITY_HINT_BOX: { return Level_HandleHintBoxDefinition(game, level); }
		case ENTITY_BARREL: { return Level_HandleBarrelDefinition(game, level); }
		case ENTITY_LEVEL_DOOR: { return Level_HandleLevelDoorDefinition(game, level); }
		default: { assert("Level_HandleEntityDefinition: Unaccounted entity type encountered." && false); }
    }
}

bool Level_IsImageFileExtension(const char* ext) {
    constexpr auto num_image_files = sizeof(IMAGE_FILE_EXTENSIONS) / sizeof(const char*);
    for (int i = 0; i < num_image_files; i++) {
        if (_strcmpi(ext, IMAGE_FILE_EXTENSIONS[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool Level_IsAnimationFileExtension(const char* ext) {
    constexpr auto num_animation_file_extensions = sizeof(ANIMATION_FILE_EXTENSIONS) / sizeof(const char*);
    for (int i = 0; i < num_animation_file_extensions ; i++) {
        if (_strcmpi(ext, ANIMATION_FILE_EXTENSIONS[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool Level_HandleGraphicDefinition(RenderData* data, SandGameLevelFile* level) {
	GraphicResource rsc;
    auto type_success = SandGameLevelFile_ReadUnsignedInteger(level, (std::uint32_t*)&rsc);
    if (!type_success)
        return false;

    char* file_path;
    std::uint32_t file_path_len;
    auto path_success = SandGameLevelFile_ReadStringLiteral(level, &file_path, &file_path_len);
    if (!path_success || file_path_len == 0)
        return false;

    auto* file_extension = &file_path[file_path_len - 1];
    while (file_extension > file_path && *file_extension != '.')
        --file_extension;

    if (*file_extension != '.')
        return false;

    if (Level_IsImageFileExtension(file_extension)) {
        Render_LoadAndSetImageResource(data, rsc, file_path);
    }
    else if (Level_IsAnimationFileExtension(file_extension)) {
        double refresh_period_s;
        auto period_success = SandGameLevelFile_ReadDouble(level, &refresh_period_s);
        if (!period_success)
            return false;

        Render_LoadAndSetAnimationResource(data, rsc, refresh_period_s, file_path);
    }
        
    return true;
}

void Level_PlaceSandRect(
    SandGame* game,
    std::uint32_t top_left_x,
    std::uint32_t top_left_y,
    std::uint32_t w,
    std::uint32_t h
) {
    auto grain_size = game->pit.grain_size;
    auto mid_x = (int)top_left_x + (int)w / 2;
    auto m_recip = (int)game->pit.stubbornness;
    for (int y = top_left_y; y < top_left_y + h; y++) {
        for (int x = top_left_x; x < top_left_x + w; x++) {
            auto pillar_w = (mid_x - x) / m_recip;
            if (std::abs(x - mid_x) <= pillar_w) {
                SandPit_PlaceGrain(&game->pit, x/grain_size, y/grain_size, PseudoRandom_GetU32() & 0b11);
            }
        }
    }
}

bool Level_HandleSandDefinition(SandGame* game, SandGameLevelFile* level) {
	std::uint32_t top_left_x;
    auto success = SandGameLevelFile_ReadUnsignedInteger(level, &top_left_x);
    if (!success) return false;

    std::uint32_t top_left_y;
    success = SandGameLevelFile_ReadUnsignedInteger(level, &top_left_y);
    if (!success) return false;

	std::uint32_t w;
    success = SandGameLevelFile_ReadUnsignedInteger(level, &w);
    if (!success) return false;

	std::uint32_t h;
    success = SandGameLevelFile_ReadUnsignedInteger(level, &h);
    if (!success) return false;

    Level_PlaceSandRect(game, top_left_x, top_left_y, w, h);
}

bool Level_ProcessCommand(
    AudioData* audio, RenderData* render, SandGame* game, SandGameLevelFile* level, const char* command
) {
    if (strcmp(command, SOUND_DEFINITION_COMMAND) == 0) {
        return Level_HandleSoundDefinition(audio, level);
    }
	else if (strcmp(command, STREAM_DEFINITION_COMMAND) == 0) {
        return Level_HandleStreamDefinition(audio, level);
    }
    else if (strcmp(command, ENTITY_DEFINITION_COMMAND) == 0) {
        return Level_HandleEntityDefinition(game, level);
    } 
    else if (strcmp(command, GRAPHIC_DEFINITION_COMMAND) == 0) {
        return Level_HandleGraphicDefinition(render, level);
    }
    else if (strcmp(command, SAND_DEFINITION_COMMAND) == 0) {
        return Level_HandleSandDefinition(game, level);
    }
    else {
        return false;
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

bool Level_ProcessFileHeader(SandGameLevelFile* level, double* out_player_x, double* out_player_y, std::uint32_t* out_pit_screens_horizontal, std::uint32_t* out_pit_screens_vertical, std::uint32_t* out_sand_stubbornness) {
    // level [player_x] [player_y] [pit_size_horizontal] [pit_size_vertical]
    auto result = true;

	char* command_word;
	result = SandGameLevelFile_ReadString(level, &command_word, nullptr);
    if (!result || strcmp(command_word, HEADER_MAGIC) != 0 || SandGameLevelFile_IsHalted(level))
        return false;

	result = SandGameLevelFile_ReadDouble(level, out_player_x);
    if (!result || SandGameLevelFile_IsHalted(level))
        return false;

	result = SandGameLevelFile_ReadDouble(level, out_player_y);
    if (!result || SandGameLevelFile_IsHalted(level))
        return false;

	result = SandGameLevelFile_ReadUnsignedInteger(level, out_pit_screens_horizontal);
    if (!result || SandGameLevelFile_IsHalted(level))
        return false;

	result = SandGameLevelFile_ReadUnsignedInteger(level, out_pit_screens_vertical);
    if (!result || SandGameLevelFile_IsHalted(level))
        return false;

	result = SandGameLevelFile_ReadUnsignedInteger(level, out_sand_stubbornness);
    if (!result || SandGameLevelFile_IsHalted(level))
        return false;

    return !SandGameLevelFile_IsHalted(level);
}

bool Level_LoadFromFile(AudioData* audio, RenderData* render, SandGame* game, const char* file_path) {
    auto* last_level_buffer = game->level_buffer;

    // Reset state
    SandGamePersistentState* persistent;
    SandGame_Destroy(game, &persistent);

    SandGameLevelFile level;
    auto open_result = SandGameLevelFile_OpenAndCreate(&level, file_path);
    if (!open_result)
        return false;

    // Parse header for initialisation info
    double player_x;
    double player_y;
    std::uint32_t pit_screens_horizontal;
    std::uint32_t pit_screens_vertical;
    std::uint32_t stubbornness;
    auto header_result = Level_ProcessFileHeader(&level, &player_x, &player_y, &pit_screens_horizontal, &pit_screens_vertical, &stubbornness);
    if (!header_result)
        return false;

    // Reset game
    SandGame_Create(            // TODO: Create default constructor elsewhere
        game,
        player_x,
        player_y,
        PLAYER_WIDTH,
        PLAYER_HEIGHT,
        PIT_SECTOR_WIDTH,
        PIT_SECTOR_HEIGHT,
        pit_screens_horizontal,
        pit_screens_vertical,
        stubbornness,
        SAND_SIZE,
        DEFAULT_MAX_ENTITIES,
        persistent
    );

    auto success = Level_ProcessFile(audio, render, game, &level);
    if (success) {
		if (last_level_buffer != nullptr)
			free((void*)last_level_buffer);
        game->level_buffer = level.buffer_start;

        SandGame_NotifyLevelLoaded(game);
    }

    return success;
}
