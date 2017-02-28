/*
	@file			DirectX.h
	@brief		DirectXの準備クラス
	@date		2017/02/27
	@author	仁科香苗
*/
#include "DirectX.h"
/*
	@brief	コンストラクタ
*/
DirectX::DirectX()
	: m_device(nullptr)
	, m_deviceContext(nullptr)
	, m_swapChain(nullptr)
	, m_backBuffer_TexRTV(nullptr)
	, m_backBuffer_DSTexDSV(nullptr)
	, m_backBuffer_DSTex(nullptr)
{
}

/*
	@brief	デストラクタ
*/
DirectX::~DirectX()
{
}


/*
	@brief	DirectXの初期化
*/
HRESULT DirectX::InitD3D(HWND wnd)
{
	m_wnd = wnd;
	// デバイスとスワップチェーンの作成
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = window_width;
	sd.BufferDesc.Height = window_height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_wnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL pFeatureLevels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL* pFeatureLevel = NULL;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		0, &pFeatureLevels, 1, D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device,
		pFeatureLevel, &m_deviceContext)))
	{
		return FALSE;
	}
	//各種テクスチャーと、それに付帯する各種ビューを作成

	//バックバッファーテクスチャーを取得（既にあるので作成ではない）
	ID3D11Texture2D *pBackBuffer_Tex;
	m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer_Tex);
	//そのテクスチャーに対しレンダーターゲットビュー(RTV)を作成
	m_device->CreateRenderTargetView(pBackBuffer_Tex, NULL, &m_backBuffer_TexRTV);
	SAFE_RELEASE(pBackBuffer_Tex);

	//デプスステンシルビュー用のテクスチャーを作成
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = window_width;
	descDepth.Height = window_height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	m_device->CreateTexture2D(&descDepth, NULL, &m_backBuffer_DSTex);
	//そのテクスチャーに対しデプスステンシルビュー(DSV)を作成
	m_device->CreateDepthStencilView(m_backBuffer_DSTex, NULL, &m_backBuffer_DSTexDSV);

	//レンダーターゲットビューと深度ステンシルビューをパイプラインにバインド
	m_deviceContext->OMSetRenderTargets(1, &m_backBuffer_TexRTV, m_backBuffer_DSTexDSV);
	//ビューポートの設定
	D3D11_VIEWPORT vp;
	vp.Width = window_width;
	vp.Height = window_height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_deviceContext->RSSetViewports(1, &vp);
	//ラスタライズ設定
	D3D11_RASTERIZER_DESC rdc;
	ZeroMemory(&rdc, sizeof(rdc));
	rdc.CullMode = D3D11_CULL_NONE;
	rdc.FillMode = D3D11_FILL_SOLID;
	ID3D11RasterizerState* pIr = NULL;
	m_device->CreateRasterizerState(&rdc, &pIr);
	m_deviceContext->RSSetState(pIr);
	SAFE_RELEASE(pIr);

	return S_OK;
}

/*
@brief	ウィンドウプロシージャ
*/
LRESULT DirectX::MsgProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_KEYDOWN:
		switch ((char)wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

/*
@brief	アプリケーションの初期化
*/
void DirectX::AppInit()
{
}

/*
@brief	アプリケーションの更新
*/
void DirectX::Update()
{
	//描画
	//画面クリア
	float ClearColor[4] = { 0,0,0,0 };
	m_deviceContext->ClearRenderTargetView(m_backBuffer_TexRTV, ClearColor);
	m_deviceContext->ClearDepthStencilView(m_backBuffer_DSTexDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//カメラの設定
	SetCamera();

	//画面の更新
	m_swapChain->Present(0, 0);
}

/*
@brief	描画の為のカメラ設定
*/
void  DirectX::SetCamera()
{
}

/*
@brief	アプリケーション処理
*/
void DirectX::Loop()
{
	//アプリケーションの初期化
	AppInit();

	// メッセージループ
	MSG msg = { 0 };
	m_startClock = timeGetTime();		//クロックの取得
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Update();	//更新

						//FPS調整
			while (timeGetTime() - m_startClock < 1000 / fps)
			{
				Sleep(1);
			}
			m_startClock = timeGetTime();
		}
	}

	//アプリケーション終了
	DestroyD3D();
}

/*
@brief	解放処理
*/
void DirectX::DestroyD3D()
{
	SAFE_RELEASE(m_swapChain);
	SAFE_RELEASE(m_backBuffer_TexRTV);
	SAFE_RELEASE(m_backBuffer_DSTexDSV);
	SAFE_RELEASE(m_backBuffer_DSTex);
	SAFE_RELEASE(m_device);
	SAFE_RELEASE(m_deviceContext);
}