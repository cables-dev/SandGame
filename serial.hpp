#pragma once
#include "common.hpp"
#include "game.hpp"
#include "render.hpp"
#include "engine/engine_audio.hpp"
#include <string>

struct DeserialiseError {
	const char* message;
	const char* last_buffer;
};

struct DeserialiseMetadata {		// Store paths to resources since they are discarded once they are loaded into memory.
	const char* graphic_resource_paths[MAX_GRAPHIC_RESOURCES]{};	// Point into loaded file memory. Requires that file 
	const char* audio_resource_paths[MAX_GRAPHIC_RESOURCES]{};		// contents are stored and kept alive in game->level_buffer.
};

constexpr auto SERIALISE_HEADER_MAGIC = "level";
constexpr auto SERIALISE_SOUND_DEFINITION_COMMAND = "sound";
constexpr auto SERIALISE_STREAM_DEFINITION_COMMAND = "soundstream";
constexpr auto SERIALISE_GRAPHIC_DEFINITION_COMMAND = "graphic";
constexpr auto SERIALISE_ENTITY_DEFINITION_COMMAND = "ent";
constexpr auto SERIALISE_SAND_DEFINITION_COMMAND = "sand";

// We admit the use of C++ strings here because:
// 1. Passing a char** with remaining buffer size requires us to bubble string resizing up the stack (which is 
//    ugly) since we require the pointer to the start of the buffer in order to memfree (nicely). std::string 
//	  unifies append and resize operations and thus avoids this issue.
// 2. Creating any configuration which solves the above issue will be equivalent to std::string in any case
//    (i.e {char* buffer, int index, int capacity} or {char* buffer, char* cursor, int capacity})
// 3. Using std::string inspires more confidence than SandGameString for example. Also prevents DRY violations.
void Serial_TrySerialiseInteger(int num, std::string& buffer);
void Serial_TrySerialiseStringLiteral(const char* literal, std::string& buffer);
void Serial_TrySerialiseStringRaw(const char* raw, std::string& buffer);
void Serial_TrySerialiseDouble(double num, std::string& buffer);
void Serial_TrySerialiseUnsignedInteger(std::uint32_t num, std::string& buffer);
void Serial_TrySerialiseColour(const GameColour* c, std::string& buffer);
void Serial_TrySerialiseBoolean(bool val, std::string& buffer);
void Serial_TrySerialiseAudioResource(AudioResource rsc, std::string& buffer);
void Serial_TrySerialiseGraphicResource(GraphicResource rsc, std::string& buffer);
void Serial_TryNewline(std::string & buffer);

void Serial_SerialiseRectangleObstacle(const EntityRectangleObstacle* rect, std::string& out_serial);
bool Serial_DeserialiseRectangleObstacle(EntityRectangleObstacle* out_rect, char** serial, DeserialiseError* out_error);

void Serial_SerialiseHintBox(const EntityHintBox* hint, std::string& out_serial);
bool Serial_DeserialiseHintBox(EntityHintBox* out_hint, char** serial, DeserialiseError* out_error);

void Serial_SerialiseBarrel(const EntityBarrel* barrel, std::string& out_serial);
bool Serial_DeserialiseBarrel(EntityBarrel* barrel, char** serial, DeserialiseError* out_error);

void Serial_SerialiseLevelDoor(const EntityLevelDoor* door, std::string& out_serial);
bool Serial_DeserialiseLevelDoor(EntityLevelDoor* out_door, char** serial, DeserialiseError* out_error);

void Serial_SerialiseLadybird(const EntityLadybird* ladybird, std::string& out_serial);
bool Serial_DeserialiseLadybird(EntityLadybird* out_ladybird, char** serial, DeserialiseError* out_error);
