#pragma once
#ifndef VERTEX_H
#define VERTEX_H

namespace Vertex
{
	struct Basic32
	{
		Basic32() : Pos(0.0f, 0.0f, 0.0f), Normal(0.0f, 0.0f, 0.0f), Tex(0.0f, 0.0f) {}
		Basic32(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT2& uv)
			: Pos(p), Normal(n), Tex(uv) {}
		Basic32(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
			: Pos(px, py, pz), Normal(nx, ny, nz), Tex(u, v) {}
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};
}

class InputLayoutDesc
{
public:
	InputLayoutDesc() = delete;
	~InputLayoutDesc() = delete;
	static const D3D11_INPUT_ELEMENT_DESC Basic32[3];
};

class InputLayouts
{
public:
	InputLayouts() = delete;
	~InputLayouts() = delete;

	static void InitAll(ID3D11Device* device);
	static ComPtr<ID3D11InputLayout> Basic32;
};
#endif //VERTEX_H

