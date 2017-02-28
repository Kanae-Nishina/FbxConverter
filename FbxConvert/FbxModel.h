/*
	@file			FbxModel.h
	@brief		FBXモデル描画及び周辺データの宣言
	@date		2017/02/27
	@author	仁科香苗
*/
#pragma once
//インクルードファイル
#include <d3dx9.h>
#include <d3d11.h>
#include <d3dx10.h>
#include <d3dx11.h>
#include "FbxLoader.h" 

//必要なライブラリファイルのロード
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx10.lib")
#pragma comment(lib,"d3dx11.lib")

//シェーダに送るマテリアルの定数データ
struct MaterialConstantData
{
	D3DXVECTOR4 ambient;
	D3DXVECTOR4 diffuse;
	D3DXVECTOR4 emissive;
	D3DXVECTOR4 specular;
};

//マテリアルデータ(テクスチャも含む)
struct MaterialData
{
	D3DXVECTOR4		ambient;
	D3DXVECTOR4		diffuse;
	D3DXVECTOR4		emissive;
	D3DXVECTOR4		specular;
	float						specularPower;
	float						transparency;

	MaterialConstantData				constantData;	//定数データ
	ID3D11ShaderResourceView* srv;					//テクスチャ
	ID3D11SamplerState*			sampler;			//テクスチャサンプラ
	ID3D11Buffer*						materialCB;		//定数バッファ

	MaterialData();
	void Release();

};

//シェーダに送る頂点定数データ
struct VertexConstantData
{
	D3DXVECTOR3 position;					//座標
	D3DXVECTOR3 normal;					//法線
	D3DXVECTOR2 textureCoordinate;	//UV座標
};

//メッシュデータ
struct MeshNode
{
	//インデックスで使うビット数
	enum IndexBit
	{
		eNone,
		e16Bit,
		e32Bit,
	};
	IndexBit						indexBit;

	ID3D11Buffer*			vb;					//バーテックスバッファ
	ID3D11Buffer*			ib;						//インデックスバッファ
	ID3D11InputLayout*	inputLayout;		//インプットレイアウト
	DWORD						vertexCount;		//頂点数
	DWORD						indexCount;		//インデックス数
	MaterialData				materialData;		//マテリアルデータ
	D3DXMATRIX				mat4x4;			//モデル行列

	MeshNode();
	void Release();
	void SetIndexBit(const size_t indexCount);
};

class FbxModel
{
private:
	std::vector<MeshNode>			m_meshNodes;	//メッシュノード配列
	std::vector<ID3D11Buffer*>	m_IndexBuffer;
	// 頂点データの構築
	bool VertexConstruction(ID3D11Device* device, FbxMeshNode &fbxNode, MeshNode& meshNode);
	// マテリアルの構築
	bool MaterialConstruction(ID3D11Device*	device, FbxMeshNode &fbxNode, MeshNode& meshNode);
	// バーテックスバッファの生成
	bool CreateVertexBuffer(ID3D11Device* device, ID3D11Buffer** buffer, void* vertices, uint32_t stride, uint32_t vertexCount);
	// インデックスバッファの生成
	bool CreateIndexBuffer(ID3D11Device* device, ID3D11Buffer** buffer, void* indices, uint32_t indexCount);
public:
	FbxModel();
	~FbxModel();

	// 読み込んだFBXから描画可能なデータを構築する
	bool Setup(FbxLoader* loader, ID3D11Device* device, ID3D11DeviceContext* context);
	// 解放処理
	void Release();
	// 描画に必要なInputLayoutを生成する
	bool CreateInputLayout(ID3D11Device* device,const void* shaderBytecodeWithInputSignature,
		size_t bytecodeLength,const D3D11_INPUT_ELEMENT_DESC* desc,unsigned int numElements);
	// 全ノードを同じ設定で描画する
	void RenderAll(ID3D11DeviceContext* context);
	// ノードを指定して描画
	void RenderNode(ID3D11DeviceContext* context, const int nodeId);
	// メッシュノード数を取得
	size_t GetNodeCount();
	// 指定したIDのメッシュノードを取得
	MeshNode& GetMeshNode(const int nodeId);
	// 指定したメッシュノードからマテリアルデータを取得
	MaterialData& GetNodeMaterial(const int nodeId);
	// 指定したメッシュノードから行列を取得
	D3DMATRIX GetNodeMatrix(const int nodeId, D3DMATRIX* mat4x4);
};

