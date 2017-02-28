/*
	@file			FbxLoader.h
	@brief		FBX�t�@�C���̃��[�_�[
	@date		2017/02/25
	@author	�m�ȍ��c
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

//UV�Z�b�g���̖��O�ƃe�N�X�`���̃t�@�C������R�Â� 
typedef std::unordered_map <std::string, std::vector<std::string>> TextureSet;

 //UV�Z�b�g�̖��O�ɔԍ��t��
typedef std::unordered_map <std::string, int> UVSetID;

//�ʂ̃}�e���A���v�f
struct FbxMaterialElement
{
	//���
	enum MaterialElementType
	{
		eNone=0,
		eColor,		//�J���[�̂�
		eTexture,	//�e�N�X�`���̂�
		eBoth,		//�J���[�ƃe�N�X�`������
		eMax,
	};
	MaterialElementType type;

	//�F
	float r;
	float g;
	float b;
	float a;

	//�e�N�X�`��
	TextureSet textureSets;
	
	~FbxMaterialElement();
	void Release();
};

//�}�e���A��
struct FbxMaterialNode
{
	//�T�[�t�F�C�X�̎��
	enum MaterialType
	{
		eLambert,		//�����o�[�g
		ePhong,			//�t�H��
	};
	MaterialType type;

	//�}�e���A���f�[�^
	FbxMaterialElement diffuse;		//�g�U���ˌ�
	FbxMaterialElement specular;	//���ʔ��ˌ�
	FbxMaterialElement ambient;	//����
	FbxMaterialElement emissive;	//���Ȕ���

	float shininess;		// �X�y�L�����̋���
	float transparency; // ���ߓx 
};

//���b�V���̗v�f��
struct MeshElements
{
	uint32_t numPosition;
	uint32_t numNormal;
	uint32_t numUVSet;

	MeshElements();
};

//���b�V��
struct FbxMeshNode
{
	std::string name;			//�m�[�h��
	std::string parentName;	//�e�m�[�h��

	MeshElements elements;

	//�}�e���A��
	std::vector<FbxMaterialNode> materials;
	//UV�̃Z�b�g����
	UVSetID uvSetID;

	//���_�f�[�^
	std::vector<uint32_t>		indices;			//�C���f�b�N�X
	std::vector<FbxVector4>	positions;		//���W
	std::vector<FbxVector4>	normals;		//�@��
	std::vector<FbxVector2>	texCoords;		//UV���W

	//���b�V���̃��[�J���}�g���N�X
	float mat4x4[16];

	~FbxMeshNode();
	void Release();
};

//FBX���[�_�[�N���X
//FBX���p�[�X�V�e�K�v�ȃf�[�^�𔲂��o���ĕێ�����
class FbxLoader
{
protected:
	//�C���|�[�g�ɕK�v�ȕϐ��Q
	FbxManager*							m_manager;
	FbxScene*								m_scene;
	FbxImporter*							m_importer;
	std::vector<FbxMeshNode>	m_meshNodes;	//���[�_�[�ɂ��\�z����郁�b�V���m�[�h

	//FBX�S�̂̃p�[�X
	void SetUp();	
	//�m�[�h���p�[�X
	void SetUpNode(FbxNode* node, std::string& parentName);
	//���_�f�[�^�̃R�s�[
	void CopyVertex(FbxMesh* mesh, FbxMeshNode* meshNode);
	//�}�e���A���̃R�s�[
	void CopyMaterial(FbxSurfaceMaterial* material, FbxMaterialNode* materialNode);
	//�m�[�h���̍s����v�Z
	void ComputeNodeMatrix(FbxNode* node, FbxMeshNode* meshNode);
	//FBX�p�J���[�f�[�^���}�e���A���ɃR�s�[
	void SetFbxColor(FbxMaterialElement* element, const FbxDouble3 color);
	//FBX�p�̍s��f�[�^��ʏ��float�z��ɃR�s�[
	static void FbxMatrixToFloat16(FbxMatrix* matrix, float array[16]);
	//�}�e���A���v�f�̎擾
	FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial* material, const char* propertyName, const char* factorPropertyName,FbxMaterialElement* element);

public:
	FbxLoader();
	~FbxLoader();

	bool LoadModel(const char* filename);		//���[�h
	void Release();											//�f�[�^�̉��
	FbxMeshNode& GetNode(const uint32_t id);	//���b�V���m�[�h�擾
	std::size_t GetNodeCount();						//���b�V���m�[�h���擾
};

