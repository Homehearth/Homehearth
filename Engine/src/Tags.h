#pragma once

enum TagType : uint8_t
{
	CAMERA = 1,
	DEBUG_CAMERA = 2,
	DYNAMIC = 4,
	STATIC = 8,
	MAP_BOUNDS = 16,
	DEFENCE = 32,
	TEXTUREEFFECT = 64
};