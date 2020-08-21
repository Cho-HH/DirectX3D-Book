#include "stdafx.h"
#include "LitSkull.h"
#include "Effects.h"
#include "Vertex.h"

LitSkull::LitSkull(HINSTANCE hInstance) :
	FrameWork(hInstance),
	m_Theta(1.5f * MathHelper::Pi),
	m_Phi(0.1f * MathHelper::Pi),
	m_Radius(15.0f)

{
	m_MainWndCaption = L"LitSkull";

	m_LastMousePos.x = 0;
	m_LastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_GridWorld, I);
	XMStoreFloat4x4(&m_View, I);
	XMStoreFloat4x4(&m_Proj, I);

	XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f); //크기
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f); //위치
	XMStoreFloat4x4(&m_BoxWorld, XMMatrixMultiply(boxScale, boxOffset));

	XMMATRIX centerSkullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX centerSkullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&m_SkullWorld, XMMatrixMultiply(centerSkullScale, centerSkullOffset));

	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&m_CylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&m_CylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

		XMStoreFloat4x4(&m_SphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&m_SphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
	}

	m_DirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	m_DirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	m_DirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	m_DirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	m_DirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	m_GridMat.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_GridMat.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_GridMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_CylinderMat.Ambient = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
	m_CylinderMat.Diffuse = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
	m_CylinderMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

	m_SphereMat.Ambient = XMFLOAT4(0.1f, 0.2f, 0.3f, 1.0f);
	m_SphereMat.Diffuse = XMFLOAT4(0.2f, 0.4f, 0.6f, 1.0f);
	m_SphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);

	m_BoxMat.Ambient = XMFLOAT4(0.651f, 0.5f, 0.392f, 1.0f);
	m_BoxMat.Diffuse = XMFLOAT4(0.651f, 0.5f, 0.392f, 1.0f);
	m_BoxMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_SkullMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_SkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_SkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
}

bool LitSkull::Init()
{
	if (!FrameWork::Init())
		return false;

	Effects::InitAll(m_spDevice.Get());
	InputLayouts::InitAll(m_spDevice.Get());

	BuildShapesGeometryBuffers();
	BuildSkullGeometryBuffers();

	return true;
}

void LitSkull::Update(float dt)
{
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_View, V);

	if (GetAsyncKeyState('1') & 0x8000)
		m_LightCount = 1;
	if (GetAsyncKeyState('2') & 0x8000)
		m_LightCount = 2;
	if (GetAsyncKeyState('3') & 0x8000)
		m_LightCount = 3;
}

void LitSkull::RenderFrame()
{
	float color[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
	m_spDeviceCon->ClearRenderTargetView(m_spTargetView.Get(), color);
	m_spDeviceCon->ClearDepthStencilView(m_spDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	m_spDeviceCon->IASetInputLayout(InputLayouts::PosNormal.Get());
	m_spDeviceCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::PosNormal);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&m_View);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);
	XMMATRIX ViewProj = view * proj;

	Effects::BasicFX->SetDirLights(m_DirLights);
	Effects::BasicFX->SetEyePosW(m_EyePosW);

	ComPtr<ID3DX11EffectTechnique> activeTech = Effects::BasicFX->Light1Tech;
	switch (m_LightCount)
	{
	case 1:
		activeTech = Effects::BasicFX->Light1Tech;
		break;
	case 2:
		activeTech = Effects::BasicFX->Light2Tech;
		break;
	case 3:
		activeTech = Effects::BasicFX->Light3Tech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_spDeviceCon->IASetVertexBuffers(0, 1, m_ShapesVB.GetAddressOf(), &stride, &offset);
		m_spDeviceCon->IASetIndexBuffer(m_ShapesIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		//grid
		XMMATRIX world = XMLoadFloat4x4(&m_GridWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldviewproj = world * ViewProj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldviewproj);
		Effects::BasicFX->SetMaterial(m_GridMat);

		activeTech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(m_GridIndexCount, m_GridIndexOffset, m_GridVertexOffset);

		//Box
		world = XMLoadFloat4x4(&m_BoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldviewproj = world * ViewProj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldviewproj);
		Effects::BasicFX->SetMaterial(m_BoxMat);

		activeTech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(m_BoxIndexCount, m_BoxIndexOffset, m_BoxVertexOffset);

		//cylinders
		for (int i = 0; i < 10; i++)
		{
			world = XMLoadFloat4x4(&m_CylWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldviewproj = world * ViewProj;

			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldviewproj);
			Effects::BasicFX->SetMaterial(m_CylinderMat);

			activeTech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
			m_spDeviceCon->DrawIndexed(m_CylinderIndexCount, m_CylinderIndexOffset, m_CylinderVertexOffset);
		}
		
		//spheres
		for (int i = 0; i < 10; i++)
		{
			world = XMLoadFloat4x4(&m_SphereWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldviewproj = world * ViewProj;

			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldviewproj);
			Effects::BasicFX->SetMaterial(m_SphereMat);

			activeTech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
			m_spDeviceCon->DrawIndexed(m_SphereIndexCount, m_SphereIndexOffset, m_SphereVertexOffset);
		}

		//skull
		m_spDeviceCon->IASetVertexBuffers(0, 1, m_SkullVB.GetAddressOf(), &stride, &offset);
		m_spDeviceCon->IASetIndexBuffer(m_SkullIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		world = XMLoadFloat4x4(&m_SkullWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldviewproj = world * ViewProj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldviewproj);
		Effects::BasicFX->SetMaterial(m_SkullMat);

		activeTech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(m_SkullIndexCount, 0, 0);
	}

	m_spSwapchain->Present(0, 0);
}

void LitSkull::OnResize()
{
	FrameWork::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, P);
}

void LitSkull::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	SetCapture(m_hWnd);
}

void LitSkull::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void LitSkull::OnMouseMove(WPARAM btnState, int x, int y)
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

void LitSkull::BuildShapesGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	m_BoxVertexOffset = 0;
	m_GridVertexOffset = box.Vertices.size();
	m_SphereVertexOffset = m_GridVertexOffset + grid.Vertices.size();
	m_CylinderVertexOffset = m_SphereVertexOffset + sphere.Vertices.size();

	m_BoxIndexCount = box.Indices.size();
	m_GridIndexCount = grid.Indices.size();
	m_SphereIndexCount = sphere.Indices.size();
	m_CylinderIndexCount = cylinder.Indices.size();

	m_BoxIndexOffset = 0;
	m_GridIndexOffset = m_BoxIndexCount;
	m_SphereIndexOffset = m_GridIndexOffset + m_GridIndexCount;
	m_CylinderIndexOffset = m_SphereIndexOffset + m_SphereIndexCount;

	UINT TotalVertexCnt = box.Vertices.size() + grid.Vertices.size() + sphere.Vertices.size() + cylinder.Vertices.size();
	UINT TotalIndexCnt = m_BoxIndexCount + m_GridIndexCount + m_SphereIndexCount + m_CylinderIndexCount;


	vector<Vertex::PosNormal> vertices(TotalVertexCnt);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNormal) * TotalVertexCnt;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	m_spDevice->CreateBuffer(&vbd, &vinitData, &m_ShapesVB);

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * TotalIndexCnt;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	m_spDevice->CreateBuffer(&ibd, &iinitData, &m_ShapesIB);
}

void LitSkull::BuildSkullGeometryBuffers()
{
	ifstream file("Models/skull.txt");

	UINT vertexcnt = 0;
	UINT tricnt = 0;
	char ignore[256] = { 0, };

	file >> ignore >> vertexcnt;
	file >> ignore >> tricnt;
	file >> ignore >> ignore >> ignore >> ignore;

	vector<Vertex::PosNormal> vertices(vertexcnt);

	for (int i = 0; i < vertexcnt; i++)
	{
		file >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z
			>> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	file >> ignore >> ignore >> ignore;

	m_SkullIndexCount = 3 * tricnt;
	vector<UINT> indices(m_SkullIndexCount);

	for (int i = 0; i < tricnt; i++)
	{
		file >> indices[i * 3] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	file.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNormal) * vertexcnt;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	m_spDevice->CreateBuffer(&vbd, &vinitData, &m_SkullVB);

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_SkullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	m_spDevice->CreateBuffer(&ibd, &iinitData, &m_SkullIB);
}

