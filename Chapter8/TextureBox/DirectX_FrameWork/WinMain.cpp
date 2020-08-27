#include "stdafx.h"
#include "TextureBox.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	TextureBox FrameWork(hInstance);

	if (!FrameWork.Init())
		return 0;

	return FrameWork.GameLoop();
}