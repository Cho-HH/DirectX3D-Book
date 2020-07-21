#include "stdafx.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	FrameWork FrameWork;

	FrameWork.InitWindow(hInstance);

	return FrameWork.GameLoop();
}