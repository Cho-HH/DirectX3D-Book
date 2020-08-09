#include "stdafx.h"
#include "FrameWork.h"

FrameWork* gpDispatch = 0;

LRESULT CALLBACK g_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return gpDispatch->WndProc(hWnd, message, wParam, lParam);
}

FrameWork::FrameWork(HINSTANCE hInstance) :
	m_hAppInst(hInstance),
	m_hWnd(NULL),
	m_Paused(false),
	m_ScreenHeight(SCREENHEIGHT),
	m_ScreenWidth(SCREENWIDTH),
	m_Minimized(false),
	m_Maximized(false),
	m_Resizing(false),
	m_4xMsaaQuality(NULL),
	m_MainWndCaption(L"기본 프레임워크"),
	m_d3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	m_Enable4xMsaa(NULL)
{
	ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));
	gpDispatch = this;
}

bool FrameWork::Init()
{
	if (!InitWindow())
		return false;

	if (!InitD3D())
		return false;

	return true;
}

bool FrameWork::InitWindow()
{
	WNDCLASSEX wcex;

	ZeroMemory(&wcex, sizeof(WNDCLASSEXW));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = g_WindowProc;
	wcex.hInstance = m_hAppInst;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wcex.lpszClassName = _T("FrameWorkClass");

	if (!RegisterClassEx(&wcex))
	{
		MessageBoxW(NULL, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT wr = { 0,0,static_cast<LONG>(m_ScreenWidth),static_cast<LONG>(m_ScreenHeight)};
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	
	m_hWnd = CreateWindowW(
		L"FrameWorkClass",
		m_MainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW,
		(GetSystemMetrics(SM_CXSCREEN)- m_ScreenWidth)/2,
		(GetSystemMetrics(SM_CYSCREEN) - m_ScreenHeight) / 2,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		m_hAppInst,
		NULL);

	if (!m_hWnd)
	{
		MessageBoxW(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return true;
}

bool FrameWork::InitD3D()
{
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		0,                
		m_d3dDriverType,
		0,             
		createDeviceFlags,
		0, 0,           
		D3D11_SDK_VERSION,
		m_spDevice.GetAddressOf(),
		&featureLevel,
		m_spDeviceCon.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBoxW(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBoxW(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	hr = m_spDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality);
	assert(m_4xMsaaQuality > 0);

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Width = m_ScreenWidth;
	scd.BufferDesc.Height = m_ScreenHeight;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = m_hWnd;
	scd.SampleDesc.Count = 4;
	scd.Windowed = TRUE;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	hr = D3D11CreateDeviceAndSwapChain(NULL,
		m_d3dDriverType,
		NULL,
		createDeviceFlags,
		NULL, NULL,
		D3D11_SDK_VERSION,
		&scd,
		m_spSwapchain.GetAddressOf(),
		m_spDevice.GetAddressOf(),
		&featureLevel,
		m_spDeviceCon.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBoxW(0, L"D3D11CreateDevice or SwaoChain Failed.", 0, 0);
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBoxW(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	if (m_Enable4xMsaa)
	{
		scd.SampleDesc.Count = 4;
		scd.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
	}

	ComPtr<IDXGIDevice> dxgiDevice = NULL;
	hr = m_spDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)dxgiDevice.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxW(NULL, L"IDXGIDevice Faild.", 0, MB_OK);
		return false;
	}

	ComPtr<IDXGIAdapter> dxgiAdapter = 0;
	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)dxgiAdapter.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxW(NULL, L"IDXGIAdapter Failed.", 0, MB_OK);
		return false;
	}

	ComPtr<IDXGIFactory> dxgiFactory = 0;
	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)dxgiFactory.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxW(NULL, L"IDXGIFactory Failed.", 0, MB_OK);
		return false;
	}

	OnResize();

	return true;
}

void FrameWork::CalcFPS()
{
	//1밀리초(ms)=1/1000초

	static int frameCnt = 0;
	static float timeElaped = 0.0f;

	frameCnt++;
	
	if ((m_Timer.TotalTime() - timeElaped) >= 1.0f)
	{
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;

		wostringstream outs;
		outs.precision(3);
		outs << L"FPS: " << fps << L"             "
			<< L"Frame Time: " << mspf << L" (ms) " <<setw(50) << m_MainWndCaption;
		SetWindowTextW(m_hWnd, outs.str().c_str());
	
		frameCnt = 0;
		timeElaped += 1.0f;
	}
}

LRESULT FrameWork::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_Paused = true;
			m_Timer.Stop();
		}
		else
		{
			m_Paused = false;
			m_Timer.Resume();
		}
		break;

	case WM_SIZE:
		m_ScreenWidth = LOWORD(lParam);
		m_ScreenHeight = HIWORD(lParam);

		if (m_spDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_Paused = true;
				m_Minimized = true;
				m_Maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_Paused = false;
				m_Minimized = false;
				m_Maximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				if (m_Minimized)
				{
					m_Paused = false;
					m_Minimized = false;
					OnResize();
				}
				else if (m_Maximized)
				{
					m_Paused = false;
					m_Maximized = false;
					OnResize();
				}
				else if (m_Resizing)
				{

				}
				else
					OnResize();
			}
		}
		return 0;

	case WM_ENTERSIZEMOVE:
		m_Paused = true;
		m_Resizing = true;
		m_Timer.Stop();
		break;

	case WM_EXITSIZEMOVE:
		m_Paused = false;
		m_Resizing = false;
		m_Timer.Start();
		OnResize();
		break;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO: //최소화면
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 640;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 480;
		break;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void FrameWork::OnResize()
{
	//이전뷰가 버퍼에 대한 참조를 가지고 있으므로 해제하고 사용

	m_spSwapchain->ResizeBuffers(1, m_ScreenWidth, m_ScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	ComPtr<ID3D11Texture2D> pBackBuffer;
	m_spSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)pBackBuffer.GetAddressOf()); //후면버퍼를 얻어온다.

	m_spDevice->CreateRenderTargetView(pBackBuffer.Get(), NULL, m_spTargetView.ReleaseAndGetAddressOf()); //후면버퍼를 이용해 리소스 뷰를 만든다.

	D3D11_TEXTURE2D_DESC dsd;
	ZeroMemory(&dsd, sizeof(D3D11_TEXTURE2D_DESC));
	dsd.Width = m_ScreenWidth;
	dsd.Height = m_ScreenHeight;
	dsd.MipLevels = 1;
	dsd.ArraySize = 1;
	dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsd.SampleDesc.Count = 4;
	dsd.Usage = D3D11_USAGE_DEFAULT;
	dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (m_Enable4xMsaa)
	{
		dsd.SampleDesc.Count = 4;
		dsd.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		dsd.SampleDesc.Count = 1;
		dsd.SampleDesc.Quality = 0;
	}

	m_spDevice->CreateTexture2D(&dsd, 0, m_spDepthStencilBuffer.ReleaseAndGetAddressOf());
	m_spDevice->CreateDepthStencilView(m_spDepthStencilBuffer.Get(), 0, m_spDepthStencilView.ReleaseAndGetAddressOf());
	m_spDeviceCon->OMSetRenderTargets(1, m_spTargetView.GetAddressOf(), m_spDepthStencilView.Get());

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(m_ScreenWidth);
	viewport.Height = static_cast<float>(m_ScreenHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_spDeviceCon->RSSetViewports(1, &viewport);
}

int FrameWork::GameLoop()
{
	m_Timer.Start();

	MSG msg = { 0, };

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_Timer.Update();

			if (!m_Paused)
			{
				CalcFPS();
				Update(m_Timer.DeltaTime());
				RenderFrame();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	ClearD3D();
	return (int)msg.wParam;
}

void FrameWork::RenderFrame()
{
	float color[4] = { 0.0f, 0.2f, 0.3f, 1.0f };
	m_spDeviceCon->ClearRenderTargetView(m_spTargetView.Get(), color);
	m_spDeviceCon->ClearDepthStencilView(m_spDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);


	m_spSwapchain->Present(0, 0);
}

void FrameWork::ClearD3D()
{
	m_spSwapchain->SetFullscreenState(FALSE, NULL);
}