#pragma once
#include "FrameWork.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class Shapes final : public FrameWork
{
public:
	Shapes(HINSTANCE hInstance);
	virtual ~Shapes() = default;

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

	void OutputShaderErrorMsg(ID3D10Blob* errMsg, HWND hWnd, LPCWSTR shaderfileName) const;
private:
	ComPtr<ID3D11Buffer> m_VB;
	ComPtr<ID3D11Buffer> m_IB;

	ComPtr<ID3DX11Effect> m_FX;
	ComPtr<ID3DX11EffectTechnique> m_Tech;
	ComPtr<ID3DX11EffectMatrixVariable> m_fxWorldViewProj;
	ComPtr<ID3D11InputLayout> m_InputLayout;
	ComPtr<ID3D11RasterizerState> m_WireframeRS;

	XMFLOAT4X4 m_SphereWorld[10];
	XMFLOAT4X4 m_CylWorld[10];
	XMFLOAT4X4 m_BoxWorld;
	XMFLOAT4X4 m_GridWorld;
	XMFLOAT4X4 m_CenterSphere;
	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Proj;

	float m_Theta;
	float m_Phi;
	float m_Radius;

	POINT m_LastMousePos;

	int m_BoxVertexOffset;
	int m_GridVertexOffset;
	int m_SphereVertexOffset;
	int m_CylinderVertexOffset;

	UINT m_BoxIndexOffset;
	UINT m_GridIndexOffset;
	UINT m_SphereIndexOffset;
	UINT m_CylinderIndexOffset;

	UINT m_BoxIndexCount;
	UINT m_GridIndexCount;
	UINT m_SphereIndexCount;
	UINT m_CylinderIndexCount;
};


