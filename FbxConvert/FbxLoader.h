/*
	@file			FbxLoader.h
	@brief		FBX�t�@�C���̃��[�_
	@date		2017/02/25
	@author	�m�ȍ��c
	@note		�Q��(https://github.com/shaderjp/FBXLoader2015forDX11)
*/
#pragma once
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <windows.h>
#include <fbxsdk.h>
using namespace fbxsdk;
// UVSet��, ���_����UV�Z�b�g����
typedef std::tr1::unordered_map<std::string, int> UVsetID;
// UVSet��, �e�N�X�`���p�X��
typedef std::tr1::unordered_map<std::string, std::vector<std::string>> TextureSet;

//�}�e���A���\��
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

//�}�e���A���̃m�[�h
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

//���b�V���\���v�f
struct MESH_ELEMENTS
{
	unsigned int numPosition;	//���_���W�Z�b�g��
	unsigned int numNormal;	//�@�����
	unsigned int numUVSet;		//UV�Z�b�g��
};

//���b�V���̃m�[�h
struct FBX_MESH_NODE
{
	std::string		name;				//�m�[�h��
	std::string		parentName;		//�e�m�[�h��(�e�����Ȃ��Ȃ�"null"�Ƃ������̂�����.root�m�[�h�̑Ή�)

	MESH_ELEMENTS								elements;				//���b�V�����ێ�����f�[�^�\��
	std::vector<FBX_MATERIAL_NODE>	materialArray;		//�}�e���A��
	UVsetID												uvsetID;

	std::vector<unsigned int>			indexArray;			//�C���f�b�N�X�z��
	std::vector<FbxVector4>			positionArray;		//�|�W�V�����z��
	std::vector<FbxVector4>			normalArray;			//�@���z��
	std::vector<FbxVector2>			texcoordArray;		//�e�N�X�`�����W�z��

	float	mat4x4[16];	// Matrix

	~FBX_MESH_NODE();
	void Release();
};

//���[�h�N���X
class FbxLoader
{
public:
	FbxLoader();
	~FbxLoader();

	void Release();

	// �ǂݍ���
	HRESULT LoadFBX(const char* filename);
	FbxNode&	GetRootNode();

	size_t GetNodesCount() { return m_meshNodeArray.size(); };		// �m�[�h���̎擾

	FBX_MESH_NODE&	GetNode(const unsigned int id);
protected:
	//FBX SDK
	FbxManager*			m_sdkManager;
	FbxScene*				m_scene;
	FbxImporter*			m_importer;
	FbxAnimLayer*		m_currentAnimLayer;

	std::vector<FBX_MESH_NODE>		m_meshNodeArray;

	//FBX�̃p�[�X
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