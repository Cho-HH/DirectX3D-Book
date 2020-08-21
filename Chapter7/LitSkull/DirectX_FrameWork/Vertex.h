#pragma once
#ifndef VERTEX_H
#define VERTEX_H

namespace Vertex
{
	struct PosNormal
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
	};
}

class InputLayoutDesc
{
public:
	InputLayoutDesc() = delete;
	~InputLayoutDesc() = delete;
	static const D3D11_INPUT_ELEMENT_DESC PosNormal[2];
};

class InputLayouts
{
public:
	InputLayouts() = delete;
	~InputLayouts() = delete;

	static void InitAll(ID3D11Device* device);
	static ComPtr<ID3D11InputLayout> PosNormal;
};
#endif //VERTEX_H

