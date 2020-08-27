#include "stdafx.h"
#include "TextureBox.h"
#include "Vertex.h"
#include "Effects.h"

TextureBox::TextureBox(HINSTANCE hInstance):
	FrameWork(hInstance),
	m_Theta(1.5f * MathHelper::Pi),
	m_Phi(0.4f * MathHelper::Pi),
	m_Radius(2.5f)
{
	m_MainWndCaption = L"TextureBox";

	m_LastMousePos.x = 0;
	m_LastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_BoxWorld, I);
	XMStoreFloat4x4(&m_TexTransform, I);
	XMStoreFloat4x4(&m_View, I);
	XMStoreFloat4x4(&m_Proj, I);

	m_DirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_DirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_DirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	m_DirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	m_DirLights[1].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[1].Diffuse = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	m_DirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	m_DirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

	m_BoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_BoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_BoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
}

bool TextureBox::Init()
{
	if (!FrameWork::Init())
		return false;

	wchar_t ext[_MAX_EXT];
	wchar_t name[_MAX_EXT];
	wchar_t drive[_MAX_EXT];
	wchar_t dir[_MAX_EXT];

	Effects::InitAll(m_spDevice.Get());
	InputLayouts::InitAll(m_spDevice.Get());

	wchar_t filename[_MAX_EXT] = L"Textures/WoodCrate01.dds";

	_wsplitpath_s(filename, drive, dir, name, ext);
	CreateShaderResourseViewFromFile(m_spDevice.Get(), filename, ext, m_DiffuseMapSRV.GetAddressOf());

	BuildShapesGeometryBuffers();

	return true;
}

void TextureBox::Update(float dt)
{
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_View, V);
}

void TextureBox::RenderFrame()
{
	float color[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
	m_spDeviceCon->ClearRenderTargetView(m_spTargetView.Get(), color);
	m_spDeviceCon->ClearDepthStencilView(m_spDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	m_spDeviceCon->IASetInputLayout(InputLayouts::Basic32.Get());
	m_spDeviceCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&m_View);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);
	XMMATRIX ViewProj = view * proj;

	Effects::BasicFX->SetDirLights(m_DirLights);
	Effects::BasicFX->SetEyePosW(m_EyePosW);

	ComPtr<ID3DX11EffectTechnique> activeTech = Effects::BasicFX->Light2TexTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_spDeviceCon->IASetVertexBuffers(0, 1, m_BoxVB.GetAddressOf(), &stride, &offset);
		m_spDeviceCon->IASetIndexBuffer(m_BoxIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&m_BoxWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldviewproj = world * ViewProj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldviewproj);
		Effects::BasicFX->SetMaterial(m_BoxMat);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&m_TexTransform));
		Effects::BasicFX->SetDiffuseMap(m_DiffuseMapSRV.Get());

		activeTech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(m_BoxIndexCount, m_BoxIndexOffset, m_BoxVertexOffset);
	}

	m_spSwapchain->Present(0, 0);
}

void TextureBox::OnResize()
{
	FrameWork::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, P);
}

void TextureBox::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	SetCapture(m_hWnd);
}

void TextureBox::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void TextureBox::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

		m_Theta += dx;
		m_Phi += dy;

		m_Phi = MathHelper::Clamp(m_Phi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = 0.005f * static_cast<float>(x - m_LastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - m_LastMousePos.y);

		m_Radius += dx - dy;

		m_Radius = MathHelper::Clamp(m_Radius, 3.0f, 15.0f);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

HRESULT TextureBox::CreateShaderResourseViewFromFile(ID3D11Device* device, const wchar_t* filename, const wchar_t* ext, ID3D11ShaderResourceView** Texture)
{
	ScratchImage scratchimage;

	HRESULT hr;

	if (_wcsicmp(ext, L".dds") == 0)
		hr = LoadFromDDSFile(filename, DDS_FLAGS_NONE, 0, scratchimage);
	else if (_wcsicmp(ext, L".tga") == 0)
		hr = LoadFromTGAFile(filename, 0, scratchimage);
	else if (_wcsicmp(ext, L".hdr") == 0)
		hr = LoadFromHDRFile(filename, 0, scratchimage);
	/*else
		hr = LoadFromWICFile(filename, WIC_FLAGS_NONE, 0, scratchimage);*/

	if (SUCCEEDED(hr))
		hr = CreateShaderResourceView(device, scratchimage.GetImages(), scratchimage.GetImageCount(), scratchimage.GetMetadata(), Texture);

	return hr;
}

void TextureBox::BuildShapesGeometryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

	m_BoxVertexOffset = 0;
	m_BoxIndexCount = box.Indices.size();
	m_BoxIndexOffset = 0;

	UINT totalVertexCount = box.Vertices.size();

	UINT totalIndexCount = m_BoxIndexCount;

	vector<Vertex::Basic32> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_spDevice.Get()->CreateBuffer(&vbd, &vinitData, &m_BoxVB));

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_spDevice.Get()->CreateBuffer(&ibd, &iinitData, &m_BoxIB));
}
