#include "EnginePCH.h"
#include "BasePass.h"

#include "PipelineManager.h"
#include "RMesh.h"

void BasePass::Initialize()
{
	// Initialize Pass.
	// accept scene.
	// accept materials.
	// accept ...
}

void BasePass::PreRender(ID3D11DeviceContext* dc, PipelineManager* pm)
{
    // INPUT ASSEMBLY.
    {
        dc->IASetInputLayout(pm->m_defaultInputLayout.Get());
        dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    // SHADER STAGES.
    {
        dc->VSSetShader(pm->m_defaultVertexShader.Get(), nullptr, 0);
        dc->PSSetShader(pm->m_defaultPixelShader.Get(), nullptr, 0);
        dc->GSSetShader(nullptr, nullptr, 0);
        dc->HSSetShader(nullptr, nullptr, 0);
        dc->DSSetShader(nullptr, nullptr, 0);
        dc->CSSetShader(nullptr, nullptr, 0);
    }

    // CONSTANT BUFFERS.
    {
        dc->PSSetConstantBuffers(0, 0, nullptr);
        dc->VSSetConstantBuffers(0, 1, pm->m_defaultModelConstantBuffer.GetAddressOf());
        dc->VSSetConstantBuffers(1, 1, m_camera->m_viewConstantBuffer.GetAddressOf());
    }

    // SHADER RESOURCES.
    {
<<<<<<< HEAD
        dc->PSSetShaderResources(0, 0, nullptr);
        dc->VSSetShaderResources(0, 0, nullptr);
=======
        dc->VSSetShaderResources(0, 0, nullptr);
        dc->PSSetShaderResources(10, 1, pm->m_depthStencilSRV.GetAddressOf());	// DepthBuffer.
>>>>>>> parent of dfc17b1 (Changes to most of the rendering system.)
        dc->PSSetSamplers(0, 1, pm->m_linearSamplerState.GetAddressOf());
    }

    // RASTERIZER.
    {
        dc->RSSetViewports(1, &pm->m_viewport);
        dc->RSSetState(pm->m_rasterStateNoCulling.Get());
    }

    // OUTPUT MERGER.
    {
<<<<<<< HEAD
        dc->OMSetRenderTargets(1, pm->m_renderTargetView.GetAddressOf(), pm->m_depthStencilView.Get());
        dc->OMSetBlendState(pm->m_blendStatepOpaque.Get(), nullptr, 0xFFFFFFFF); 
        dc->OMSetDepthStencilState(pm->m_depthStencilStateLess.Get(), 1);
=======
        dc->OMSetRenderTargets(1, pm->m_backBufferTarget.GetAddressOf(), nullptr);
        dc->OMSetBlendState(pm->m_blendStatepOpaque.Get(), nullptr, 0xFFFFFFFF); 
        dc->OMSetDepthStencilState(nullptr, 0);
>>>>>>> parent of dfc17b1 (Changes to most of the rendering system.)
    }
}

void BasePass::Render(Scene* pScene)
{
	// Render objects.
    pScene->Render();
}

void BasePass::PostRender(ID3D11DeviceContext* dc, PipelineManager* pm)
{
	// return rendertarget for next pass?
    D3D11Core::Get().DeviceContext()->UpdateSubresource(m_camera->m_viewConstantBuffer.Get(), 0, nullptr, m_camera->GetCameraMatrixes(), 0, 0);
}

void BasePass::GetCamera(Camera* camera)
{
    m_camera = camera;
}
	// Cleanup.
    ID3D11ShaderResourceView* const nullSRV[] = { nullptr };
    dc->PSSetShaderResources(10, 1, nullSRV);
}