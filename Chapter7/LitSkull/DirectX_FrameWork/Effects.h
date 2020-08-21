#pragma once
#ifndef EFFECTS_H
#define EFFECTS_H

#pragma region Effect
class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename);
	Effect(const Effect& rhs) = delete;
	Effect& operator=(const Effect& rhs) = delete;
	virtual ~Effect() = default;

protected:
	ComPtr<ID3DX11Effect> m_FX;
};
#pragma endregion

#pragma region BasicEffect
class BasicEffect final : public Effect
{
public:
	BasicEffect(ID3D11Device* device, const wstring& filename);
	virtual ~BasicEffect() = default;

	ComPtr<ID3DX11EffectTechnique> Light1Tech;
	ComPtr<ID3DX11EffectTechnique> Light2Tech;
	ComPtr<ID3DX11EffectTechnique> Light3Tech;

	ComPtr<ID3DX11EffectMatrixVariable> WorldViewProj;
	ComPtr<ID3DX11EffectMatrixVariable> World;
	ComPtr<ID3DX11EffectMatrixVariable> WorldInvTranspose;
	ComPtr<ID3DX11EffectVectorVariable> EyePosW;
	ComPtr<ID3DX11EffectVariable> DirLights;
	ComPtr<ID3DX11EffectVariable> Mat;

	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }
};
#pragma endregion

#pragma region Effects
class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	
	static shared_ptr<BasicEffect> BasicFX;
};
#pragma endregion

#endif //EFFECTS_H