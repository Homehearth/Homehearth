#include "ParticlePass.h"

void ParticlePass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	DC->IASetInputLayout(PM->m_ParticleInputLayout.Get());

	DC->VSSetShader(PM->m_ParticleVertexShader.Get(), nullptr, 0);
	DC->PSSetShader(PM->m_ParticlePixelShader.Get(), nullptr, 0);

	//DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());

}

void ParticlePass::Render(Scene* pScene)
{
}

void ParticlePass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
}
