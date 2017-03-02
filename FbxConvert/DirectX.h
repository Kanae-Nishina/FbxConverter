/*
	@file			DirectX.h
	@brief		DirectX�̏����N���X
	@date		2017/02/27
	@author	�m�ȍ��c
*/
#pragma once
//�C���N���[�h�t�@�C��
#include "Common.h"
#include "Camera.h"
#include "FbxModel.h"

class DirectX
{
public:
	DirectX();
	~DirectX();

	HRESULT InitD3D(HWND wnd);																		//DirectX�̏�����
	LRESULT MsgProc(HWND wnd, UINT iMsg, WPARAM wParam, LPARAM lParam);	//�E�B���h�E�v���V�[�W��
	void Loop();

private:
	HWND								m_wnd;
	ID3D11Device*					m_device;
	ID3D11DeviceContext*		m_deviceContext;
	IDXGISwapChain*				m_swapChain;
	ID3D11RenderTargetView*	m_backBuffer_TexRTV;
	ID3D11DepthStencilView*	m_backBuffer_DSTexDSV;
	ID3D11Texture2D*				m_backBuffer_DSTex;

	//���f���`��m�F�p
	FbxModel* m_model;
	Camera* m_camera;

	long m_startClock;								//�J�n���̃N���b�N��

	void AppInit();							//�A�v���P�[�V�����̏�����
	void Update();							//�A�v���P�[�V�����̍X�V
	void SetCamera();						//�`��ׂ̈̃}�g���b�N�X�ݒ�
	void DestroyD3D();					//�������
																							//�A�v���P�[�V��������
};