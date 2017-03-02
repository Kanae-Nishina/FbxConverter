/*
	@file			FbxLoader.h
	@brief		FBXファイルのローダ
	@date		2017/02/25
	@author	仁科香苗
	@note		参照(https://github.com/shaderjp/FBXLoader2015forDX11)
*/
#pragma once
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <windows.h>
#include <fbxsdk.h>
using namespace fbxsdk;
// UVSet名, 頂点内のUVセット順序
typedef std::tr1::unordered_map<std::string, int> UVsetID;
// UVSet名, テクスチャパス名
typedef std::tr1::unordered_map<std::string, std::vector<std::string>> TextureSet;

//マテリアル構成
struct FBX_MATERIAL_ELEMENT
{
	enum MaterialElementType
	{
		ELEMENT_NONE = 0,
		ELEMENT_COLOR,
		ELEMENT_TEXTURE,
		ELEMENT_BOTH,
		ELEMENT_MAX,
	};
	MaterialElementType type;
	float r, g, b, a;
	TextureSet textureSetArray;

	FBX_MATERIAL_ELEMENT();
	~FBX_MATERIAL_ELEMENT();
	void Release();
};

//マテリアルのノード
struct FBX_MATERIAL_NODE
{
	enum MaterialType
	{
		MATERIAL_LAMBERT = 0,
		MATERIAL_PHONG,
	};
	MaterialType type;
	FBX_MATERIAL_ELEMENT ambient;
	FBX_MATERIAL_ELEMENT diffuse;
	FBX_MATERIAL_ELEMENT emmisive;
	FBX_MATERIAL_ELEMENT specular;
	float shininess;
	float transparencyFactor;
};

//メッシュ構成要素
struct MESH_ELEMENTS
{
	unsigned int numPosition;	//頂点座標セット数
	unsigned int numNormal;	//法線情報
	unsigned int numUVSet;		//UVセット数
};

//メッシュのノード
struct FBX_MESH_NODE
{
	std::string		name;				//ノード名
	std::string		parentName;		//親ノード名(親がいないなら"null"という名称が入る.rootノードの対応)

	MESH_ELEMENTS								elements;				//メッシュが保持するデータ構造
	std::vector<FBX_MATERIAL_NODE>	materialArray;		//マテリアル
	UVsetID												uvsetID;

	std::vector<unsigned int>			indexArray;			//インデックス配列
	std::vector<FbxVector4>			positionArray;		//ポジション配列
	std::vector<FbxVector4>			normalArray;			//法線配列
	std::vector<FbxVector2>			texcoordArray;		//テクスチャ座標配列

	float	mat4x4[16];	// Matrix

	~FBX_MESH_NODE();
	void Release();
};

//ロードクラス
class FbxLoader
{
public:
	FbxLoader();
	~FbxLoader();

	void Release();

	// 読み込み
	HRESULT LoadFBX(const char* filename);
	FbxNode&	GetRootNode();

	size_t GetNodesCount() { return m_meshNodeArray.size(); };		// ノード数の取得

	FBX_MESH_NODE&	GetNode(const unsigned int id);
protected:
	//FBX SDK
	FbxManager*			m_sdkManager;
	FbxScene*				m_scene;
	FbxImporter*			m_importer;
	FbxAnimLayer*		m_currentAnimLayer;

	std::vector<FBX_MESH_NODE>		m_meshNodeArray;

	//FBXのパース
	void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
	void TriangulateRecursive(FbxNode* pNode);
	void SetupNode(FbxNode* pNode, std::string parentName);
	void Setup();
	void CopyVertexData(FbxMesh*	pMesh, FBX_MESH_NODE* meshNode);
	void CopyMatrialData(FbxSurfaceMaterial* mat, FBX_MATERIAL_NODE* destMat);
	void ComputeNodeMatrix(FbxNode* pNode, FBX_MESH_NODE* meshNode);
	void SetFbxColor(FBX_MATERIAL_ELEMENT& destColor, const FbxDouble3 srcColor);
	FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial * pMaterial,const char * pPropertyName,
		const char * pFactorPropertyName,FBX_MATERIAL_ELEMENT*			pElement);
	static void FBXMatrixToFloat16(FbxMatrix* src, float dest[16]);
};