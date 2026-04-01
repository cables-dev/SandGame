#include "deserialise.hpp"
#include <cassert>

constexpr auto* PNG_FILE_EXTENSION = ".png";
constexpr auto* JPG_FILE_EXTENSION = ".jpg";
constexpr auto* JPEG_FILE_EXTENSION = ".jpeg";
constexpr auto* GIF_FILE_EXTENSION = ".gif";

constexpr const char* IMAGE_FILE_EXTENSIONS[]{
	PNG_FILE_EXTENSION,
	JPG_FILE_EXTENSION,
	JPEG_FILE_EXTENSION
};
constexpr const char* ANIMATION_FILE_EXTENSIONS[]{
    GIF_FILE_EXTENSION
};

// Will return false if the file could not be opened.
bool SandGameLevelFile_OpenAndRead(const char* file_path, NEEDS_FREE char** out_buffer) {
    if (!out_buffer)
        return false;

    FILE* file;
    auto err = fopen_s(&file, file_path, "rb");                 // Note: "r" -> "rb" guarantees the value returned by ftell is accurate.
    if (file == nullptr)
        return false;

    fseek(file, 0, SEEK_END); 
	auto file_size = ftell(file);
    file_size += 1;                     // +1 for null terminator.
	fseek(file, 0, SEEK_SET);

    auto* buffer = (char*)(malloc(file_size));
    fread(buffer, 1, file_size, file);
    buffer[file_size - 1] = '\0';          // null-terminate file (not done automatically by OS)
    fclose(file);

    *out_buffer = buffer;
    return true;
}

bool Level_ProcessFileHeader(
    char** in_out_buffer, 
    double* out_player_x, 
    double* out_player_y, 
    std::uint32_t* out_pit_screens_horizontal, 
    std::uint32_t* out_pit_screens_vertical, 
    std::uint32_t* out_sand_stubbornness, 
    bool* out_do_tick,
    DeserialiseError* out_error
) {
    // level [player_x] [player_y] [pit_size_horizontal] [pit_size_vertical] [stubbornness] [do_timer]
    auto result = true;
    StringChomper chomp;
    StringChomper_Create(&chomp, *in_out_buffer);

	char* magic;
	StringChomper_SkipWhitespace(&chomp);
    StringChomper_ReadString(&chomp, &magic, nullptr);
    if (strcmp(magic, SERIALISE_HEADER_MAGIC) != 0) {
        *out_error = DeserialiseError{ "Could not find header magic!", StringChomper_GetPointer(&chomp)};
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
	result = StringChomper_ReadDouble(&chomp, out_player_x);
    if (!result) {
        *out_error = DeserialiseError{ "Expected double for player_start_x in header!", StringChomper_GetPointer(&chomp) };
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
	result = StringChomper_ReadDouble(&chomp, out_player_y);
    if (!result) {
        *out_error = DeserialiseError{ "Expected double for player_start_y in header!", StringChomper_GetPointer(&chomp) };
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
	result = StringChomper_ReadUnsignedInteger(&chomp, out_pit_screens_horizontal);
    if (!result) {
        *out_error = DeserialiseError{ "Expected uint for pit_screens_horizontal in header!", StringChomper_GetPointer(&chomp) };
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
	result = StringChomper_ReadUnsignedInteger(&chomp, out_pit_screens_vertical);
    if (!result) {
        *out_error = DeserialiseError{ "Expected uint for pit_screens_vertical in header!", StringChomper_GetPointer(&chomp) };
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
	result = StringChomper_ReadUnsignedInteger(&chomp, out_sand_stubbornness);
    if (!result) {
        *out_error = DeserialiseError{ "Expected uint for sand_stubbornness in header!", StringChomper_GetPointer(&chomp) };
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
	result = StringChomper_ReadBoolean(&chomp, out_do_tick);
    if (!result) {
        *out_error = DeserialiseError{ "Expected bool for do_tick in header!", StringChomper_GetPointer(&chomp) };
        return false;
    }

    *in_out_buffer = StringChomper_GetPointer(&chomp);
    return true;
}

void Level_ResetGame(
    SandGame* game,
    double player_start_x,
    double player_start_y,
    std::uint32_t pit_screens_horizontal,
    std::uint32_t pit_screens_vertical,
    std::uint32_t sand_stubbornness,
    bool do_tick
) {
    // Reset state
    auto* last_level_buffer = game->level_buffer;
    SandGamePersistentState* persistent;
    SandGame_Destroy(game, &persistent);

    // Reset game
    SandGame_Create(            // TODO: Create default constructor elsewhere
        game,
        player_start_x,
        player_start_y,
        PLAYER_WIDTH,
        PLAYER_HEIGHT,
        PIT_SECTOR_WIDTH,
        PIT_SECTOR_HEIGHT,
        pit_screens_horizontal,
        pit_screens_vertical,
        sand_stubbornness,
        SAND_SIZE,
        DEFAULT_MAX_ENTITIES,
        persistent,
        do_tick
    );

	if (last_level_buffer != nullptr)
		free((void*)last_level_buffer);
}

bool Level_HandleSoundDefinition(
    StringChomper* chomp, 
    EngineAudioData* audio, 
    DeserialiseMetadata* out_metadata,
    DeserialiseError* out_error
) {
    // [AUDIO_RSC] [FILE_PATH]
    AudioResource rsc{};
    char* file_path{};
    bool result{};

    StringChomper_SkipWhitespace(chomp);
    result = StringChomper_ReadInteger(chomp, (int*)&rsc);
    if (!result) {
        *out_error = DeserialiseError{ "Expected an integer for audio_rsc in sound definition!", StringChomper_GetPointer(chomp) };
        return false;
    }
    StringChomper_SkipWhitespace(chomp);
    result = StringChomper_ReadStringLiteral(chomp, &file_path, nullptr);
    if (!result) {
        *out_error = DeserialiseError{ "Expected a file path in sound definition!", StringChomper_GetPointer(chomp) };
        return false;
    }

	EngineAudio_LoadAndSetSoundResource(audio, rsc, file_path);
    // Store path in metadata. No need to deepcopy under the lifetime guarantee of SandGame.
    out_metadata->audio_resource_paths[rsc] = file_path;
    return true;
}

bool Level_HandleStreamDefinition(
    StringChomper* chomp,
    EngineAudioData* audio,
    DeserialiseMetadata* out_metadata,
    DeserialiseError* out_error
) {
    // [AUDIO_RSC] [FILE_PATH]
    AudioResource rsc{};
    char* file_path{};
    bool result{};

    StringChomper_SkipWhitespace(chomp);
    result = StringChomper_ReadInteger(chomp, (int*)&rsc);
    if (!result) {
        *out_error = DeserialiseError{ "Expected integer for audio_resource in sound stream definition!", StringChomper_GetPointer(chomp) };
        return false;
    }

    StringChomper_SkipWhitespace(chomp);
    result = StringChomper_ReadStringLiteral(chomp, &file_path, nullptr);
    if (!result) {
        *out_error = DeserialiseError{ "Expected string for file_path in sound stream definition!", StringChomper_GetPointer(chomp) };
        return false;
    }

	EngineAudio_LoadAndSetStreamResource(audio, rsc, file_path);
    // Store path in metadata. No need to deepcopy under the lifetime guarantee of SandGame.
    out_metadata->audio_resource_paths[rsc] = file_path;
    return true;
}

bool Level_HandleEntityDefinition(    
    StringChomper* chomp,
    SandGame* game,
    DeserialiseError* out_error
) {
    EntityType type{};
    bool result{};

    StringChomper_SkipWhitespace(chomp);
    result = StringChomper_ReadUnsignedInteger(chomp, (std::uint32_t*)&type);
    if (!result)
        return false;

    StringChomper_SkipWhitespace(chomp);
    switch (type) {
		case ENTITY_RECTANGLE: { 
            EntityRectangleObstacle rect;
            result = Serial_DeserialiseRectangleObstacle(&rect, &chomp->ptr, out_error);
            if (result)
				SandGame_AddEntity(game, &rect, ENTITY_RECTANGLE);
            return result;
		}
		case ENTITY_HINT_BOX: {
            EntityHintBox hint;
            result = Serial_DeserialiseHintBox(&hint, &chomp->ptr, out_error);
            if (result)
				SandGame_AddEntity(game, &hint, ENTITY_HINT_BOX);
            return result;
        }
		case ENTITY_BARREL: { 
            EntityBarrel barrel;
            result = Serial_DeserialiseBarrel(&barrel, &chomp->ptr, out_error);
            if (result)
				SandGame_AddEntity(game, &barrel, ENTITY_BARREL);
            return result;
        }
		case ENTITY_LEVEL_DOOR: { 
            EntityLevelDoor door;
            result = Serial_DeserialiseLevelDoor(&door, &chomp->ptr, out_error);
            if (result)
				SandGame_AddEntity(game, &door, ENTITY_LEVEL_DOOR);
            return result;
        }
		case ENTITY_LADYBIRD: { 
            EntityLadybird ladybird;
            result = Serial_DeserialiseLadybird(&ladybird, &chomp->ptr, out_error);
            if (result)
				SandGame_AddEntity(game, &ladybird, ENTITY_LADYBIRD);
            return result;
        }
        default: {
            assert("Level_HandleEntityDefinition: Unaccounted entity type encountered." && false);
            return false;
        }
    }
}

bool Deserialise_IsImageFileExtension(const char* ext) {
    constexpr auto num_image_files = sizeof(IMAGE_FILE_EXTENSIONS) / sizeof(const char*);
    for (int i = 0; i < num_image_files; i++) {
        if (_strcmpi(ext, IMAGE_FILE_EXTENSIONS[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool Deserialise_IsAnimationFileExtension(const char* ext) {
    constexpr auto num_animation_file_extensions = sizeof(ANIMATION_FILE_EXTENSIONS) / sizeof(const char*);
    for (int i = 0; i < num_animation_file_extensions ; i++) {
        if (_strcmpi(ext, ANIMATION_FILE_EXTENSIONS[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool Level_GetFileExtensionFromFilePath(
    const char* file_path, 
    std::uint32_t file_path_len, 
    const char** out_file_extension
) {
    auto* file_path_back = &file_path[file_path_len - 1];
    while (file_path_back > file_path && *file_path_back != '.')
        --file_path_back;

    if (*file_path_back != '.')
        return false;

    if (out_file_extension)
        *out_file_extension = file_path_back;
}

bool Level_HandleGraphicDefinition(    
    StringChomper* chomp,
    RenderData* data,
    DeserialiseMetadata* out_meta,
    DeserialiseError* out_error
) {
	GraphicResource rsc{};
	char* file_path{};
    bool result{};
    auto file_path_len = 0u;
    const char* file_extension{};

    StringChomper_SkipWhitespace(chomp);
    result = StringChomper_ReadUnsignedInteger(chomp, (std::uint32_t*)&rsc);
    if (!result) {
        *out_error = DeserialiseError{ "Expected integer value for graphic resource! ", StringChomper_GetPointer(chomp) };
        return false;
    }

    StringChomper_SkipWhitespace(chomp);
    result = StringChomper_ReadStringLiteral(chomp, &file_path, &file_path_len);
    if (!result || file_path_len == 0) {
        *out_error = DeserialiseError{ "Expected file path for graphic resource! ", StringChomper_GetPointer(chomp) };
        return false;
    }

    Level_GetFileExtensionFromFilePath(file_path, file_path_len, &file_extension);
    if (Deserialise_IsImageFileExtension(file_extension)) {
        EngineRender_LoadAndSetImageResource(&data->engine, rsc, file_path);
    }
    else if (Deserialise_IsAnimationFileExtension(file_extension)) {
        double refresh_period_s;

		StringChomper_SkipWhitespace(chomp);
        result = StringChomper_ReadDouble(chomp, &refresh_period_s);
        if (!result) {
            *out_error = DeserialiseError{ "Expected refresh period for animated graphic resource!", StringChomper_GetPointer(chomp) };
            return false;
        }

        EngineRender_LoadAndSetAnimationResource(&data->engine, rsc, refresh_period_s, file_path);
    }
    else {
        *out_error = DeserialiseError{ "Invalid file type given for graphic resource!", StringChomper_GetPointer(chomp) };
        return false;
    }
    out_meta->graphic_resource_paths[rsc] = file_path;
    return true;
}

bool Level_HandleSandDefinition(
    StringChomper* chomp,
    SandGame* game, 
    DeserialiseError* out_error
) {
	std::uint32_t top_left_x;
    std::uint32_t top_left_y;
	std::uint32_t r;
    bool result;

    StringChomper_SkipWhitespace(chomp);
    result = StringChomper_ReadUnsignedInteger(chomp, &top_left_x);
    if (!result) {
        *out_error = { "Expected uint for top_left_x in sand definition", StringChomper_GetPointer(chomp) };
        return false;
    }

    StringChomper_SkipWhitespace(chomp);
    result = StringChomper_ReadUnsignedInteger(chomp, &top_left_y);
    if (!result) {
        *out_error = { "Expected uint for top_left_y in sand definition", StringChomper_GetPointer(chomp) };
        return false;
    }

    StringChomper_SkipWhitespace(chomp);
    result = StringChomper_ReadUnsignedInteger(chomp, &r);
    if (!result) {
        *out_error = { "Expected uint for radius in sand definition", StringChomper_GetPointer(chomp) };
        return false;
    }

    PlaceSandCircle(&game->pit, top_left_x/game->pit.grain_size, top_left_y/game->pit.grain_size, r/game->pit.grain_size, nullptr);
    return true;
}

bool Level_ProcessCommand(
    char* command,
    StringChomper* chomp,
    EngineAudioData* audio,
    RenderData* render,
    SandGame* game,
    DeserialiseMetadata* out_meta,
    DeserialiseError* out_error
) {
	if (strcmp(command, SERIALISE_SOUND_DEFINITION_COMMAND) == 0) {
        return Level_HandleSoundDefinition(chomp, audio, out_meta, out_error);
    }
	else if (strcmp(command, SERIALISE_STREAM_DEFINITION_COMMAND) == 0) {
        return Level_HandleStreamDefinition(chomp, audio, out_meta, out_error);
    }
    else if (strcmp(command, SERIALISE_ENTITY_DEFINITION_COMMAND) == 0) {
        return Level_HandleEntityDefinition(chomp, game, out_error);
    } 
    else if (strcmp(command, SERIALISE_GRAPHIC_DEFINITION_COMMAND) == 0) {
        return Level_HandleGraphicDefinition(chomp, render, out_meta, out_error);
    }
    else if (strcmp(command, SERIALISE_SAND_DEFINITION_COMMAND) == 0) {
        return Level_HandleSandDefinition(chomp, game, out_error);
    }
    else {
        *out_error = DeserialiseError{ "Unknown command encountered...", StringChomper_GetPointer(chomp)};
        return false;
    }
}

bool Level_ProcessFile(
    EngineAudioData* audio,
    RenderData* render,
    SandGame* game,
    DeserialiseMetadata* out_meta,
    char** in_out_file_buffer,
    DeserialiseError* out_error
) {
	StringChomper chomp;
    StringChomper_Create(&chomp, *in_out_file_buffer);

    while (StringChomper_Peek(&chomp) != '\0') {
        char* command;
        StringChomper_SkipWhitespace(&chomp);
        StringChomper_ReadString(&chomp, &command, nullptr);

        if (!Level_ProcessCommand(command, &chomp, audio, render, game, out_meta, out_error))
            return false;
    }

    *in_out_file_buffer = StringChomper_GetPointer(&chomp);
    return true;
}

bool Level_DeserialiseFile(
	EngineAudioData* audio, 
	RenderData* render, 
	SandGame* game, 
	DeserialiseMetadata* out_meta, 
	const char* file_path,
    DeserialiseError* out_error
) {
    auto player_x = 0.0;
    auto player_y = 0.0;
    auto pit_screens_horizontal = 0u;
    auto pit_screens_vertical = 0u;
    auto stubbornness = 0u;
    auto do_tick = false;
    char* level_buffer_top;
    if (!SandGameLevelFile_OpenAndRead(file_path, &level_buffer_top)) {
        *out_error = DeserialiseError{ "Could not open file for reading!", nullptr };
        return false;
    }
    char* level_buffer = level_buffer_top;

    // Parse header for initialisation info
    if (!Level_ProcessFileHeader(&level_buffer, &player_x, &player_y, &pit_screens_horizontal, &pit_screens_vertical, &stubbornness, &do_tick, out_error)) {
        free(level_buffer_top);
        return false;
    }

    Level_ResetGame(
        game,
        player_x,
        player_y,
        pit_screens_horizontal,
        pit_screens_vertical,
        stubbornness,
        do_tick
    );

    if (!Level_ProcessFile(audio, render, game, out_meta, &level_buffer, out_error)) {
        free(level_buffer_top);
        return false;
    }

    // Ensure level_buffer is loaded into sandgame to keep alive all the references we have just made.
    game->level_buffer = level_buffer_top;
    return true;
}
