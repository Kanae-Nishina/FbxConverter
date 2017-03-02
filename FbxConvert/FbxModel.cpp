/*
	@file			FbxModel.h
	@brief		FBXシェーダクラス
	@date		2017/03/01
	@author	仁科香苗
*/
#include "FbxModel.h"
/*
	@brief	コンストラクタ
*/
FbxModel::FbxModel()
	:m_blendState(nullptr)
	, m_rs(nullptr)
	, m_buffer(nullptr)
	, m_vsFBX(nullptr)
	, m_psFBX(nullptr)
	, m_vsFBXInstancing(nullptr)
	, m_inputLayout(nullptr)
	, m_transformStructuredBuffer(nullptr)
	, m_transformSRV(nullptr)
	, m_fbxRender(nullptr)
	, m_instanceMAX(32)
{
}

/*
	@brief	デストラクタ
*/
FbxModel::~FbxModel()
{
	SAFE_RELEASE(m_blendState);
	SAFE_RELEASE(m_rs);
	SAFE_RELEASE(m_buffer);
	SAFE_RELEASE(m_vsFBX);
	SAFE_RELEASE(m_psFBX);
	SAFE_RELEASE(m_vsFBXInstancing);
	SAFE_RELEASE(m_transformStructuredBuffer);
	SAFE_RELEASE(m_transformSRV);
	SAFE_RELEASE(m_fbxRender);
}

/*
	@brief	シェーダのコンパイル
*/
HRESULT FbxModel::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if  _DEBUG 
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		}
		if (pErrorBlob)
		{
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob)
	{
		pErrorBlob->Release();
	}

	return S_OK;
}

/*
	@brief	解放
*/
void FbxModel::Release()
{
	SAFE_RELEASE(m_transformSRV);
	SAFE_RELEASE(m_transformStructuredBuffer);
	SAFE_RELEASE(m_blendState);
	SAFE_RELEASE(m_fbxRender);
	SAFE_RELEASE(m_rs);
	SAFE_RELEASE(m_vsFBXInstancing);
	SAFE_RELEASE(m_vsFBX);
	SAFE_RELEASE(m_psFBX);
	SAFE_RELEASE(m_buffer);
}

/*
	@brief	初期化
*/
HRESULT FbxModel::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_d3dDevice = device;
	m_deviceContext = deviceContext;

	HRESULT hr = S_OK;
	//シェーダの初期化
	hr = InitShader();
	if (FAILED(hr))
	{
		return E_FAIL;
	}
	//シェーダのリソースビューの作成
	hr = CreateTransdormSRV();
	if (FAILED(hr))
	{
		return E_FAIL;
	}
	return S_OK;
}

/*
	@brief	シェーダの初期化
*/
HRESULT FbxModel::InitShader()
{
	// Create the vertex shader
	ID3DBlob* pVSBlob = NULL;
	HRESULT hr;

#if 0 こっちのシェーダは今は使わないので消す
	hr = CompileShaderFromFile(L"simpleRenderVS.hlsl", "vs_main", "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"GeometryShader.hlsl　.", L"Error", MB_OK);
		return hr;
	}
	hr = m_d3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_vsFBX);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}
	// Compile the vertex shader
	pVSBlob->Release();
#endif

	hr = CompileShaderFromFile(L"simpleRenderInstancingVS.hlsl", "vs_main", "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"GeometryShader.hlsl", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = m_d3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_vsFBXInstancing);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}


	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	//// Todo: InputLayoutの作成には頂点シェーダが必要なのでこんなタイミングでCreateするのをなんとかしたい
	//hr = m_fbxRender->CreateInputLayout(m_d3dDevice, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), layout, numElements);
	m_d3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_inputLayout);
	pVSBlob->Release();
	//if (FAILED(hr))
	//{
	//	return hr;
	//}
	//頂点インプットレイアウトをセット
	m_deviceContext->IASetInputLayout(m_inputLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(L"simpleRenderPS.hlsl", "PS", "ps_5_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"simpleRenderPS.hlsl", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = m_d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_psFBX);
	pPSBlob->Release();
	if (FAILED(hr))
	{
		return hr;
	}

	// Create Constant Buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(SIMPLECONSTANT_BUFFER);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;

	hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_buffer);
	if (FAILED(hr))
	{
		return hr;
	}

	//
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = FALSE;
	m_d3dDevice->CreateRasterizerState(&rsDesc, &m_rs);
	m_deviceContext->RSSetState(m_rs);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;      ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;     ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_d3dDevice->CreateBlendState(&blendDesc, &m_blendState);
}

/*
	@brief	シェーダのリソースビューの作成
*/
HRESULT FbxModel::CreateTransdormSRV()
{
	HRESULT hr = S_OK;

	const uint32_t count = m_instanceMAX;
	const uint32_t stride = static_cast<uint32_t>(sizeof(SRV_PER_INSTANCE_DATA));

	// Create StructuredBuffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = stride * count;
	bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = stride;
	hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_transformStructuredBuffer);
	if (FAILED(hr))
		return hr;

	// Create ShaderResourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;   // 拡張されたバッファーであることを指定する
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.BufferEx.NumElements = count;                  // リソース内の要素の数

	 // 構造化バッファーをもとにシェーダーリソースビューを作成する
	hr = m_d3dDevice->CreateShaderResourceView(m_transformStructuredBuffer, &srvDesc, &m_transformSRV);
	if (FAILED(hr))
	{
		return hr;
	}
	return hr;
}

/*
	@brief	モデルの読み込み
*/
HRESULT FbxModel::LoadFBX(const char* fileName)
{
	HRESULT hr = S_OK;
	m_fbxRender = new FBX_LOADER::FbxRender;
	hr = m_fbxRender->LoadFBX(fileName, m_d3dDevice);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FBXLoad::FbxModel.cpp", L"Error", MB_OK);
		return hr;
	}
}

/*
	@brief	描画
*/
void FbxModel::Render(D3DXVECTOR3 pos, float scale, float yaw)
{
	D3DXMATRIX World, Trans, Scale, Rot;
	D3DXMatrixTranslation(&Trans, pos.x, pos.y, pos.z);
	D3DXMatrixScaling(&Scale, scale, scale, scale);
	D3DXMatrixRotationY(&Rot, yaw);
	World = Scale*Rot*Trans;

	//頂点インプットレイアウトをセット
	m_deviceContext->IASetInputLayout(m_inputLayout);
	//プリミティブ・トポロジーをセット
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//シェーダのセット
	m_deviceContext->VSSetShader(m_vsFBXInstancing, NULL, 0);
	m_deviceContext->VSSetConstantBuffers(0, 1, &m_buffer);
	m_deviceContext->PSSetShader(m_psFBX, NULL, 0);

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	if (SUCCEEDED(m_deviceContext->Map(m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource)))
	{
		size_t nodeCount = m_fbxRender->GetNodeCount();	//FBXのノード数を取得
		//全ノードの描画
		for (size_t i = 0; i < nodeCount; i++)
		{
			SIMPLECONSTANT_BUFFER mat;
			D3DXMATRIX local;
			local = m_fbxRender->GetNodeMatrix(i);	// このnodeのMatrix
			mat.world = World;
			D3DXMatrixTranspose(&mat.world, &mat.world);
			mat.wvp = local*World*m_view*m_proj;
			D3DXMatrixTranspose(&mat.wvp,&mat.wvp);
			mat.lightDir = { 1,1,1,0 };
			mat.eye = { 1,1,1,0 };
			memcpy_s(MappedResource.pData, MappedResource.RowPitch, (void*)(&mat), sizeof(mat));
			m_deviceContext->Unmap(m_buffer, 0);
			//		//ワールド、カメラ、射影行列を渡す
			//		D3DXMATRIX m = World*m_view*m_proj;
			//		D3DXMatrixTranspose(&m, &m);
			//		mat.wvp = m;
			///*		m_fbxMatrix = (FBX_MATRIX*)MappedResource.pData;
			//		m_fbxMatrix->world = (World);
			//		m_fbxMatrix->view = (m_view);
			//		m_fbxMatrix->proj = (m_proj);*/

			//		//m_fbxMatrix->wvp = local*World*m_view*m_proj;
			//		//D3DXMatrixTranspose(&m_fbxMatrix->wvp, &m_fbxMatrix->wvp);
			//		memcpy_s(MappedResource.pData, MappedResource.RowPitch, (void*)(&mat), sizeof(mat));
			//		m_deviceContext->Unmap(m_buffer, 0);

					//SetMatrix();

			FBX_LOADER::MATERIAL_DATA material = m_fbxRender->GetNodeMaterial(i);

			if (material.materialCb)
			{
				m_deviceContext->UpdateSubresource(material.materialCb, 0, NULL, &material.materialConstantData, 0, 0);
			}
			m_deviceContext->VSSetShaderResources(0, 1, &m_transformSRV);
			m_deviceContext->PSSetShaderResources(0, 1, &material.srv);
			m_deviceContext->PSSetConstantBuffers(0, 1, &material.materialCb);
			m_deviceContext->PSSetSamplers(0, 1, &material.sampler);

			m_fbxRender->RenderNode(m_deviceContext, i, m_inputLayout);
		}
	}
}

/*
	@brief	カメラのセット
*/
void FbxModel::SetCamera(D3DXMATRIX view, D3DXMATRIX proj)
{
	m_view = view;
	m_proj = proj;
}