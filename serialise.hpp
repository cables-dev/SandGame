#pragma once
//⠀⠀⠀⠀⠀⠀⢠⣤⣴⣶⣶⣶⣶⣶⣶⣶⣶⣦⣤⣤⣄⣀⣀⣀⡀⠀⠀⠀⠀
//⠀⠀⠀⢠⣶⣿⣿⡿⠿⠿⠿⠛⠛⠛⠛⠛⠛⠿⠿⠿⠿⢿⣿⣿⣿⣿⡆⠀⠀
//⠀⠀⠀⢸⣿⠟⠉⠀⠀⠀⠀⠀⢀⠀⠀⠀⠀⠀⠀⣀⠀⠀⠀⠀⢸⣿⣇⠀⠀
//⠀⠀⢠⣿⣿⠀⠀⠀⠀⠀⠀⠀⠹⣦⠀⠀⠀⠀⣰⡟⠀⠀⠀⠀⢸⣿⣿⣀⠀
//⠀⠀⢸⣿⡿⠀⢰⣶⣤⣤⣄⣀⡀⠙⣃⠀⠀⡀⠛⠀⣀⣠⣤⣤⣬⣹⡟⢩⡿
//⢸⣟⢻⣿⢷⣤⣴⡟⣿⣿⣿⣿⣿⣶⣋⣀⣀⣻⣿⣿⣿⣿⣿⣿⣿⢿⣧⡟⠁
//⠀⠙⢷⣿⡀⠀⢹⣄⡙⠛⠛⠛⣿⢹⡏⠉⠙⣿⠀⠛⠻⠿⠿⢿⡇⢸⡏⠀⠀
//⠀⠀⠀⠙⡧⠀⠀⠉⠉⠛⠛⠛⠋⢸⡇⠀⠀⣿⠛⠛⠛⠛⠛⠛⠃⢸⡇⠀⠀
//⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⣸⡇⠀⠀⢻⠀⠀⠀⠀⠀⠀⠀⣸⠃⠀⠀
//⠀⠀⠀⠀⣷⡀⠀⠀⠀⠀⠀⠀⣠⣿⣤⣀⣠⣾⡇⠀⠀⠀⠀⠀⠀⡟⠀⠀⠀
//⠀⠀⠀⠀⠘⣇⠀⠀⠀⠀⣠⠀⠛⠀⠀⠉⠁⠈⠛⠰⣦⠀⠀⠀⢰⡇⠀⠀⠀
//⠀⠀⠀⠀⠀⢿⡀⠀⠀⣾⢋⣤⡴⠞⠛⠳⠞⠛⠳⣦⣌⢷⡄⢀⡾⠁⠀⠀⠀
//     ⢿⡀	    🥣		  ⡾				- "lov me cereal me"
// ⠀⠀⠀⠀⠸⣧⠀⢀⣴⣿⡛⠛⠛⠛⠛⠛⠛⠛⣛⣿⢷⣄⣼⠇⠀⠀⠀⠀
//⠀⠀⠀⠀⠀⠀⠙⣷⡟⠁⠈⠻⢦⣤⣤⣤⡤⠶⠟⠋⠁⣠⡿⠃⠀⠀⠀⠀⠀
//⠀⠀⠀⠀⠀⠀⠀⠈⠻⣶⣄⠀⠀⠾⠟⠛⠳⠆⠀⣠⣾⠋⠀⠀⠀⠀⠀⠀⠀
//⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⢷⣄⣀⣀⣀⣠⡶⠟⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀
//⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠉⠉⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀


#include "serial.hpp"

bool Serialise_SerialiseGame(
	const SandGame* game, 
	NEEDS_FREE char** out_buffer, 
	std::uint32_t* out_buffer_size,
	DeserialiseMetadata* md = nullptr
);

bool Serialise_SerialiseGameAndDumpToFile(
	const SandGame* game, 
	const char* file_path,
	DeserialiseMetadata* md = nullptr
);
