#pragma once
#ifndef VERTEX_H
#define VERTEX_H

namespace Vertex
{
	struct Basic32
	{
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

