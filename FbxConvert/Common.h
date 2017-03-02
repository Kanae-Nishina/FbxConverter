/*
	@file	Common.h
	@brief	��ʓI�Ȓ�`��C���N���[�h�n
	@date	2017/02/27
	@autho	�m�ȍ��c
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

//�K�v�ȃ��C�u�����t�@�C���̃��[�h
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx10.lib")
#pragma comment(lib,"d3dx11.lib")
#pragma comment(lib,"d3dCompiler.lib")

//�E�B���h�E�T�C�Y
const int window_width = 800;
const int window_height = 600;

//FPS
const int fps = 60;

//�E�B���h�E��
#define GAME_NAME L"FbxConvert"

//�}�N����`
#define SAFE_DELETE(x) if(x){delete x; x=nullptr; };
#define SAFE_RELEASE(x) if(x){x->Release(); x=nullptr;}
#define SAFE_DELETE_ARRAY(x)  if( x != nullptr ){ delete[] x;  x = nullptr; }