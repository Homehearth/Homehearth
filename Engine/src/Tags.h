#pragma once

typedef uint8_t tag_bits;

enum TagType : tag_bits
{
	CAMERA = 1,
	DEBUG_CAMERA = 2,
	DYNAMIC = 4,
	STATIC = 8,
	MAP_BOUNDS = 16,
	DEFENCE = 32,
	TEXTUREEFFECT = 64
};