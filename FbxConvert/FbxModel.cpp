/*
	@file			FbxModel.h
	@brief		FBXモデル描画及び周辺データの宣言
	@date		2017/02/27
	@author	仁科香苗
*/
#include "FbxModel.h"

//===========================
//	MaterialData構造体
//===========================
/*
	@brief	MaterialDataコンストラクタ
*/
MaterialData::MaterialData()
	:srv(nullptr)
	,sampler(nullptr)
	,materialCB(nullptr)
{
}

/*
	@brief 解放
*/
void MaterialData::Release()
{
	if (materialCB)
	{
		materialCB->Release();
		materialCB = nullptr;
	}

	if (sampler)
	{
		sampler->Release();
		sampler = nullptr;
	}

	if (srv)
	{
		srv->Release();
		srv = nullptr;
	}
}

//===========================
//	MeshNode構造体
//===========================
/*
	@brief	MeshNodeコンストラクタ
*/
MeshNode::MeshNode()
	:vb(nullptr)
	,ib(nullptr)
	,inputLayout(nullptr)
	,indexBit(IndexBit::eNone)
	,vertexCount(0)
	,indexCount(0)
{
}

/*
	@brief	解放
*/
void MeshNode::Release()
{
	materialData.Release();

	if (inputLayout)
	{
		inputLayout->Release();
		inputLayout = nullptr;
	}

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

/*
	@brief	インデックスのビット数を設定する
*/
void MeshNode::SetIndexBit(const size_t indexCount)
{
	indexBit = IndexBit::eNone;
	//32ビットだけ対応
	if (indexCount > 0)
	{
		indexBit = IndexBit::e32Bit;
	}
}

//===========================
//	FbxModelクラス
//===========================
/*
	@brief	コンストラクタ
*/
FbxModel::FbxModel()
{
}

/*
	@brief	デストラクタ
*/
FbxModel::~FbxModel()
{
}

/*
	@brief	読み込んだFBXから描画可能なデータを構築する
*/
bool FbxModel::Setup(FbxLoader* loader, ID3D11Device* device, ID3D11DeviceContext* context)
{
	std::size_t nodeCount = loader->GetNodeCount();
	if (nodeCount == 0)
	{
		return false;
	}

	// fbxノード分ループしてモデルを構築
	for (auto i = 0; i < nodeCount; i++)
	{
		MeshNode meshNode;
		auto& fbxNode = loader->GetNode(i);

		// 頂点データの構築
		VertexConstruction(device, fbxNode, meshNode);

		// インデックスの構築
		meshNode.indexCount = static_cast<DWORD>(fbxNode.indices.size());
		meshNode.SetIndexBit(meshNode.indexCount);
		if (fbxNode.indices.size() > 0)
		{
			CreateIndexBuffer(device,
				&meshNode.ib,
				&fbxNode.indices[0],
				static_cast<uint32_t>(fbxNode.indices.size()));
		}

		// マテリアル構築
		MaterialConstruction(device, fbxNode, meshNode);

		// モデル行列を保存
		D3DMATRIX mat =
		{
			fbxNode.mat4x4[0],fbxNode.mat4x4[1],fbxNode.mat4x4[2],fbxNode.mat4x4[3],
			fbxNode.mat4x4[4],fbxNode.mat4x4[5],fbxNode.mat4x4[6],fbxNode.mat4x4[7],
			fbxNode.mat4x4[8],fbxNode.mat4x4[9],fbxNode.mat4x4[10],fbxNode.mat4x4[11],
			fbxNode.mat4x4[12],fbxNode.mat4x4[13],fbxNode.mat4x4[14],fbxNode.mat4x4[15],
		};
		//meshNode.mat4x4 = D3DMATRIX(fbxNode.mat4x4);
		meshNode.mat4x4 = D3DMATRIX(mat);

		// 出来上がったモデルを、vector配列にプッシュ
		m_meshNodes.push_back(meshNode);
	}

	return true;
}

/*
	@brief	解放処理
*/
void FbxModel::Release()
{
	for (auto& node : m_meshNodes)
	{
		node.Release();
	}
	m_meshNodes.clear();
}

/*
	@brief	描画に必要なInputLayoutを生成する
*/
bool FbxModel::CreateInputLayout(ID3D11Device* device, const void* shaderBytecodeWithInputSignature,
	size_t bytecodeLength, const D3D11_INPUT_ELEMENT_DESC* desc, unsigned int numElements)
{
	if (!device || !shaderBytecodeWithInputSignature || !desc)
	{
		return false;
	}

	HRESULT hr;
	for (auto& meshNode : m_meshNodes)
	{
		hr = device->CreateInputLayout(desc,numElements,shaderBytecodeWithInputSignature,
			bytecodeLength,&meshNode.inputLayout);
	}

	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

/*
	@brief	全ノードを同じ設定で描画する
*/
void FbxModel::RenderAll(ID3D11DeviceContext* context)
{
	if (m_meshNodes.size() == 0)
	{
		return;
	}

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ノードを一気に描画
	for (auto& node : m_meshNodes)
	{
		if (node.vertexCount == 0)
		{
			continue;
		}

		// シェーダに流す頂点データの設定
		UINT stride = sizeof(VertexConstantData);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &node.vb, &stride, &offset);
		context->IASetInputLayout(node.inputLayout);

		// 今回はインデックス未使用のモデルは描画しないよ
		if (node.indexBit != MeshNode::eNone)
		{
			// インデックスバッファを設定
			DXGI_FORMAT indexBit = DXGI_FORMAT_R16_UINT;
			if (node.indexBit == MeshNode::e32Bit)
			{
				indexBit = DXGI_FORMAT_R32_UINT;
			}
			context->IASetIndexBuffer(node.ib, indexBit, 0);

			// 描画
			context->DrawIndexed(node.indexCount, 0, 0);
		}
	}
}

/*
	@brief	ノードを指定して描画
*/
void FbxModel::RenderNode(ID3D11DeviceContext* context, const int nodeId)
{
	if (m_meshNodes.size() == 0)
	{
		return;
	}

	auto& node = m_meshNodes[nodeId];
	if (node.vertexCount == 0)
	{
		return;
	}

	// シェーダに流す頂点データの設定
	UINT stride = sizeof(VertexConstantData);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &node.vb, &stride, &offset);
	context->IASetInputLayout(node.inputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 今回はインデックス未使用のモデルは描画しないよ
	if (node.indexBit != MeshNode::eNone)
	{
		// インデックスバッファを設定
		DXGI_FORMAT indexbit = DXGI_FORMAT_R16_UINT;
		if (node.indexBit == MeshNode::e32Bit)
		{
			indexbit = DXGI_FORMAT_R32_UINT;
		}
		context->IASetIndexBuffer(node.ib, indexbit, 0);

		// 描画
		context->DrawIndexed(node.indexCount, 0, 0);
	}
}

/*
	@brief	メッシュノード数を取得
*/
size_t FbxModel::GetNodeCount()
{
	return m_meshNodes.size();
}

/*
	@brief	指定したIDのメッシュノードを取得
*/
MeshNode& FbxModel::GetMeshNode(const int nodeId)
{
	return m_meshNodes[nodeId];
}

/*
	@brief	指定したメッシュノードからマテリアルデータを取得
*/
MaterialData& FbxModel::GetNodeMaterial(const int nodeId)
{
	return m_meshNodes[nodeId].materialData;
}

/*
	@brief	指定したメッシュノードから行列を取得
*/
D3DMATRIX FbxModel::GetNodeMatrix(const int nodeId, D3DMATRIX* mat4x4)
{
	return m_meshNodes[nodeId].mat4x4;
}

/*
	@brief	頂点データの構築
*/
bool FbxModel::VertexConstruction(ID3D11Device* device, FbxMeshNode &fbxNode, MeshNode& meshNode)
{
	meshNode.vertexCount = static_cast<DWORD>(fbxNode.positions.size());

	if (!device || meshNode.vertexCount == 0)
	{
		return false;
	}

	// 頂点データをためこむベクタ
	// 頂点のレイアウトは、座標・法線・UV座標１こで固定
	// このへんはゲームの設定やキャラクタの重要度によって変える必要があるね
	std::vector<VertexConstantData> vertices(meshNode.vertexCount);

	// 頂点分ループしてデータをコピーします
	for (size_t i = 0; i < meshNode.vertexCount; i++)
	{
		auto v = fbxNode.positions[i];
		vertices[i].position = D3DXVECTOR3(
			static_cast<float>(v.mData[0]),
			static_cast<float>(v.mData[1]),
			static_cast<float>(v.mData[2])
		);

		v = fbxNode.normals[i];
		vertices[i].normal = D3DXVECTOR3(
			static_cast<float>(v.mData[0]),
			static_cast<float>(v.mData[1]),
			static_cast<float>(v.mData[2])
		);

		if (fbxNode.texCoords.size() > 0)
		{
			auto t = fbxNode.texCoords[i];
			vertices[i].textureCoordinate = D3DXVECTOR2(
				static_cast<float>(abs(1.0f - t.mData[0])),
				static_cast<float>(abs(1.0f - t.mData[1]))
			);
		}
		else
		{
			vertices[i].textureCoordinate = D3DXVECTOR2(0, 0);
		}
	}

	// バーテックスバッファの生成
	CreateVertexBuffer(device,
		&meshNode.vb,
		vertices.data(),
		sizeof(VertexConstantData),
		meshNode.vertexCount);

	return true;
}

/*
	@brief	マテリアルの構築
*/
bool FbxModel::MaterialConstruction(ID3D11Device*	device, FbxMeshNode &fbxNode, MeshNode& meshNode)
{
	if (!device || fbxNode.materials.size() == 0)
	{
		return false;
	}


	//マテリアルの数だけインデックスバッファーを作成
	FbxMaterialNode fbxMaterial = fbxNode.materials[0];
	meshNode.materialData.ambient = D3DXVECTOR4(fbxMaterial.ambient.r,
		fbxMaterial.ambient.b,
		fbxMaterial.ambient.b,
		fbxMaterial.ambient.a);

	meshNode.materialData.diffuse = D3DXVECTOR4(fbxMaterial.diffuse.r,
		fbxMaterial.diffuse.b,
		fbxMaterial.diffuse.b,
		fbxMaterial.diffuse.a);

	meshNode.materialData.emissive = D3DXVECTOR4(fbxMaterial.emissive.r,
		fbxMaterial.emissive.b,
		fbxMaterial.emissive.b,
		fbxMaterial.emissive.a);

	meshNode.materialData.specular = D3DXVECTOR4(fbxMaterial.specular.r,
		fbxMaterial.specular.b,
		fbxMaterial.specular.b,
		fbxMaterial.specular.a);

	meshNode.materialData.specularPower = fbxMaterial.shininess;
	meshNode.materialData.transparency = fbxMaterial.transparency;

	// テクスチャロード
	if (fbxMaterial.diffuse.textureSets.size() > 0)
	{
		// 今回は１テクスチャ＆ディフューズテクスチャのみ対応
		auto it = std::begin(fbxMaterial.diffuse.textureSets);
		if (it->second.size())
		{
			//テクスチャパスを通す
			std::string path = "Assets\\" + it->second[0];

//#if 0	DirectXTK→DirectX11に変換したため不要
//			wchar_t  str[512];
//			size_t len = 0;
//			// charからwchar_tに変換してます
//			mbstowcs_s(&len, str, path.size() + 1, path.c_str(), _TRUNCATE);
//
//			// 決め打ちでDDSテクスチャとしてロード
//			//DirectX::CreateDDSTextureFromFile(device, str, nullptr, &meshNode.materialData.srv);
//			//png読み込み
//			//CreateWICTextureFromFile(device, str, nullptr, &meshNode.materialData.srv);
//#endif	
			D3DX11CreateShaderResourceViewFromFileA(device, path.c_str(), NULL, NULL, &meshNode.materialData.srv, NULL);
		}
	}

	HRESULT hr;

	// テクスチャサンプラ
	D3D11_SAMPLER_DESC desc{};
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = device->CreateSamplerState(&desc, &meshNode.materialData.sampler);
	if (FAILED(hr))
	{
		return false;
	}

	// マテリアルデータをシェーダに渡すための定数バッファを生成
	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(MaterialConstantData);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	hr = device->CreateBuffer(&bd, nullptr, &meshNode.materialData.materialCB);
	if (FAILED(hr))
	{
		return false;
	}

	// 定数バッファにデータをコピー
	// ただし今回はDirectXTKのシェーダを使っているので、こいつは未使用
	meshNode.materialData.constantData.ambient = meshNode.materialData.ambient;
	meshNode.materialData.constantData.diffuse = meshNode.materialData.diffuse;
	meshNode.materialData.constantData.emissive = meshNode.materialData.emissive;
	meshNode.materialData.constantData.specular = meshNode.materialData.specular;

	return true;
}

/*
	@brief	バーテックスバッファの生成
*/
bool FbxModel::CreateVertexBuffer(ID3D11Device* device, ID3D11Buffer** buffer, void* vertices, uint32_t stride, uint32_t vertexCount)
{
	if (!device || stride == 0 || vertexCount == 0)
	{
		return false;
	}

	// 生成処理はお決まりの流れです
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = stride * vertexCount;    // 1頂点のサイズ×頂点でバッファサイズを決めてる
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA subresource{};
	subresource.pSysMem = vertices;

	auto hr = device->CreateBuffer(&bd, &subresource, buffer);
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

/*
	@brief	インデックスバッファの生成
*/
bool FbxModel::CreateIndexBuffer(ID3D11Device* device, ID3D11Buffer** buffer, void* indices, uint32_t indexCount)
{
	if (!device || indexCount == 0)
	{
		return false;
	}

	// 32ビットにしか対応してないので決め打ち
	auto stride = sizeof(uint32_t);

	// 生成処理はお決まりの流れです
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = static_cast<uint32_t>(stride * indexCount);  // ビットサイズ×インデックス数でバッファサイズを決めてる
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA subresource{};
	subresource.pSysMem = indices;

	auto hr = device->CreateBuffer(&bd, &subresource, buffer);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}