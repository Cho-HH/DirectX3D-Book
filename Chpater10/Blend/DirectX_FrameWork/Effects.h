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

	ComPtr<ID3DX11EffectTechnique> Light0TexTech;
	ComPtr<ID3DX11EffectTechnique> Light1TexTech;
	ComPtr<ID3DX11EffectTechnique> Light2TexTech;
	ComPtr<ID3DX11EffectTechnique> Light3TexTech;

	ComPtr<ID3DX11EffectTechnique> Light0TexAlphaClipTech;
	ComPtr<ID3DX11EffectTechnique> Light1TexAlphaClipTech;
	ComPtr<ID3DX11EffectTechnique> Light2TexAlphaClipTech;
	ComPtr<ID3DX11EffectTechnique> Light3TexAlphaClipTech;

	ComPtr<ID3DX11EffectTechnique> Light1FogTech;
	ComPtr<ID3DX11EffectTechnique> Light2FogTech;
	ComPtr<ID3DX11EffectTechnique> Light3FogTech;
	
	ComPtr<ID3DX11EffectTechnique> Light0TexFogTech;
	ComPtr<ID3DX11EffectTechnique> Light1TexFogTech;
	ComPtr<ID3DX11EffectTechnique> Light2TexFogTech;
	ComPtr<ID3DX11EffectTechnique> Light3TexFogTech;
	
	ComPtr<ID3DX11EffectTechnique> Light0TexAlphaClipFogTech;
	ComPtr<ID3DX11EffectTechnique> Light1TexAlphaClipFogTech;
	ComPtr<ID3DX11EffectTechnique> Light2TexAlphaClipFogTech;
	ComPtr<ID3DX11EffectTechnique> Light3TexAlphaClipFogTech;

	ComPtr<ID3DX11EffectMatrixVariable> WorldViewProj;
	ComPtr<ID3DX11EffectMatrixVariable> World;
	ComPtr<ID3DX11EffectMatrixVariable> WorldInvTranspose;
	ComPtr<ID3DX11EffectVectorVariable> EyePosW;
	ComPtr<ID3DX11EffectVariable> DirLights;
	ComPtr<ID3DX11EffectVariable> Mat;
	ComPtr<ID3DX11EffectShaderResourceVariable> DiffuseMap;
	ComPtr<ID3DX11EffectMatrixVariable> TexTransform;
	ComPtr<ID3DX11EffectVectorVariable> FogColor;
	ComPtr<ID3DX11EffectScalarVariable> FogStart;
	ComPtr<ID3DX11EffectScalarVariable> FogRange;

	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f) { FogStart->SetFloat(f); }
	void SetFogRange(float f) { FogRange->SetFloat(f); }
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