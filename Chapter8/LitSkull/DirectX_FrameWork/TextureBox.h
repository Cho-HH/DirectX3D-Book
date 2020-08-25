#pragma once
#include "FrameWork.h"

class TextureBox :public FrameWork
{
public:
	TextureBox(HINSTANCE hInstance);
	virtual ~TextureBox() = default;

	bool Init() override;
	void Update(float dt) override;
	void RenderFrame() override;
	void OnResize() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

	HRESULT CreateShaderResourseViewFromFile(ID3D11Device* device, const wchar_t* filename,const wchar_t* ext, ID3D11ShaderResourceView** Texture);
private:
	void BuildShapesGeometryBuffers();

private:
	ComPtr<ID3D11Buffer> m_BoxVB;
	ComPtr<ID3D11Buffer> m_BoxIB;

	ComPtr<ID3D11ShaderResourceView> m_DiffuseMapSRV;

	DirectionalLight m_DirLights[3];
	Material m_BoxMat;
	
	XMFLOAT4X4 m_BoxWorld;
	XMFLOAT4X4 m_TexTransform;

	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Proj;

	float m_Theta;
	float m_Phi;
	float m_Radius;

	POINT m_LastMousePos;

	int m_BoxVertexOffset;
	UINT m_BoxIndexOffset;
	UINT m_BoxIndexCount;

	UINT m_LightCount;

	XMFLOAT3 m_EyePosW;
};

