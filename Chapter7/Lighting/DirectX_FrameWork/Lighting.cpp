#include "stdafx.h"
#include "Lighting.h"

Lighting::Lighting(HINSTANCE hInstance) :
	FrameWork(hInstance),
	m_Theta(1.5f * MathHelper::Pi),
	m_Phi(0.1f * MathHelper::Pi),
	m_Radius(200.0f),
	m_GridIndexCount(0),
	m_EyePosW(0.0f, 0.0f, 0.0f)

{
	m_MainWndCaption = L"Lighting Demo";

	m_LastMousePos.x = 0;
	m_LastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_GridWorld, I);
	XMStoreFloat4x4(&m_WaveWorld, I);
	XMStoreFloat4x4(&m_View, I);
	XMStoreFloat4x4(&m_Proj, I);

	XMMATRIX wavesOffset = XMMatrixTranslation(0.0f, -3.0f, 0.0f);
	XMStoreFloat4x4(&m_WaveWorld, wavesOffset);

	m_DirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	m_PointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_PointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_PointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_PointLight.Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_PointLight.Range = 25.0f;

	m_SpotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_SpotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	m_SpotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_SpotLight.Att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_SpotLight.Spot = 96.0f;
	m_SpotLight.Range = 10000.0f;

	m_LandMat.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_LandMat.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_LandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_WavesMat.Ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	m_WavesMat.Diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	m_WavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
}

bool Lighting::Init()
{
	if (!FrameWork::Init())
		return false;

	m_Wave.Init(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

	BuildGeometryBuffers();
	BuildWavesGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	m_spDevice->CreateRasterizerState(&wireframeDesc, &m_WireframeRS);

	return true;
}

void Lighting::Update(float dt)
{
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	m_EyePosW = XMFLOAT3(x, y, z);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_View, V);

	static float t_base = 0.0f;
	if ((m_Timer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % (m_Wave.RowCount() - 10);
		DWORD j = 5 + rand() % (m_Wave.ColumnCount() - 10);

		float r = MathHelper::RandF(1.0f, 2.0f);

		m_Wave.Disturb(i, j, r);
	}

	m_Wave.Update(dt);

	//
	// Update the wave vertex buffer with the new solution.
	//

	D3D11_MAPPED_SUBRESOURCE mappedData;
	m_spDeviceCon->Map(m_WavesVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

	Vertex* v = reinterpret_cast<Vertex*>(mappedData.pData);
	for (UINT i = 0; i < m_Wave.VertexCount(); ++i)
	{
		v[i].Pos = m_Wave[i];
		v[i].Normal = m_Wave.Normal(i);
	}

	m_spDeviceCon->Unmap(m_WavesVB.Get(), 0);

	//
	// Animate the lights.
	//
	m_PointLight.Position.x = 70.0f * cosf(0.2f * m_Timer.TotalTime());
	m_PointLight.Position.z = 70.0f * sinf(0.2f * m_Timer.TotalTime());
	m_PointLight.Position.y = MathHelper::Max(GetHeight(m_PointLight.Position.x,
		m_PointLight.Position.z), -3.0f) + 10.0f;


	// The spotlight takes on the camera position and is aimed in the
	// same direction the camera is looking.  In this way, it looks
	// like we are holding a flashlight.
	m_SpotLight.Position = m_EyePosW;
	XMStoreFloat3(&m_SpotLight.Direction, XMVector3Normalize(target - pos));
}

void Lighting::RenderFrame()
{
	float color[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
	m_spDeviceCon->ClearRenderTargetView(m_spTargetView.Get(), color);
	m_spDeviceCon->ClearDepthStencilView(m_spDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	m_spDeviceCon->IASetInputLayout(m_InputLayout.Get());
	m_spDeviceCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// Set constants
	XMMATRIX view = XMLoadFloat4x4(&m_View);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);
	XMMATRIX ViewProj = view * proj;

	m_fxDirLight->SetRawValue(&m_DirLight, 0, sizeof(m_DirLight));
	m_fxPointLight->SetRawValue(&m_PointLight, 0, sizeof(m_PointLight));
	m_fxSpotLight->SetRawValue(&m_SpotLight, 0, sizeof(m_SpotLight));
	m_fxEyePosW->SetRawValue(&m_EyePosW, 0, sizeof(m_EyePosW));

	D3DX11_TECHNIQUE_DESC techDesc;
	m_Tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		//Land
		m_spDeviceCon->IASetVertexBuffers(0, 1, m_LandVB.GetAddressOf(), &stride, &offset);
		m_spDeviceCon->IASetIndexBuffer(m_LandIB.Get(), DXGI_FORMAT_R32_UINT, 0);
		
		XMMATRIX world = XMLoadFloat4x4(&m_GridWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldviewproj = world * ViewProj;

		m_fxWorld->SetMatrix(reinterpret_cast<float*>(&world));
		m_fxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
		m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldviewproj));
		m_fxMaterial->SetRawValue(&m_LandMat, 0, sizeof(m_LandMat));

		m_Tech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(m_GridIndexCount, 0, 0);
		//

		//Wave
		m_spDeviceCon->IASetVertexBuffers(0, 1, m_WavesVB.GetAddressOf(), &stride, &offset);
		m_spDeviceCon->IASetIndexBuffer(m_WavesIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		world = XMLoadFloat4x4(&m_WaveWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldviewproj = world * view * proj;

		m_fxWorld->SetMatrix(reinterpret_cast<float*>(&world));
		m_fxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
		m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldviewproj));
		m_fxMaterial->SetRawValue(&m_WavesMat, 0, sizeof(m_WavesMat));

		m_Tech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(3 * m_Wave.TriangleCount(), 0, 0);
		//

		// Restore default.
		m_spDeviceCon->RSSetState(0);
	}

	m_spSwapchain->Present(0, 0);
}

void Lighting::OnResize()
{
	FrameWork::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, P);
}

void Lighting::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	SetCapture(m_hWnd);
}

void Lighting::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Lighting::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.03f * static_cast<float>(x - m_LastMousePos.x);
		float dy = 0.03f * static_cast<float>(y - m_LastMousePos.y);

		m_Radius += dx - dy;

		m_Radius = MathHelper::Clamp(m_Radius, 50.0f, 500.0f);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

float Lighting::GetHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));;
}

XMFLOAT3 Lighting::GetHillNormal(float x, float z) const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void Lighting::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData grid;

	GeometryGenerator geoGen;
	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);
	m_GridIndexCount = grid.Indices.size();

	vector<Vertex> vertices(grid.Vertices.size());

	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHeight(p.x, p.z);

		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.y);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	m_spDevice->CreateBuffer(&vbd, &vinitData, &m_LandVB);

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_GridIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	m_spDevice->CreateBuffer(&ibd, &iinitData, &m_LandIB);
}

void Lighting::BuildWavesGeometryBuffers()
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC; //움직이는거
	vbd.ByteWidth = sizeof(Vertex) * m_Wave.VertexCount(); 
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //움직이는거
	vbd.MiscFlags = 0;
	m_spDevice->CreateBuffer(&vbd, 0, &m_WavesVB);

	vector<UINT> indices(3 * m_Wave.TriangleCount());

	UINT m = m_Wave.RowCount();
	UINT n = m_Wave.ColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

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
	m_spDevice->CreateBuffer(&ibd, &iinitData, &m_WavesIB);
}

void Lighting::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3D10Blob> compiledShader;
	ComPtr<ID3D10Blob> compilationMsgs;
	//효과파일을 compiledShader에 담아준다.
	HRESULT hr = D3DCompileFromFile(L"FX/Lighting.fx", 0, 0, 0, "fx_5_0", shaderFlags, 0, compiledShader.GetAddressOf(), compilationMsgs.GetAddressOf());

	if (FAILED(hr))
	{
		OutputShaderErrorMsg(compilationMsgs.Get(), m_hWnd, L"FX/Lighting.fx");
	}

	D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
		0, m_spDevice.Get(), m_FX.GetAddressOf());

	m_Tech = m_FX->GetTechniqueByName("LightTech");
	m_fxWorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	m_fxWorld = m_FX->GetVariableByName("gWorld")->AsMatrix();
	m_fxWorldInvTranspose = m_FX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	m_fxEyePosW = m_FX->GetVariableByName("gEyePosW")->AsVector();
	m_fxDirLight = m_FX->GetVariableByName("gDirLight");
	m_fxPointLight = m_FX->GetVariableByName("gPointLight");
	m_fxSpotLight = m_FX->GetVariableByName("gSpotLight");
	m_fxMaterial = m_FX->GetVariableByName("gMaterial");
}

void Lighting::BuildVertexLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	D3DX11_PASS_DESC passDesc;
	m_Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	m_spDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &m_InputLayout);
}

void Lighting::OutputShaderErrorMsg(ID3D10Blob* errMsg, HWND hWnd, LPCWSTR shaderfileName) const
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;

	compileErrors = (char*)(errMsg->GetBufferPointer());

	bufferSize = errMsg->GetBufferSize();

	fout.open("shader-error.txt");

	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}
	fout.close();

	MessageBoxW(hWnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderfileName, MB_OK);
}
