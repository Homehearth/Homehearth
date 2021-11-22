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

	GOOD = 128,
	BAD = 256,
	NO_RESPONSE = 1 << 9,
};