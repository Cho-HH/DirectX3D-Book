#pragma once
#include "FrameWork.h"

enum RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

class Mirror :public FrameWork
{
public:
	Mirror(HINSTANCE hInstance);
	virtual ~Mirror() = default;

	bool Init() override;
	void Update(float dt) override;
	void RenderFrame() override;
	void OnResize() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

	HRESULT CreateShaderResourseViewFromFile(ID3D11Device* device, const wchar_t* filename, const wchar_t* ext, ID3D11ShaderResourceView** Texture);
private:
	void BuildRoomGeoBuffer();
	void BuildSkullGeoBuffer();

private:
	ComPtr<ID3D11Buffer> m_RoomVB;

	ComPtr<ID3D11Buffer> m_SkullVB;
	ComPtr<ID3D11Buffer> m_SkullIB;

	ComPtr<ID3D11ShaderResourceView> m_FloorMapSRV;
	ComPtr<ID3D11ShaderResourceView> m_WallMapSRV;
	ComPtr<ID3D11ShaderResourceView> m_MirrorMapSRV;

	DirectionalLight m_DirLights[3];
	Material m_RoomMat;
	Material m_SkullMat;
	Material m_MirrorMat;
	Material m_ShadowMat;

	XMFLOAT4X4 m_RoomWorld;
	XMFLOAT4X4 m_SkullWorld;

	UINT m_SkullIndexCnt;
	XMFLOAT3 m_SkullTranslation;

	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Proj;

	float m_Theta;
	float m_Phi;
	float m_Radius;

	POINT m_LastMousePos;

	RenderOptions m_RenderOptions;

	UINT m_LightCount;

	XMFLOAT3 m_EyePosW;
};

