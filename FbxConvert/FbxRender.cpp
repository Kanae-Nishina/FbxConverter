/*
	@file			FbxRender.cpp
	@brief		FBXファイルの描画
	@date		2017/02/28
	@author	仁科香苗
	@note		参照(https://github.com/shaderjp/FBXLoader2015forDX11)
*/
#include "FbxRender.h"
namespace FBX_LOADER
{

	//=================================
	//	MATERIAL_DATA構造体
	//=================================
	/*
		@brief	コンストラクタ
	*/
	MATERIAL_DATA::MATERIAL_DATA()
		:srv(nullptr)
		, sampler(nullptr)
		, materialCb(nullptr)
	{
	}

	/*
		@brief	解放
	*/
	void MATERIAL_DATA::Release()
	{
		if (materialCb)
		{
			materialCb->Release();
			materialCb = nullptr;
		}

		if (srv)
		{
			srv->Release();
			srv = nullptr;
		}

		if (sampler)
		{
			sampler->Release();
			sampler = nullptr;
		}
	}

	//=================================
	//	MESH_NODE構造体
	//=================================
	/*
		@brief	コンストラクタ
	*/
	MESH_NODE::MESH_NODE()
		:vb(nullptr)
		, ib(nullptr)
		, vertexCount(0)
		, indexCount(0)
	{
	}

	/*
		@brief	解放
	*/
	void MESH_NODE::Release()
	{
		materialData.Release();

		/*if (inputLayout)
		{
			inputLayout->Release();
			inputLayout = nullptr;
		}*/
		if (ib)
		{
			ib->Release();
			ib = nullptr;
		}
		if (vb)
		{
			vb->Release();
			vb = nullptr;
		}
	}

	//=================================
	//	FbxRender構造体
	//=================================
	/*
		@brief	コンストラクタ
	*/
	FbxRender::FbxRender()
		:m_loader(nullptr)
	{
	}

	/*
		@brief	デストラクタ
	*/
	FbxRender::~FbxRender()
	{
		Release();
	}

	/*
		@brief	解放
	*/
	void FbxRender::Release()
	{
		for (auto m : m_meshNodeArray)
		{
			m.Release();
		}
		m_meshNodeArray.clear();

		if (m_loader)
		{
			delete m_loader;
			m_loader = nullptr;
		}
	}

	/*
		@brief	FBXの読み込み
	*/
	HRESULT FbxRender::LoadFBX(const char* fileName, ID3D11Device* device11)
	{
		if (!fileName || !device11)
		{
			return E_FAIL;
		}
		HRESULT hr = S_OK;

		m_loader = new FbxLoader;
		hr = m_loader->LoadFBX(fileName);
		if (FAILED(hr))
		{
			return hr;
		}
		hr = CreateNodes(device11);
		if (FAILED(hr))
		{
			return hr;
		}
		return hr;
	}

	/*
		@brief	ノード作成
	*/
	HRESULT FbxRender::CreateNodes(ID3D11Device* device11)
	{
		if (!device11)
		{
			return E_FAIL;
		}
		HRESULT hr = S_OK;

		size_t nodeCount = m_loader->GetNodesCount();
		if (nodeCount == 0)
		{
			return E_FAIL;
		}
		for (size_t i = 0; i < nodeCount; i++)
		{
			MESH_NODE meshNode;
			FBX_MESH_NODE fbxNode = m_loader->GetNode(static_cast<unsigned int>(i));
			VertexConstruction(device11, fbxNode, meshNode);

			//インデックスバッファー作成
			meshNode.indexCount = static_cast<DWORD>(fbxNode.indexArray.size());
			if (fbxNode.indexArray.size() > 0)
			{
				hr = CreateIndexBuffer(device11, &meshNode.ib, &fbxNode.indexArray[0], static_cast<uint32_t>(fbxNode.indexArray.size()));
			}
			memcpy(meshNode.mat4x4, fbxNode.mat4x4, sizeof(float) * 16);
			//マテリアル
			MaterialConstruction(device11, fbxNode, meshNode);
			m_meshNodeArray.push_back(meshNode);
		}
		return hr;
	}

	/*
		@broef	バーテックスバッファーの作成
	*/
	HRESULT FbxRender::CreateVertexBuffer(ID3D11Device*	pd3dDevice, ID3D11Buffer** pBuffer, void* pVertices, uint32_t stride, uint32_t vertexCount)
	{
		if (!pd3dDevice || stride == 0 || vertexCount == 0)
		{
			return E_FAIL;
		}

		HRESULT hr = S_OK;

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = stride * vertexCount;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));

		InitData.pSysMem = pVertices;

		hr = pd3dDevice->CreateBuffer(&bd, &InitData, pBuffer);
		if (FAILED(hr))
			return hr;

		return hr;
	}

	/*
		@brief	インデックスバッファーの作成
	*/
	HRESULT FbxRender::CreateIndexBuffer(ID3D11Device*	pd3dDevice, ID3D11Buffer** pBuffer, void* pIndices, uint32_t indexCount)
	{
		if (!pd3dDevice || indexCount == 0)
		{
			return E_FAIL;
		}
		HRESULT hr = S_OK;
		size_t stride = sizeof(unsigned int);

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = static_cast<uint32_t>(stride*indexCount);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));

		InitData.pSysMem = pIndices;

		hr = pd3dDevice->CreateBuffer(&bd, &InitData, pBuffer);
		if (FAILED(hr))
		{
			return hr;
		}
		return hr;
	}

	/*
		@brief	バーテックスの構築
	*/
	HRESULT FbxRender::VertexConstruction(ID3D11Device*	pd3dDevice, FBX_MESH_NODE &fbxNode, MESH_NODE& meshNode)
	{
		meshNode.vertexCount = static_cast<DWORD>(fbxNode.positionArray.size());
		if (!pd3dDevice || meshNode.vertexCount == 0)
		{
			return E_FAIL;
		}
		HRESULT hr = S_OK;

		VERTEX_DATA*	pV = new VERTEX_DATA[meshNode.vertexCount];

		//	const size_t numUVSet = fbxNode.m_texcoordArray.size();

		for (size_t i = 0; i < meshNode.vertexCount; i++)
		{
			FbxVector4 v = fbxNode.positionArray[i];
			pV[i].position = D3DXVECTOR3((float)v.mData[0],
				(float)v.mData[1],
				(float)v.mData[2]);

			v = fbxNode.normalArray[i];

			pV[i].normal = D3DXVECTOR3((float)v.mData[0],
				(float)v.mData[1],
				(float)v.mData[2]);

			if ((float)fbxNode.texcoordArray.size() > 0)
			{
				// 今回はUV1つしかやらない
				// UVのV値反転
				pV[i].textureCood = D3DXVECTOR2((float)abs(1.0f - fbxNode.texcoordArray[i].mData[0]),
					(float)abs(1.0f - fbxNode.texcoordArray[i].mData[1]));
			}
			else
				pV[i].textureCood = D3DXVECTOR2(0, 0);
		}

		CreateVertexBuffer(pd3dDevice, &meshNode.vb, pV, sizeof(VERTEX_DATA), meshNode.vertexCount);

		if (pV)
		{
			delete[] pV;
		}
		return hr;
	}

	/*
		@brief	マテリアルの構築
	*/
	HRESULT FbxRender::MaterialConstruction(ID3D11Device*	pd3dDevice, FBX_MESH_NODE &fbxNode, MESH_NODE& meshNode)
	{
		if (!pd3dDevice || fbxNode.materialArray.size() == 0)
		{
			return E_FAIL;
		}
		HRESULT hr = S_OK;
		//for (int i = 0; i < m_meshNodeArray.size(); i++)
		//{
		FBX_MATERIAL_NODE fbxMaterial = fbxNode.materialArray[0];
		meshNode.materialData.specularPower = fbxMaterial.shininess;
		meshNode.materialData.transparencyFactor = fbxMaterial.transparencyFactor;

		meshNode.materialData.ambient
			= D3DXVECTOR4(fbxMaterial.ambient.r, fbxMaterial.ambient.g, fbxMaterial.ambient.b, fbxMaterial.ambient.a);
		meshNode.materialData.diffuse
			= D3DXVECTOR4(fbxMaterial.diffuse.r, fbxMaterial.diffuse.g, fbxMaterial.diffuse.b, fbxMaterial.diffuse.a);
		meshNode.materialData.specular
			= D3DXVECTOR4(fbxMaterial.specular.r, fbxMaterial.specular.g, fbxMaterial.specular.b, fbxMaterial.specular.a);
		meshNode.materialData.emmisive
			= D3DXVECTOR4(fbxMaterial.emmisive.r, fbxMaterial.emmisive.g, fbxMaterial.emmisive.b, fbxMaterial.emmisive.a);


		//Diffuseだけからテクスチャを読み込む
		if (fbxMaterial.diffuse.textureSetArray.size() > 0)
		{
			TextureSet::const_iterator it = fbxMaterial.diffuse.textureSetArray.begin();
			if (it->second.size())
			{
				std::string path = it->second[0];

				// June 2010の時から変更
				hr = D3DX11CreateShaderResourceViewFromFileA(pd3dDevice, path.c_str(), NULL, NULL, &meshNode.materialData.srv, NULL);

#if 0	使わなそう
				//// Todo: 決め打ちよくないけど暫定対応
				//// FBXのSDKだと文字列はcharなんだけど、こっちではwcharにしないといけない...
				WCHAR	wstr[512];
				size_t wLen = 0;
				mbstowcs_s(&wLen, wstr, path.size() + 1, path.c_str(), _TRUNCATE);
				CreateWICTextureFromFile(pd3dDevice, wstr, NULL, &meshNode.materialData.srv, 0);	// DXTexから
#endif
			}
		}
		//}

		// samplerstate
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = pd3dDevice->CreateSamplerState(&sampDesc, &meshNode.materialData.sampler);

		// material Constant Buffer
		D3D11_BUFFER_DESC bufDesc;
		ZeroMemory(&bufDesc, sizeof(bufDesc));
		bufDesc.ByteWidth = sizeof(MATERIAL_CONSTANT_DATA);
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = 0;

		hr = pd3dDevice->CreateBuffer(&bufDesc, NULL, &meshNode.materialData.materialCb);

		meshNode.materialData.materialConstantData.ambient = meshNode.materialData.ambient;
		meshNode.materialData.materialConstantData.diffuse = meshNode.materialData.ambient;
		meshNode.materialData.materialConstantData.specular = meshNode.materialData.specular;
		meshNode.materialData.materialConstantData.emmisive = meshNode.materialData.emmisive;


		return hr;
	}

	/*
		@brief	インプットレイアウトの作成
	*/
	HRESULT FbxRender::CreateInputLayout(ID3D11Device*	pd3dDevice, const void* pShaderBytecodeWithInputSignature, size_t BytecodeLength, D3D11_INPUT_ELEMENT_DESC* pLayout, unsigned int layoutSize)
	{
		// InputeLayoutは頂点シェーダのコンパイル結果が必要
		if (!pd3dDevice || !pShaderBytecodeWithInputSignature || !pLayout)
		{
			return E_FAIL;
		}
		HRESULT hr = S_OK;

		size_t nodeCount = m_meshNodeArray.size();

		for (size_t i = 0; i < nodeCount; i++)
		{
			pd3dDevice->CreateInputLayout(pLayout, layoutSize, pShaderBytecodeWithInputSignature, BytecodeLength, &m_meshNodeArray[i].inputLayout);
		}

		return hr;
	}

	/*
		@brief	一括描画
	*/
	HRESULT FbxRender::RenderAll(ID3D11DeviceContext* pImmediateContext)
	{
		size_t nodeCount = m_meshNodeArray.size();
		if (nodeCount == 0)
		{
			return S_OK;
		}
		HRESULT hr = S_OK;

		pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (size_t i = 0; i < nodeCount; i++)
		{
			MESH_NODE* node = &m_meshNodeArray[i];

			if (node->vertexCount == 0)
				continue;

			UINT stride = sizeof(VERTEX_DATA);
			UINT offset = 0;
			pImmediateContext->IASetVertexBuffers(0, 1, &node->vb, &stride, &offset);

			DXGI_FORMAT indexbit = DXGI_FORMAT_R32_UINT;
			pImmediateContext->IASetInputLayout(node->inputLayout);
			pImmediateContext->IASetIndexBuffer(node->ib, indexbit, 0);

			pImmediateContext->DrawIndexed(node->indexCount, 0, 0);
		}

		return hr;
	}

	/*
		@brief	ノードのレンダー
	*/
	HRESULT FbxRender::RenderNode(ID3D11DeviceContext* pImmediateContext, const size_t nodeId, ID3D11InputLayout* inputLayout)
	{
		size_t nodeCount = m_meshNodeArray.size();
		if (nodeCount == 0 || nodeCount <= nodeId)
		{
			return S_OK;
		}
		HRESULT hr = S_OK;

		MESH_NODE* node = &m_meshNodeArray[nodeId];

		if (node->vertexCount == 0)
		{
			return S_OK;
		}
		UINT stride = sizeof(VERTEX_DATA);
		UINT offset = 0;
		pImmediateContext->IASetVertexBuffers(0, 1, &node->vb, &stride, &offset);
		//pImmediateContext->IASetInputLayout(node->inputLayout);
		pImmediateContext->IASetInputLayout(inputLayout);
		pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// インデックスバッファが存在する場合
		if (node->indexBit != MESH_NODE::INDEX_NOINDEX)
		{
			DXGI_FORMAT indexbit = DXGI_FORMAT_R32_UINT;

			pImmediateContext->IASetIndexBuffer(node->ib, indexbit, 0);

			pImmediateContext->DrawIndexed(node->indexCount, 0, 0);
		}

		return hr;
	}

	/*
		@brief	描画するノードのインスタンス化
	*/
	HRESULT FbxRender::RenderNodeInstancing(ID3D11DeviceContext* pImmediateContext, const size_t nodeId, const uint32_t InstanceCount)
	{
		size_t nodeCount = m_meshNodeArray.size();
		if (nodeCount == 0 || nodeCount <= nodeId || InstanceCount == 0)
		{
			return S_OK;
		}
		HRESULT hr = S_OK;

		MESH_NODE* node = &m_meshNodeArray[nodeId];

		if (node->vertexCount == 0)
		{
			return S_OK;
		}
		UINT stride = sizeof(VERTEX_DATA);
		UINT offset = 0;
		pImmediateContext->IASetVertexBuffers(0, 1, &node->vb, &stride, &offset);
		pImmediateContext->IASetInputLayout(node->inputLayout);
		pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// インデックスバッファが存在する場合
		if (node->indexBit != MESH_NODE::INDEX_NOINDEX)
		{
			DXGI_FORMAT indexbit = DXGI_FORMAT_R32_UINT;

			pImmediateContext->IASetIndexBuffer(node->ib, indexbit, 0);

			pImmediateContext->DrawIndexedInstanced(node->indexCount, InstanceCount, 0, 0, 0);
		}

		return hr;
	}

	/*
		@brief 描画するノードの関節インスタンス化
	*/
	HRESULT FbxRender::RenderNodeInstancingIndirect(ID3D11DeviceContext* pImmediateContext, const size_t nodeId, ID3D11Buffer* pBufferForArgs, const uint32_t AlignedByteOffsetForArgs)
	{
		size_t nodeCount = m_meshNodeArray.size();
		if (nodeCount == 0 || nodeCount <= nodeId)
		{
			return S_OK;
		}
		HRESULT hr = S_OK;

		MESH_NODE* node = &m_meshNodeArray[nodeId];

		if (node->vertexCount == 0)
			return S_OK;

		UINT stride = sizeof(VERTEX_DATA);
		UINT offset = 0;
		pImmediateContext->IASetVertexBuffers(0, 1, &node->vb, &stride, &offset);
		pImmediateContext->IASetInputLayout(node->inputLayout);
		pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// インデックスバッファが存在する場合
		if (node->indexBit != MESH_NODE::INDEX_NOINDEX)
		{
			DXGI_FORMAT indexbit = DXGI_FORMAT_R32_UINT;

			pImmediateContext->IASetIndexBuffer(node->ib, indexbit, 0);

			pImmediateContext->DrawIndexedInstancedIndirect(pBufferForArgs, AlignedByteOffsetForArgs);
		}

		return hr;
	}
}