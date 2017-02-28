/*
	@file			Main.cpp
	@brief		プロジェクトのメイン
	@date		2017/02/27
	@author	仁科香苗
*/
#include "DirectX.h"

/*
	@brief	グローバル変数
*/
DirectX* pDirectX;
HWND wnd;

/*
	@brief	関数のプロトタイプ宣言
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT InitWindow(HINSTANCE hInstance);

/*
	@brief	アプリケーションのエントリーポイント
*/
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, INT)
{
	pDirectX = new DirectX;
	if (pDirectX != NULL)
	{
		if (SUCCEEDED(InitWindow(hInstance)))
		{
			if (SUCCEEDED(pDirectX->InitD3D(wnd)))
			{
				pDirectX->Loop();
			}
		}
		//アプリ終了
		delete pDirectX;
	}
	return 0;
}

/*
	@brief	OSから見たウィンドウプロシージャー
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return pDirectX->MsgProc(hWnd, uMsg, wParam, lParam);
}

/*
	@brief	ウィンドウ初期化
*/
HRESULT InitWindow(HINSTANCE hInstance)
{
	// ウィンドウの定義
	WNDCLASSEX  wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszClassName = GAME_NAME;
	wc.hIconSm = LoadIcon(hInstance, NULL);
	RegisterClassEx(&wc);

	//ウィンドウの作成
	wnd = CreateWindow(GAME_NAME, GAME_NAME, WS_OVERLAPPEDWINDOW,
		0, 0, window_width, window_height, 0, 0, hInstance, 0);
	if (!wnd)
	{
		return E_FAIL;
	}
	//ウインドウの表示
	ShowWindow(wnd, SW_SHOW);
	UpdateWindow(wnd);



	return S_OK;
}
