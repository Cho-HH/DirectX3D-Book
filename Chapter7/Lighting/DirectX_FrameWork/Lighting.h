#pragma once
#include "FrameWork.h"
#include "Waves.h"
#include "LightHelper.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
};

class Lighting final : public FrameWork
{
public:
	Lighting(HINSTANCE hInstance);
	virtual ~Lighting() = default;

	bool Init() override;
	void Update(float dt) override;
	void RenderFrame() override;
	void OnResize() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	float GetHeight(float x, float z)const;
	XMFLOAT3 GetHillNormal(float x, float z)const;
	void BuildGeometryBuffers();
	void BuildWavesGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

	void OutputShaderErrorMsg(ID3D10Blob* errMsg, HWND hWnd, LPCWSTR shaderfileName) const;
private:
	ComPtr<ID3D11Buffer> m_LandVB;
	ComPtr<ID3D11Buffer> m_LandIB;
	ComPtr<ID3D11Buffer> m_WavesVB;
	ComPtr<ID3D11Buffer> m_WavesIB;

	ComPtr<ID3DX11Effect> m_FX;
	ComPtr<ID3DX11EffectTechnique> m_Tech;
	ComPtr<ID3DX11EffectMatrixVariable> m_fxWorldViewProj;
	ComPtr<ID3DX11EffectMatrixVariable> m_fxWorld;
	ComPtr<ID3DX11EffectMatrixVariable> m_fxWorldInvTranspose;
	ComPtr<ID3DX11EffectVectorVariable> m_fxEyePosW;
	ComPtr<ID3DX11EffectVariable> m_fxDirLight;
	ComPtr<ID3DX11EffectVariable> m_fxPointLight;
	ComPtr<ID3DX11EffectVariable> m_fxSpotLight;
	ComPtr<ID3DX11EffectVariable> m_fxMaterial;

	ComPtr<ID3D11InputLayout> m_InputLayout;

	ComPtr<ID3D11RasterizerState> m_WireframeRS;

	XMFLOAT4X4 m_GridWorld;
	XMFLOAT4X4 m_WaveWorld;
	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Proj;

	float m_Theta;
	float m_Phi;
	float m_Radius;

	POINT m_LastMousePos;

	UINT m_GridIndexCount;

	Waves m_Wave;
	DirectionalLight m_DirLight;
	PointLight m_PointLight;
	SpotLight m_SpotLight;
	Material m_LandMat;
	Material m_WavesMat;

	XMFLOAT3 m_EyePosW;
};


