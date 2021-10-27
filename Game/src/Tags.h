#pragma once

enum TagType : uint8_t
{
	CAMERA = (1 << 0),
	DEBUG_CAMERA = (1 << 1),
	GOOD = (1 << 2),
	BAD = (1 << 3),
	LOCAL_PLAYER = (1 << 4)
};

