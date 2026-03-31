#pragma once
#include "common.hpp"
#include "game.hpp"

struct DeserialiseError {
	const char* message;
	const char* last_buffer;
};


// Returns remaining buffer length after parse
int Serial_SerialiseRectangleObstacle(const EntityRectangleObstacle* rect, char** out_serial, int remaining_buffer_size);
bool Serial_DeserialiseRectangleObstacle(EntityRectangleObstacle* out_rect, char** serial, DeserialiseError* out_error);

int Serial_SerialiseHintBox(const EntityHintBox* hint, char** out_serial, int remaining_buffer_size);
bool Serial_DeserialiseHintBox(EntityHintBox* out_hint, char** serial, DeserialiseError* out_error);

int Serial_SerialiseBarrel(const EntityBarrel* barrel, char** out_serial, int remaining_buffer_size);
bool Serial_DeserialiseBarrel(EntityBarrel* barrel, char** serial, DeserialiseError* out_error);

int Serial_SerialiseLevelDoor(const EntityLevelDoor* door, char** out_serial, int remaining_buffer_size);
bool Serial_DeserialiseLevelDoor(EntityLevelDoor* out_door, char** serial, DeserialiseError* out_error);

int Serial_SerialiseLadybird(const EntityLadybird* ladybird, char** out_serial, int remaining_buffer_size);
bool Serial_DeserialiseLadybird(EntityLadybird* out_ladybird, char** serial, DeserialiseError* out_error);
