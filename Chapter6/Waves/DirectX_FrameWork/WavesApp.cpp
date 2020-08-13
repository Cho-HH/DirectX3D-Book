#include "stdafx.h"
#include "WavesApp.h"

WavesApp::WavesApp(HINSTANCE hInstance) :
	FrameWork(hInstance),
	m_Theta(1.5f * MathHelper::Pi),
	m_Phi(0.1f * MathHelper::Pi),
	m_Radius(200.0f),
	m_GridIndexCount(0)

{
	m_MainWndCaption = L"Waves Demo";

	m_LastMousePos.x = 0;
	m_LastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_GridWorld, I);
	XMStoreFloat4x4(&m_WaveWorld, I);
	XMStoreFloat4x4(&m_View, I);
	XMStoreFloat4x4(&m_Proj, I);
}

bool WavesApp::Init()
{
	if (!FrameWork::Init())
		return false;

	m_Wave.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

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

void WavesApp::Update(float dt)
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

		DWORD i = 5 + rand() % 190;
		DWORD j = 5 + rand() % 190;

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
		v[i].Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	m_spDeviceCon->Unmap(m_WavesVB.Get(), 0);
}

void WavesApp::RenderFrame()
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

	m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&ViewProj));

	D3DX11_TECHNIQUE_DESC techDesc;
	m_Tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_spDeviceCon->IASetVertexBuffers(0, 1, m_LandVB.GetAddressOf(), &stride, &offset);
		m_spDeviceCon->IASetIndexBuffer(m_LandIB.Get(), DXGI_FORMAT_R32_UINT, 0);
		
		XMMATRIX world = XMLoadFloat4x4(&m_GridWorld);
		XMMATRIX worldviewproj = world * ViewProj;
		m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldviewproj));
		m_Tech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(m_GridIndexCount, 0, 0);

		m_spDeviceCon->RSSetState(m_WireframeRS.Get());

		m_spDeviceCon->IASetVertexBuffers(0, 1, m_WavesVB.GetAddressOf(), &stride, &offset);
		m_spDeviceCon->IASetIndexBuffer(m_WavesIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		world = XMLoadFloat4x4(&m_WaveWorld);
		worldviewproj = world * view * proj;
		m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldviewproj));
		m_Tech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(3 * m_Wave.TriangleCount(), 0, 0);

		// Restore default.
		m_spDeviceCon->RSSetState(0);
	}

	m_spSwapchain->Present(0, 0);
}

void WavesApp::OnResize()
{
	FrameWork::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, P);
}

void WavesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	SetCapture(m_hWnd);
}

void WavesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void WavesApp::OnMouseMove(WPARAM btnState, int x, int y)
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

float WavesApp::GetHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));;
}

void WavesApp::BuildGeometryBuffers()
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

		// Color the vertex based on its height.
		if (p.y < -10.0f)
		{
			// Sandy beach color.
			vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (p.y < 5.0f)
		{
			// Light yellow-green.
			vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (p.y < 12.0f)
		{
			// Dark yellow-green.
			vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (p.y < 20.0f)
		{
			// Dark brown.
			vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			// White snow.
			vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
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

void WavesApp::BuildWavesGeometryBuffers()
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

void WavesApp::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3D10Blob> compiledShader;
	ComPtr<ID3D10Blob> compilationMsgs;
	//효과파일을 compiledShader에 담아준다.
	HRESULT hr = D3DCompileFromFile(L"FX/color.fx", 0, 0, 0, "fx_5_0", shaderFlags, 0, compiledShader.GetAddressOf(), compilationMsgs.GetAddressOf());

	// compilationMsgs can store errors or warnings.
	/*if (compilationMsgs != 0)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
	}*/

	if (FAILED(hr))
	{
		OutputShaderErrorMsg(compilationMsgs.Get(), m_hWnd, L"FX/color.fx");
	}

	D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
		0, m_spDevice.Get(), m_FX.GetAddressOf());

	m_Tech = m_FX->GetTechniqueByName("ColorTech");
	m_fxWorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void WavesApp::BuildVertexLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	D3DX11_PASS_DESC passDesc;
	m_Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	m_spDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &m_InputLayout);
}

void WavesApp::OutputShaderErrorMsg(ID3D10Blob* errMsg, HWND hWnd, LPCWSTR shaderfileName) const
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
