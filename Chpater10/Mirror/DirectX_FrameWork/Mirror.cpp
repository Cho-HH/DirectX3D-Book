#include "stdafx.h"
#include "Mirror.h"
#include "Vertex.h"
#include "Effects.h"
#include "RenderState.h"

Mirror::Mirror(HINSTANCE hInstance) :
	FrameWork(hInstance),
	m_Theta(1.3f * MathHelper::Pi),
	m_Phi(0.4f * MathHelper::Pi),
	m_Radius(12.0f), m_LightCount(1), m_EyePosW(0.0f, 0.0f, 0.0f), m_SkullTranslation(0.0f, 1.0f, -5.0f)
{
	m_MainWndCaption = L"Mirror";

	m_LastMousePos.x = 0;
	m_LastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_RoomWorld, I);
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

	m_RoomMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_RoomMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_RoomMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	m_SkullMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_SkullMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_SkullMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	// Reflected material is transparent so it blends into mirror.
	m_MirrorMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_MirrorMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	m_MirrorMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	m_ShadowMat.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_ShadowMat.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	m_ShadowMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}

bool Mirror::Init()
{
	if (!FrameWork::Init())
		return false;

	Effects::InitAll(m_spDevice.Get());
	InputLayouts::InitAll(m_spDevice.Get());
	RenderState::InitAll(m_spDevice.Get());

	wchar_t ext[_MAX_EXT];
	wchar_t name[_MAX_EXT];
	wchar_t drive[_MAX_EXT];
	wchar_t dir[_MAX_EXT];

	wchar_t Floor[_MAX_EXT] = L"Textures/checkboard.dds";

	_wsplitpath_s(Floor, drive, dir, name, ext);
	CreateShaderResourseViewFromFile(m_spDevice.Get(), Floor, ext, m_FloorMapSRV.GetAddressOf());

	wchar_t brick[_MAX_EXT] = L"Textures/brick01.dds";

	_wsplitpath_s(brick, drive, dir, name, ext);
	CreateShaderResourseViewFromFile(m_spDevice.Get(), brick, ext, m_WallMapSRV.GetAddressOf());

	wchar_t ice[_MAX_EXT] = L"Textures/ice.dds";

	_wsplitpath_s(ice, drive, dir, name, ext);
	CreateShaderResourseViewFromFile(m_spDevice.Get(), ice, ext, m_MirrorMapSRV.GetAddressOf());

	BuildRoomGeoBuffer();
	BuildSkullGeoBuffer();

	return true;
}

void Mirror::Update(float dt)
{
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_View, V);

	//
	// Switch the render mode based in key input.
	//
	if (GetAsyncKeyState('1') & 0x8000)
		m_RenderOptions = RenderOptions::Lighting;

	if (GetAsyncKeyState('2') & 0x8000)
		m_RenderOptions = RenderOptions::Textures;

	if (GetAsyncKeyState('3') & 0x8000)
		m_RenderOptions = RenderOptions::TexturesAndFog;

	if (GetAsyncKeyState('A') & 0x8000)
		m_SkullTranslation.x -= 1.0f*dt;

	if (GetAsyncKeyState('D') & 0x8000)
		m_SkullTranslation.x += 1.0f*dt;

	if (GetAsyncKeyState('W') & 0x8000)
		m_SkullTranslation.y += 1.0f*dt;

	if (GetAsyncKeyState('S') & 0x8000)
		m_SkullTranslation.y -= 1.0f*dt;

	m_SkullTranslation.y = MathHelper::Max(m_SkullTranslation.y, 0.0f);

	XMMATRIX skullRotate = XMMatrixRotationY(0.5f*MathHelper::Pi);
	XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	XMMATRIX skullOffset = XMMatrixTranslation(m_SkullTranslation.x, m_SkullTranslation.y, m_SkullTranslation.z);
	XMStoreFloat4x4(&m_SkullWorld, skullRotate*skullScale*skullOffset);
}

void Mirror::RenderFrame()
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

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	XMMATRIX view = XMLoadFloat4x4(&m_View);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);
	XMMATRIX ViewProj = view * proj;

	Effects::BasicFX->SetDirLights(m_DirLights);
	Effects::BasicFX->SetEyePosW(m_EyePosW);
	Effects::BasicFX->SetFogColor(Colors::Silver);
	Effects::BasicFX->SetFogStart(2.0f);
	Effects::BasicFX->SetFogRange(40.0f);

	ComPtr<ID3DX11EffectTechnique> activeTech;
	ComPtr<ID3DX11EffectTechnique> activeSkullTech;
	D3DX11_TECHNIQUE_DESC techDesc;

	switch (m_RenderOptions)
	{
	case RenderOptions::Lighting:
		activeTech = Effects::BasicFX->Light3Tech;
		activeSkullTech = Effects::BasicFX->Light3Tech;
		break;
	case RenderOptions::Textures:
		activeTech = Effects::BasicFX->Light3TexAlphaClipTech;
		activeSkullTech = Effects::BasicFX->Light3TexTech;
		break;
	case RenderOptions::TexturesAndFog:
		activeTech = Effects::BasicFX->Light3TexAlphaClipFogTech;
		activeSkullTech = Effects::BasicFX->Light3TexFogTech;
		break;
	}

#pragma region 방그리기
	activeTech ->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_spDeviceCon->IASetVertexBuffers(0, 1, m_RoomVB.GetAddressOf(), &stride, &offset);

		XMMATRIX world = XMLoadFloat4x4(&m_RoomWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldviewproj = world * ViewProj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldviewproj);
		Effects::BasicFX->SetMaterial(m_RoomMat);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());

		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		// Floor
		Effects::BasicFX->SetDiffuseMap(m_FloorMapSRV.Get());
		pass->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->Draw(6, 0);

		// Wall
		Effects::BasicFX->SetDiffuseMap(m_WallMapSRV.Get());
		pass->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->Draw(18, 6);
	}
#pragma endregion 

#pragma region 해골그리기
	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

		m_spDeviceCon->IASetVertexBuffers(0, 1, m_SkullVB.GetAddressOf(), &stride, &offset);
		m_spDeviceCon->IASetIndexBuffer(m_SkullIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&m_SkullWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldviewproj = world * ViewProj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldviewproj);
		Effects::BasicFX->SetMaterial(m_SkullMat);

		pass->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(m_SkullIndexCnt, 0, 0);
	}
#pragma endregion
	
#pragma region 거울부분에 스텐실버퍼적용하여 "스텐실버퍼에만" 그리기
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		m_spDeviceCon->IASetVertexBuffers(0, 1, m_RoomVB.GetAddressOf(), &stride, &offset);

		XMMATRIX world = XMLoadFloat4x4(&m_RoomWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());

		// 후면버퍼에 기록되지 않게 한다
		m_spDeviceCon->OMSetBlendState(RenderState::NoRenderTargetWritesBS.Get(), blendFactor, 0xffffffff);

		// Render visible mirror pixels to stencil buffer.
		// Do not write mirror depth to depth buffer at this point, otherwise it will occlude the reflection.
		m_spDeviceCon->OMSetDepthStencilState(RenderState::MarkMirrorDSS.Get(), 1);

		pass->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->Draw(6, 24);

		// Restore states.
		m_spDeviceCon->OMSetDepthStencilState(0, 0);
		m_spDeviceCon->OMSetBlendState(0, blendFactor, 0xffffffff);
	}
#pragma endregion
	
#pragma region 거울속에 해골그리기
	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

		m_spDeviceCon->IASetVertexBuffers(0, 1, m_SkullVB.GetAddressOf(), &stride, &offset);
		m_spDeviceCon->IASetIndexBuffer(m_SkullIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
		XMMATRIX R = XMMatrixReflect(mirrorPlane);
		XMMATRIX world = XMLoadFloat4x4(&m_SkullWorld) * R;
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(m_SkullMat);

		// Cache the old light directions, and reflect the light directions.
		XMFLOAT3 oldLightDirections[3];
		for (int i = 0; i < 3; ++i)
		{
			oldLightDirections[i] = m_DirLights[i].Direction;

			XMVECTOR lightDir = XMLoadFloat3(&m_DirLights[i].Direction);
			XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
			XMStoreFloat3(&m_DirLights[i].Direction, reflectedLightDir);
		}

		Effects::BasicFX->SetDirLights(m_DirLights);

		// Cull clockwise triangles for reflection.
		m_spDeviceCon->RSSetState(RenderState::CullClockwiseRS.Get());

		// Only draw reflection into visible mirror pixels as marked by the stencil buffer. 
		m_spDeviceCon->OMSetDepthStencilState(RenderState::DrawReflectionDSS.Get(), 1);
		pass->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(m_SkullIndexCnt, 0, 0);

		// Restore default states.
		m_spDeviceCon->RSSetState(0);
		m_spDeviceCon->OMSetDepthStencilState(0, 0);

		// Restore light directions.
		for (int i = 0; i < 3; ++i)
		{
			m_DirLights[i].Direction = oldLightDirections[i];
		}

		Effects::BasicFX->SetDirLights(m_DirLights);
	}
#pragma endregion

#pragma region 후면버퍼에 거울그리기
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		m_spDeviceCon->IASetVertexBuffers(0, 1, m_RoomVB.GetAddressOf(), &stride, &offset);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&m_RoomWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(m_MirrorMat);
		Effects::BasicFX->SetDiffuseMap(m_MirrorMapSRV.Get());

		// Mirror
		m_spDeviceCon->OMSetBlendState(RenderState::TransparentBS.Get(), blendFactor, 0xffffffff);
		pass->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->Draw(6, 24);
	}
#pragma endregion
	
#pragma region 그림자 그리기
	ac
tiveSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

		m_spDeviceCon->IASetVertexBuffers(0, 1, m_SkullVB.GetAddressOf(), &stride, &offset);
		m_spDeviceCon->IASetIndexBuffer(m_SkullIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // xz plane
		XMVECTOR toMainLight = -XMLoadFloat3(&m_DirLights[0].Direction);
		XMMATRIX S = XMMatrixShadow(shadowPlane, toMainLight);
		XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&m_SkullWorld)*S*shadowOffsetY;
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(m_ShadowMat);

		m_spDeviceCon->OMSetDepthStencilState(RenderState::NoDoubleBlendDSS.Get(), 0);
		pass->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(m_SkullIndexCnt, 0, 0);

		// Restore default states.
		m_spDeviceCon->OMSetBlendState(0, blendFactor, 0xffffffff);
		m_spDeviceCon->OMSetDepthStencilState(0, 0);
	}
#pragma endregion

	m_spSwapchain->Present(0, 0);
}

void Mirror::OnResize()
{
	FrameWork::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, P);
}

void Mirror::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	SetCapture(m_hWnd);
}

void Mirror::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Mirror::OnMouseMove(WPARAM btnState, int x, int y)
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

HRESULT Mirror::CreateShaderResourseViewFromFile(ID3D11Device* device, const wchar_t* filename, const wchar_t* ext, ID3D11ShaderResourceView** Texture)
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

void Mirror::BuildRoomGeoBuffer()
{
	// Create and specify geometry.  For this sample we draw a floor
	// and a wall with a mirror on it.  We put the floor, wall, and
	// mirror geometry in one vertex buffer.
	//
	//   |--------------|
	//   |              |
	//   |----|----|----|
	//   |Wall|Mirr|Wall|
	//   |    | or |    |
	//   /--------------/
	//  /   Floor      /
	// /--------------/


	Vertex::Basic32 v[30];

	// Floor: Observe we tile texture coordinates.
	v[0] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[1] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);

	v[3] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[4] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
	v[5] = Vertex::Basic32(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f);

	// Wall: Observe we tile texture coordinates, and that we
	// leave a gap in the middle for the mirror.
	v[6] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[7] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[8] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);

	v[9] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[10] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);
	v[11] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f);

	v[12] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[13] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[14] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);

	v[15] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[16] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
	v[17] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f);

	v[18] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[19] = Vertex::Basic32(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[20] = Vertex::Basic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);

	v[21] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[22] = Vertex::Basic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);
	v[23] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f);

	// Mirror
	v[24] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[25] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[26] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[27] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[28] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[29] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * 30;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;
	m_spDevice->CreateBuffer(&vbd, &vinitData, m_RoomVB.GetAddressOf());
}

void Mirror::BuildSkullGeoBuffer()
{
	ifstream fin("Models/skull.txt");

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	vector<Vertex::Basic32> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	m_SkullIndexCnt = 3 * tcount;
	vector<UINT> indices(m_SkullIndexCnt);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	m_spDevice->CreateBuffer(&vbd, &vinitData, m_SkullVB.GetAddressOf());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_SkullIndexCnt;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	m_spDevice->CreateBuffer(&ibd, &iinitData, m_SkullIB.GetAddressOf());
}