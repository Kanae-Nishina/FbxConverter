/*
	@file			DirectX.h
	@brief		DirectX�̏����N���X
	@date		2017/02/27
	@author	�m�ȍ��c
*/
#pragma once
//�C���N���[�h�t�@�C��
#include <stdio.h>
#include <windows.h>
#include <d3dx9.h>
#include <d3d11.h>
#include <d3dx10.h>
#include <d3dx11.h>
#include <d3dCompiler.h>
#include <assert.h>
#include "Common.h"
#include "FbxModel.h"
#include "FbxLoader.h"

//�K�v�ȃ��C�u�����t�@�C���̃��[�h
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx10.lib")
#pragma comment(lib,"d3dx11.lib")
#pragma comment(lib,"d3dCompiler.lib")

class DirectX
{
private:
	HWND								m_wnd;
	ID3D11Device*					m_device;
	ID3D11DeviceContext*		m_deviceContext;
	IDXGISwapChain*				m_swapChain;
	ID3D11RenderTargetView*	m_backBuffer_TexRTV;
	ID3D11DepthStencilView*	m_backBuffer_DSTexDSV;
	ID3D11Texture2D*				m_backBuffer_DSTex;

	//FBX�������ƕ`�悳��邩�m�F�p
	FbxLoader m_loader;
	FbxModel m_model;
	D3DMATRIX view;

	long m_startClock;								//�J�n���̃N���b�N��

	void AppInit();							//�A�v���P�[�V�����̏�����
	void Update();							//�A�v���P�[�V�����̍X�V
	void SetCamera();						//�`��ׂ̈̃}�g���b�N�X�ݒ�
	void DestroyD3D();					//�������
public:
	DirectX();
	~DirectX();

	HRESULT InitD3D(HWND wnd);																		//DirectX�̏�����
	LRESULT MsgProc(HWND wnd, UINT iMsg, WPARAM wParam, LPARAM lParam);	//�E�B���h�E�v���V�[�W��
	void Loop();																									//�A�v���P�[�V��������
};