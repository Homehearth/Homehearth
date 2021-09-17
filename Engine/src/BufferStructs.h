#pragma once

namespace cbuffer {
	struct PerObject {
		sm::Matrix world;
	};

	struct PerFrame {
		sm::Matrix viewProjection;
	};
}