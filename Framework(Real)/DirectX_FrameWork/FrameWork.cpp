#include "stdafx.h"
#include "FrameWork.h"

FrameWork* gpDispatch = 0;

LRESULT CALLBACK g_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return gpDispatch->WndProc(hWnd, message, wParam, lParam);
}

FrameWork::FrameWork() :
	mScreenHeight(SCREENHEIGHT),
	mScreenWidth(SCREENWIDTH),
	m_Paused(false),
	mMinimized(false),
	mMaximized(false),
	mResizing(false),
	m_hWnd(NULL)
{
	gpDispatch = this;
}

void FrameWork::InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	ZeroMemory(&wcex, sizeof(WNDCLASSEXW));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = g_WindowProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wcex.lpszClassName = _T("FrameWork");

	RegisterClassEx(&wcex);

	RECT wr = { 0,0,static_cast<LONG>(mScreenWidth),static_cast<LONG>(mScreenHeight) };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	m_hWnd = CreateWindow(
		_T("FrameWork"),
		_T("FrameWork"),
		WS_OVERLAPPEDWINDOW,
		300,
		100,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(m_hWnd, SW_SHOW);

	InitD3D(m_hWnd);
}

void FrameWork::CalcFPS()
{
	static int frameCnt = 0;
	static float timeElaped = 0.0f;

	frameCnt++;
	
	if ((m_Timer.TotalTime() - timeElaped) >= 1.0f)
	{
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;

		wostringstream outs;
		outs.precision(3);
		outs<< L"FPS: " << fps << L"             "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowTextW(m_hWnd, outs.str().c_str());

		frameCnt = 0;
		timeElaped += 1.0f;
	}
}

void FrameWork::InitD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC scd;

	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Width = mScreenWidth;
	scd.BufferDesc.Height = mScreenHeight;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 4;
	scd.Windowed = TRUE;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		m_spSwapchain.GetAddressOf(),
		m_spDevice.GetAddressOf(),
		NULL,
		m_spDeviceCon.GetAddressOf());

	//스마트포인터생성
	m_spKeyboard = make_unique<Keyboard>();
	m_spMouse = make_unique<Mouse>();
	m_spMouse->SetWindow(hWnd);

	OnResize();
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
		mScreenWidth = LOWORD(lParam);
		mScreenHeight = HIWORD(lParam);

		if (m_spDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_Paused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_Paused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				if (mMinimized)
				{
					m_Paused = false;
					mMinimized = false;
					OnResize();
				}
				else if (mMaximized)
				{
					m_Paused = false;
					mMaximized = false;
					OnResize();
				}
				else if (mResizing)
				{

				}
				else
					OnResize();
			}
		}
		return 0;
		break;

	case WM_ENTERSIZEMOVE:
		m_Paused = true;
		mResizing = true;
		m_Timer.Stop();
		return 0;
		break;

	case WM_EXITSIZEMOVE:
		m_Paused = false;
		mResizing = false;
		m_Timer.Resume();
		OnResize();
		return 0;
		break;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);
		break;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 640;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 480;
		return 0;
		break;

	case WM_ACTIVATEAPP: //윈도가 활성화 되었을 때 발생
		Keyboard::ProcessMessage(message, wParam, lParam);
		Mouse::ProcessMessage(message, wParam, lParam);
		m_KeyboardTracker.Reset();
		m_MouseTracker.Reset();
		break;

	case WM_MOUSEHOVER:
		Mouse::ProcessMessage(message, wParam, lParam);
		break;

	case WM_SYSKEYUP:
		Keyboard::ProcessMessage(message, wParam, lParam);
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
	m_spSwapchain->ResizeBuffers(1, mScreenWidth, mScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	ComPtr<ID3D11Texture2D> pBackBuffer;
	m_spSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)pBackBuffer.GetAddressOf()); //후면버퍼를 얻어온다.

	m_spDevice->CreateRenderTargetView(pBackBuffer.Get(), NULL, m_spTargetView.ReleaseAndGetAddressOf()); //후면버퍼를 이용해 리소스 뷰를 만든다.

	D3D11_TEXTURE2D_DESC dsd;
	ZeroMemory(&dsd, sizeof(D3D11_TEXTURE2D_DESC));
	dsd.Width = mScreenWidth;
	dsd.Height = mScreenHeight;
	dsd.MipLevels = 1;
	dsd.ArraySize = 1;
	dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsd.SampleDesc.Count = 4;
	dsd.Usage = D3D11_USAGE_DEFAULT;
	dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	m_spDevice->CreateTexture2D(&dsd, 0, m_spDepthStencilBuffer.ReleaseAndGetAddressOf());
	m_spDevice->CreateDepthStencilView(m_spDepthStencilBuffer.Get(), 0, m_spDepthStencilView.ReleaseAndGetAddressOf());
	m_spDeviceCon->OMSetRenderTargets(1, m_spTargetView.GetAddressOf(), m_spDepthStencilView.Get());

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<LONG>(mScreenWidth);
	viewport.Height = static_cast<LONG>(mScreenHeight);

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

				m_KeyboardTracker.Update(m_spKeyboard->GetState());
				m_MouseTracker.Update(m_spMouse->GetState());

				//Update(m_Timer.DeltaTime());
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