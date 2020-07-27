#pragma once
#include "Timer.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

const int g_ScreenWidth = 960;
const int g_SCreenHeight = 600;

class FrameWork
{
public:
	FrameWork();
	virtual ~FrameWork();

protected:
	ComPtr<IDXGISwapChain>  m_spSwapchain;
	ComPtr<ID3D11Device>	m_spDevice;
	ComPtr<ID3D11DeviceContext> m_spDeviceCon;
	ComPtr<ID3D11RenderTargetView> m_spTargetView;
	ComPtr<ID3D11Texture2D> m_spDepthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> m_spDepthStencilView;

	unique_ptr<Keyboard> m_spKeyboard;
	Keyboard::KeyboardStateTracker m_KeyboardTracker;

	unique_ptr<Mouse> m_spMouse;
	Mouse::ButtonStateTracker m_MouseTracker;

	UINT mScreenWidth;
	UINT mScreenHeight;

	bool mMinimized;
	bool mMaximized;
	bool mResizing;

	bool m_Paused;
	HWND m_hWnd;	
	Timer m_Timer;

protected:
	void CalcFPS();

public:
	virtual void InitD3D(HWND hWnd);
	virtual void ClearD3D();
	//virtual void Update(float dt) = 0;
	virtual void RenderFrame();
	virtual void OnResize();

public:
	void InitWindow(HINSTANCE hInstance);
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	int GameLoop();
};

