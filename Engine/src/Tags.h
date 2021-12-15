#pragma once

typedef uint16_t tag_bits;

enum TagType : tag_bits
{
	CAMERA = 1,
	DEBUG_CAMERA = 2,
	DYNAMIC = 4,
	STATIC = 8,
	MAP_BOUNDS = 16,
	DEFENCE = 32,
	RANGED_ATTACK = 64,
	GOOD = 128,
	BAD = 256,
	ENEMY_ATTACK = 512,
	PLAYER_ATTACK = 1024,
	NO_RESPONSE = 2048

};