#pragma once
#include "FrameWork.h"
#include "Waves.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class WavesApp final : public FrameWork
{
public:
	WavesApp(HINSTANCE hInstance);
	virtual ~WavesApp() = default;

	bool Init() override;
	void Update(float dt) override;
	void RenderFrame() override;
	void OnResize() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	float GetHeight(float x, float z)const;
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
};


