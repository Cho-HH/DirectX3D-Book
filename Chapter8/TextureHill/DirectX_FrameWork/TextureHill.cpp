#include "stdafx.h"
#include "TextureHill.h"
#include "Vertex.h"
#include "Effects.h"

TextureHill::TextureHill(HINSTANCE hInstance) :
	FrameWork(hInstance),
	m_Theta(1.3f * MathHelper::Pi),
	m_Phi(0.3f * MathHelper::Pi),
	m_Radius(150.0f), m_LightCount(1), m_LandIndexCount(NULL), m_WaterTexOffset(0.0f,0.0f), m_EyePosW(0.0f,0.0f,0.0f)
{
	m_MainWndCaption = L"TextureHill";

	m_LastMousePos.x = 0;
	m_LastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_LandWorld, I);
	XMStoreFloat4x4(&m_WavesWorld, I);
	XMStoreFloat4x4(&m_View, I);
	XMStoreFloat4x4(&m_Proj, I);

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	XMStoreFloat4x4(&m_GrassTexTransform, grassTexScale);

	m_DirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_DirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_DirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	m_DirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	m_DirLights[1].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[1].Diffuse = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	m_DirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	m_DirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

	m_LandMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_LandMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_LandMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

	m_WavesMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_WavesMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_WavesMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 32.0f);
}

bool TextureHill::Init()
{
	if (!FrameWork::Init())
		return false;

	m_Waves.Init(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

	Effects::InitAll(m_spDevice.Get());
	InputLayouts::InitAll(m_spDevice.Get());

	wchar_t ext[_MAX_EXT];
	wchar_t name[_MAX_EXT];
	wchar_t drive[_MAX_EXT];
	wchar_t dir[_MAX_EXT];
	wchar_t filename[_MAX_EXT];

	wchar_t Grassfilename[_MAX_EXT] = L"Textures/grass.dds";

	_wsplitpath_s(Grassfilename, drive, dir, name, ext);
	CreateShaderResourseViewFromFile(m_spDevice.Get(), Grassfilename, ext, m_GrassMapSRV.GetAddressOf());

	wchar_t Wavesfilename[_MAX_EXT] = L"Textures/water1.dds";

	_wsplitpath_s(Wavesfilename, drive, dir, name, ext);
	CreateShaderResourseViewFromFile(m_spDevice.Get(), Wavesfilename, ext, m_WavesMapSRV.GetAddressOf());

	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();

	return true;
}

void TextureHill::Update(float dt)
{
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_View, V);

	static float t_base = 0.0f;
	if ((m_Timer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % (m_Waves.RowCount() - 10);
		DWORD j = 5 + rand() % (m_Waves.ColumnCount() - 10);

		float r = MathHelper::RandF(1.0f, 2.0f);

		m_Waves.Disturb(i, j, r);
	}

	m_Waves.Update(dt);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	m_spDeviceCon->Map(m_WaveVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

	Vertex::Basic32* v = reinterpret_cast<Vertex::Basic32*>(mappedData.pData);
	for (UINT i = 0; i < m_Waves.VertexCount(); ++i)
	{
		v[i].Pos = m_Waves[i];
		v[i].Normal = m_Waves.Normal(i);

		// Derive tex-coords in [0,1] from position.
		v[i].Tex.x = 0.5f + m_Waves[i].x / m_Waves.Width();
		v[i].Tex.y = 0.5f - m_Waves[i].z / m_Waves.Depth();
	}

	m_spDeviceCon->Unmap(m_WaveVB.Get(), 0);

	// Tile water texture.
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// Translate texture over time.
	m_WaterTexOffset.y += 0.05f*dt;
	m_WaterTexOffset.x += 0.1f*dt;
	XMMATRIX wavesOffset = XMMatrixTranslation(m_WaterTexOffset.x, m_WaterTexOffset.y, 0.0f);

	// Combine scale and translation.
	XMStoreFloat4x4(&m_WavesTexTransform, wavesScale*wavesOffset);
}

void TextureHill::RenderFrame()
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
		//Land
		m_spDeviceCon->IASetVertexBuffers(0, 1, m_LandVB.GetAddressOf(), &stride, &offset);
		m_spDeviceCon->IASetIndexBuffer(m_LandIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&m_LandWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldviewproj = world * ViewProj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldviewproj);
		Effects::BasicFX->SetMaterial(m_LandMat);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&m_GrassTexTransform));
		Effects::BasicFX->SetDiffuseMap(m_GrassMapSRV.Get());

		activeTech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(m_LandIndexCount, 0, 0);

		//Wave
		m_spDeviceCon->IASetVertexBuffers(0, 1, m_WaveVB.GetAddressOf(), &stride, &offset);
		m_spDeviceCon->IASetIndexBuffer(m_WaveIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		world = XMLoadFloat4x4(&m_WavesWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldviewproj = world * ViewProj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldviewproj);
		Effects::BasicFX->SetMaterial(m_WavesMat);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&m_WavesTexTransform));
		Effects::BasicFX->SetDiffuseMap(m_WavesMapSRV.Get());

		activeTech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(3 * m_Waves.TriangleCount(), 0, 0);
	}

	m_spSwapchain->Present(0, 0);
}

void TextureHill::OnResize()
{
	FrameWork::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, P);
}

void TextureHill::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	SetCapture(m_hWnd);
}

void TextureHill::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void TextureHill::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.05f * static_cast<float>(x - m_LastMousePos.x);
		float dy = 0.05f * static_cast<float>(y - m_LastMousePos.y);

		m_Radius += dx - dy;

		m_Radius = MathHelper::Clamp(m_Radius, 3.0f, 150.0f);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

HRESULT TextureHill::CreateShaderResourseViewFromFile(ID3D11Device* device, const wchar_t* filename, const wchar_t* ext, ID3D11ShaderResourceView** Texture)
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

void TextureHill::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;

	GeometryGenerator geoGen;
	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	vector<Vertex::Basic32> vertices(grid.Vertices.size());

	UINT k = 0;
	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHillHeight(p.x, p.z);

		vertices[k].Pos = p;
		vertices[k].Normal = GetHillNormal(p.x, p.y);
		vertices[k].Tex = grid.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	m_spDevice.Get()->CreateBuffer(&vbd, &vinitData, &m_LandVB);

	m_LandIndexCount = grid.Indices.size();

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_LandIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	m_spDevice.Get()->CreateBuffer(&ibd, &iinitData, &m_LandIB);
}

void TextureHill::BuildWaveGeometryBuffers()
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * m_Waves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	m_spDevice->CreateBuffer(&vbd, 0, &m_WaveVB);

	vector<UINT> indices(3 * m_Waves.TriangleCount());

	UINT m = m_Waves.RowCount();
	UINT n = m_Waves.ColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1)*n + j;

			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	m_spDevice->CreateBuffer(&ibd, &iinitData, &m_WaveIB);
}

XMFLOAT3 TextureHill::GetHillNormal(float x, float z) const
{
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

float TextureHill::GetHillHeight(float x, float z) const
{
	return 0.3f*(z*sinf(0.1f*x) + x * cosf(0.1f*z));
}