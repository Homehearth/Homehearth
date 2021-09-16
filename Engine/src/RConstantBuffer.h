#pragma once
#include "GResource.h"
#include "EnginePCH.h"

class RConstantBuffer : public resource::GResource
{
private:

	ID3D11Buffer* m_buffer;

public:

	RConstantBuffer();

	/*
		Get the internal buffer of this Resource.
		Returns false if the internal buffer is a nullptr.
	*/
	const bool GetBuffer(ID3D11Buffer** p_pointer);

	/*
		Ignore this virtual for this Resource.
	*/
	virtual bool Create(const std::string& filename) override;

};