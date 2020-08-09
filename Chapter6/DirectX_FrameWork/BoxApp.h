#pragma once
#include "FrameWork.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class BoxApp final : public FrameWork
{
public:
	BoxApp(HINSTANCE hInstance);
	virtual ~BoxApp() = default;

	bool Init() override;
	void Update(float dt) override;
	void RenderFrame() override;
	void OnResize() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private:
	ComPtr<ID3D11Buffer> m_BoxVB;
	ComPtr<ID3D11Buffer> m_BoxIB;

	ComPtr<ID3DX11Effect> m_FX;
	ComPtr<ID3DX11EffectTechnique> m_Tech;
	ComPtr<ID3DX11EffectMatrixVariable> m_fxWorldViewProj;
	ComPtr<ID3D11InputLayout> m_InputLayout;

	XMFLOAT4X4 m_World;
	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Proj;

	float m_Theta;
	float m_Phi;
	float m_Radius;

	POINT m_LastMousePos;
};

