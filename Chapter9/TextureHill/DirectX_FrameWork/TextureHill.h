#pragma once
#include "FrameWork.h"
#include "Waves.h"

enum RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

class TextureHill :public FrameWork
{
public:
	TextureHill(HINSTANCE hInstance);
	virtual ~TextureHill() = default;

	bool Init() override;
	void Update(float dt) override;
	void RenderFrame() override;
	void OnResize() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

	HRESULT CreateShaderResourseViewFromFile(ID3D11Device* device, const wchar_t* filename, const wchar_t* ext, ID3D11ShaderResourceView** Texture);
private:
	void BuildCrateGeometryBuffers();
	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();
	XMFLOAT3 GetHillNormal(float x, float z)const;
	float GetHillHeight(float x, float z)const;

private:
	ComPtr<ID3D11Buffer> m_LandVB;
	ComPtr<ID3D11Buffer> m_LandIB;
	ComPtr<ID3D11Buffer> m_WaveVB;
	ComPtr<ID3D11Buffer> m_WaveIB;
	ComPtr<ID3D11Buffer> m_BoxVB;
	ComPtr<ID3D11Buffer> m_BoxIB;

	ComPtr<ID3D11ShaderResourceView> m_GrassMapSRV;
	ComPtr<ID3D11ShaderResourceView> m_WavesMapSRV;
	ComPtr<ID3D11ShaderResourceView> m_BoxMapSRV;

	Waves m_Waves;

	DirectionalLight m_DirLights[3];
	Material m_LandMat;
	Material m_WavesMat;
	Material m_BoxMat;

	XMFLOAT4X4 m_LandWorld;
	XMFLOAT4X4 m_WavesWorld;
	XMFLOAT4X4 m_GrassTexTransform;
	XMFLOAT4X4 m_WavesTexTransform;
	XMFLOAT4X4 m_BoxWorld;

	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Proj;

	float m_Theta;
	float m_Phi;
	float m_Radius;

	POINT m_LastMousePos;

	RenderOptions m_RenderOptions;

	UINT m_LandIndexCount;
	XMFLOAT2 m_WaterTexOffset;

	UINT m_LightCount;

	XMFLOAT3 m_EyePosW;
};

