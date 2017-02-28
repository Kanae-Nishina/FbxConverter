/*
	@file			FbxLoader.h
	@brief		FBXファイルのローダー
	@date		2017/02/25
	@author	仁科香苗
*/
#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <cassert>
#include <fbxsdk.h>
#include <d3dx9.h>

#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

//UVセット名の名前とテクスチャのファイル名を紐づけ 
typedef std::unordered_map <std::string, std::vector<std::string>> TextureSet;

 //UVセットの名前に番号付け
typedef std::unordered_map <std::string, int> UVSetID;

//個別のマテリアル要素
struct FbxMaterialElement
{
	//種別
	enum MaterialElementType
	{
		eNone=0,
		eColor,		//カラーのみ
		eTexture,	//テクスチャのみ
		eBoth,		//カラーとテクスチャ両方
		eMax,
	};
	MaterialElementType type;

	//色
	float r;
	float g;
	float b;
	float a;

	//テクスチャ
	TextureSet textureSets;
	
	~FbxMaterialElement();
	void Release();
};

//マテリアル
struct FbxMaterialNode
{
	//サーフェイスの種類
	enum MaterialType
	{
		eLambert,		//ランバート
		ePhong,			//フォン
	};
	MaterialType type;

	//マテリアルデータ
	FbxMaterialElement diffuse;		//拡散反射光
	FbxMaterialElement specular;	//鏡面反射光
	FbxMaterialElement ambient;	//環境光
	FbxMaterialElement emissive;	//自己発光

	float shininess;		// スペキュラの強さ
	float transparency; // 透過度 
};

//メッシュの要素数
struct MeshElements
{
	uint32_t numPosition;
	uint32_t numNormal;
	uint32_t numUVSet;

	MeshElements();
};

//メッシュ
struct FbxMeshNode
{
	std::string name;			//ノード名
	std::string parentName;	//親ノード名

	MeshElements elements;

	//マテリアル
	std::vector<FbxMaterialNode> materials;
	//UVのセット順序
	UVSetID uvSetID;

	//頂点データ
	std::vector<uint32_t>		indices;			//インデックス
	std::vector<FbxVector4>	positions;		//座標
	std::vector<FbxVector4>	normals;		//法線
	std::vector<FbxVector2>	texCoords;		//UV座標

	//メッシュのローカルマトリクス
	float mat4x4[16];

	~FbxMeshNode();
	void Release();
};

//FBXローダークラス
//FBXをパースシテ必要なデータを抜き出して保持する
class FbxLoader
{
protected:
	//インポートに必要な変数群
	FbxManager*							m_manager;
	FbxScene*								m_scene;
	FbxImporter*							m_importer;
	std::vector<FbxMeshNode>	m_meshNodes;	//ローダーにより構築されるメッシュノード

	//FBX全体のパース
	void SetUp();	
	//ノードをパース
	void SetUpNode(FbxNode* node, std::string& parentName);
	//頂点データのコピー
	void CopyVertex(FbxMesh* mesh, FbxMeshNode* meshNode);
	//マテリアルのコピー
	void CopyMaterial(FbxSurfaceMaterial* material, FbxMaterialNode* materialNode);
	//ノード内の行列を計算
	void ComputeNodeMatrix(FbxNode* node, FbxMeshNode* meshNode);
	//FBX用カラーデータをマテリアルにコピー
	void SetFbxColor(FbxMaterialElement* element, const FbxDouble3 color);
	//FBX用の行列データを通常にfloat配列にコピー
	static void FbxMatrixToFloat16(FbxMatrix* matrix, float array[16]);
	//マテリアル要素の取得
	FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial* material, const char* propertyName, const char* factorPropertyName,FbxMaterialElement* element);

public:
	FbxLoader();
	~FbxLoader();

	bool LoadModel(const char* filename);		//ロード
	void Release();											//データの解放
	FbxMeshNode& GetNode(const uint32_t id);	//メッシュノード取得
	std::size_t GetNodeCount();						//メッシュノード数取得
};

