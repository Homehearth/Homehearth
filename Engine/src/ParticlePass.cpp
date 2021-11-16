#include "ParticlePass.h"

void ParticlePass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	DC->IASetInputLayout(PM->m_ParticleInputLayout.Get());

	DC->VSSetShader(PM->m_ParticleVertexShader.Get(), nullptr, 0);
	DC->GSSetShader(PM->m_ParticleGeometryShader.Get(), nullptr, 0);
	DC->PSSetShader(PM->m_ParticlePixelShader.Get(), nullptr, 0);
	DC->CSSetShader(PM->m_ParticleComputeShader.Get(), nullptr, 0);


	static UINT stride = sizeof(Particle_t);
	static UINT offset = 0;

	DC->IASetVertexBuffers(0,1, &m_nullVertexBuffer, &stride, &offset);

	//DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());

	//DC->Dispatch();
}

void ParticlePass::Render(Scene* pScene)
{
	std::vector<comp::EmitterParticle> emitters;
	pScene->ForEachComponent<comp::EmitterParticle>([&](Entity entity, comp::EmitterParticle& particleEmitter) 
	{
			emitters.push_back(particleEmitter);
	});
	
	for (auto emitter : emitters) 
	{
		D3D11Core::Get().DeviceContext()->CSSetUnorderedAccessViews(0, 1, emitter.particleUAV.GetAddressOf(), nullptr);
		//Dispatch
		D3D11Core::Get().DeviceContext()->CSSetUnorderedAccessViews(0, 1, &m_nullUAV, nullptr);


		D3D11Core::Get().DeviceContext()->VSSetShaderResources(0, 1, emitter.particleSRV.GetAddressOf());

		D3D11Core::Get().DeviceContext()->DrawInstanced(1, emitter.nrOfParticles, 0, 0);

		D3D11Core::Get().DeviceContext()->VSSetShaderResources(0, 1, &m_nullSRV);
	}

}

void ParticlePass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
	DC->VSSetShaderResources(0, 1, &m_nullSRV);
	DC->CSSetUnorderedAccessViews(0, 1, &m_nullUAV, nullptr);
}
