/*
	@file			Main.cpp
	@brief		�v���W�F�N�g�̃��C��
	@date		2017/02/27
	@author	�m�ȍ��c
*/
#include "DirectX.h"

/*
	@brief	�O���[�o���ϐ�
*/
DirectX* pDirectX;
HWND wnd;

/*
	@brief	�֐��̃v���g�^�C�v�錾
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT InitWindow(HINSTANCE hInstance);

/*
	@brief	�A�v���P�[�V�����̃G���g���[�|�C���g
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
		//�A�v���I��
		delete pDirectX;
	}
	return 0;
}

/*
	@brief	OS���猩���E�B���h�E�v���V�[�W���[
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return pDirectX->MsgProc(hWnd, uMsg, wParam, lParam);
}

/*
	@brief	�E�B���h�E������
*/
HRESULT InitWindow(HINSTANCE hInstance)
{
	// �E�B���h�E�̒�`
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

	//�E�B���h�E�̍쐬
	wnd = CreateWindow(GAME_NAME, GAME_NAME, WS_OVERLAPPEDWINDOW,
		0, 0, window_width, window_height, 0, 0, hInstance, 0);
	if (!wnd)
	{
		return E_FAIL;
	}
	//�E�C���h�E�̕\��
	ShowWindow(wnd, SW_SHOW);
	UpdateWindow(wnd);



	return S_OK;
}
