#include "stdafx.h"
#include "Shapes.h"

Shapes::Shapes(HINSTANCE hInstance) :
	FrameWork(hInstance),
	m_Theta(1.5f * MathHelper::Pi),
	m_Phi(0.1f * MathHelper::Pi),
	m_Radius(20.0f)

{
	m_MainWndCaption = L"Skull Demo";

	m_LastMousePos.x = 0;
	m_LastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_View, I);
	XMStoreFloat4x4(&m_Proj, I);

	XMMATRIX T = XMMatrixTranslation(0.0f, -2.0f, 0.0f);
	XMStoreFloat4x4(&m_SkullWorld, T);
}

bool Shapes::Init()
{
	if (!FrameWork::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	D3D11_RASTERIZER_DESC wireframe;
	ZeroMemory(&wireframe, sizeof(wireframe));
	wireframe.FillMode = D3D11_FILL_WIREFRAME;
	wireframe.CullMode = D3D11_CULL_BACK;
	wireframe.FrontCounterClockwise = false;
	wireframe.DepthClipEnable = true;

	m_spDevice->CreateRasterizerState(&wireframe, m_WireframeRS.GetAddressOf());

	return true;
}

void Shapes::Update(float dt)
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

void Shapes::RenderFrame()
{
	float color[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
	m_spDeviceCon->ClearRenderTargetView(m_spTargetView.Get(), color);
	m_spDeviceCon->ClearDepthStencilView(m_spDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	m_spDeviceCon->IASetInputLayout(m_InputLayout.Get());
	m_spDeviceCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_spDeviceCon->RSSetState(m_WireframeRS.Get());
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_spDeviceCon->IASetVertexBuffers(0, 1, m_VB.GetAddressOf(), &stride, &offset);
	m_spDeviceCon->IASetIndexBuffer(m_IB.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	XMMATRIX view = XMLoadFloat4x4(&m_View);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);
	XMMATRIX ViewProj = view * proj;


	D3DX11_TECHNIQUE_DESC techDesc;
	m_Tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		XMMATRIX world = XMLoadFloat4x4(&m_SkullWorld);
		m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world * ViewProj)));
		m_Tech->GetPassByIndex(p)->Apply(0, m_spDeviceCon.Get());
		m_spDeviceCon->DrawIndexed(m_SkullIndexCount, 0, 0);
	}

	m_spSwapchain->Present(0, 0);
}

void Shapes::OnResize()
{
	FrameWork::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, P);
}

void Shapes::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	SetCapture(m_hWnd);
}

void Shapes::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Shapes::OnMouseMove(WPARAM btnState, int x, int y)
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

void Shapes::BuildGeometryBuffers()
{
	ifstream file("Models/skull.txt");

	UINT vertexcnt = 0;
	UINT tricnt = 0;
	char ignore[256] = { 0, };

	file >> ignore >> vertexcnt;
	file >> ignore >> tricnt;
	file >> ignore >> ignore >> ignore >> ignore;

	float nx, ny, nz;
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	vector<Vertex> vertices(vertexcnt);

	for (int i = 0; i < vertexcnt; i++)
	{
		file >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z >> nx >> ny >> nz;
		vertices[i].Color = black;
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
	vbd.ByteWidth = sizeof(Vertex) * vertexcnt;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	m_spDevice->CreateBuffer(&vbd, &vinitData, &m_VB);

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_SkullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	m_spDevice->CreateBuffer(&ibd, &iinitData, &m_IB);
}

void Shapes::BuildFX()
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

void Shapes::BuildVertexLayout()
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

void Shapes::OutputShaderErrorMsg(ID3D10Blob* errMsg, HWND hWnd, LPCWSTR shaderfileName) const
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
