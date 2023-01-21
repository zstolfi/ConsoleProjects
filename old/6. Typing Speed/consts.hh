#pragma once

#include "draw.hh"

enum GameModeType {
	MENU ,
	TYPING ,
	FINISHED
};

std::map<std::string, ConsText> sprite;

void initSprites() {
	sprite["car"] =  ConsText(
		"                                   \n"
		"     |\"\"\"/   _____                 \n"
		"     |  |__-\"    `|_______         \n"
		"     |     `      `       '''---_  \n"
		"     `/   \\_______________/   \\___\\\n"
		"      \\ _ /               \\ _ /    \n"
		,
		
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE."
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE."
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE."
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE."
		"EEEEEE8EEE8EEEEEEEEEEEEEEE8EEE8EEEE."
		"EEEEEE88888EEEEEEEEEEEEEEE88888EEEE."
	);
}