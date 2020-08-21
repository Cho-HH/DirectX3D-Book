#include "stdafx.h"
#include "Effects.h"

#pragma region Effect
Effect::Effect(ID3D11Device* device, const wstring& filename)
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3D10Blob> compiledShader;
	ComPtr<ID3D10Blob> compilationMsgs;
	//효과파일을 compiledShader에 담아준다.
	HRESULT hr = D3DCompileFromFile(L"FX/Basic.fx", 0, 0, 0, "fx_5_0", shaderFlags, 0, compiledShader.GetAddressOf(), compilationMsgs.GetAddressOf());

	D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
		0, device, m_FX.GetAddressOf());
}
#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const wstring& filename) :Effect(device, filename)
{
	Light1Tech		  = m_FX->GetTechniqueByName("Light1");
	Light2Tech		  = m_FX->GetTechniqueByName("Light2");
	Light3Tech		  = m_FX->GetTechniqueByName("Light3");
	WorldViewProj	  = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World			  = m_FX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = m_FX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	EyePosW			  = m_FX->GetVariableByName("gEyePosW")->AsVector();
	DirLights		  = m_FX->GetVariableByName("gDirLights");
	Mat				  = m_FX->GetVariableByName("gMaterial");
}
#pragma endregion

#pragma region Effects
shared_ptr<BasicEffect> Effects::BasicFX = nullptr;
void Effects::InitAll(ID3D11Device* device)
{
	BasicFX=make_shared<BasicEffect>(device, L"FX/Basic.fx");
}
#pragma endregion
