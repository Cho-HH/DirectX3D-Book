#pragma once
class RenderState
{
public:
	static void InitAll(ID3D11Device* device);

	static ComPtr<ID3D11RasterizerState> WireframeRS;
	static ComPtr<ID3D11RasterizerState> NoCullRS;

	static ComPtr<ID3D11BlendState> AlphaToCoverageBS;
	static ComPtr<ID3D11BlendState> TransparentBS;
};

