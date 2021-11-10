#pragma once
#pragma once
#include "EnginePCH.h"

enum class MODE
{
	OFF,
	WRITE,
	MASK
};

class Stencil
{
	
private:

	ComPtr<ID3D11DepthStencilState> m_depthStencil;

public:

	Stencil();
	~Stencil();

	void Initialize(MODE mode);
	void Bind();

;
};

