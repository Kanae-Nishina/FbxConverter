/*
	@file			DirectX.h
	@brief		DirectXの準備クラス
	@date		2017/02/27
	@author	仁科香苗
*/
#pragma once
//インクルードファイル
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

//必要なライブラリファイルのロード
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

	//FBXがちゃんと描画されるか確認用
	FbxLoader m_loader;
	FbxModel m_model;
	D3DMATRIX view;

	long m_startClock;								//開始時のクロック数

	void AppInit();							//アプリケーションの初期化
	void Update();							//アプリケーションの更新
	void SetCamera();						//描画の為のマトリックス設定
	void DestroyD3D();					//解放処理
public:
	DirectX();
	~DirectX();

	HRESULT InitD3D(HWND wnd);																		//DirectXの初期化
	LRESULT MsgProc(HWND wnd, UINT iMsg, WPARAM wParam, LPARAM lParam);	//ウィンドウプロシージャ
	void Loop();																									//アプリケーション処理
};