#pragma once
#include "FrameWork.h"

class LitSkull :public FrameWork
{
public:
	LitSkull(HINSTANCE hInstance);
	virtual ~LitSkull() = default;

	bool Init() override;
	void Update(float dt) override;
	void RenderFrame() override;
	void OnResize() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildShapesGeometryBuffers();
	void BuildSkullGeometryBuffers();

private:
	ComPtr<ID3D11Buffer> m_ShapesVB;
	ComPtr<ID3D11Buffer> m_ShapesIB;
	ComPtr<ID3D11Buffer> m_SkullVB;
	ComPtr<ID3D11Buffer> m_SkullIB;

	DirectionalLight m_DirLights[3];
	Material m_GridMat;
	Material m_BoxMat;
	Material m_CylinderMat;
	Material m_SphereMat;
	Material m_SkullMat;

	XMFLOAT4X4 m_SphereWorld[10];
	XMFLOAT4X4 m_CylWorld[10];
	XMFLOAT4X4 m_BoxWorld;
	XMFLOAT4X4 m_GridWorld;
	XMFLOAT4X4 m_SkullWorld;

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

	UINT m_SkullIndexCount;

	UINT m_LightCount;

	XMFLOAT3 m_EyePosW;
};

