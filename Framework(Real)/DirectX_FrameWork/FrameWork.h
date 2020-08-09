#pragma once
#include "Timer.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

class FrameWork
{
public:
	FrameWork(HINSTANCE hInstance);
	virtual ~FrameWork() = default;

protected:
	ComPtr<IDXGISwapChain>  m_spSwapchain;
	ComPtr<ID3D11Device>	m_spDevice;
	ComPtr<ID3D11DeviceContext> m_spDeviceCon;
	ComPtr<ID3D11RenderTargetView> m_spTargetView;
	ComPtr<ID3D11Texture2D> m_spDepthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> m_spDepthStencilView;

	D3D11_VIEWPORT m_ScreenViewport;

	HINSTANCE m_hAppInst;
	HWND m_hWnd;
	bool m_Paused;

	enum { SCREENWIDTH = 1280 , SCREENHEIGHT = 800 };

	UINT m_ScreenWidth;
	UINT m_ScreenHeight;

	bool m_Minimized;
	bool m_Maximized;
	bool m_Resizing;
	UINT m_4xMsaaQuality;

	Timer m_Timer;

	wstring m_MainWndCaption;
	D3D_DRIVER_TYPE m_d3dDriverType;
	bool m_Enable4xMsaa;

public:
	virtual bool Init();
	virtual void ClearD3D();
	//virtual void Update(float dt) = 0;
	virtual void RenderFrame();
	virtual void OnResize();
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	inline HINSTANCE getInst() const;
	inline HWND getMainWnd() const;
	inline float AspectRatio() const;

	int GameLoop();

	virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
	virtual void OnMouseMove(WPARAM btnState, int x, int y) {}

protected:
	void CalcFPS();
	bool InitD3D();
	bool InitWindow();
};

HINSTANCE FrameWork::getInst() const
{
	return m_hAppInst;
}
 
HWND FrameWork::getMainWnd() const
{
	return m_hWnd;
}

float FrameWork::AspectRatio() const
{
	return static_cast<float>(m_ScreenWidth) / m_ScreenHeight;
}


