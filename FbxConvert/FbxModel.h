/*
	@file			FbxModel.h
	@brief		FBX���f���`��y�ю��Ӄf�[�^�̐錾
	@date		2017/02/27
	@author	�m�ȍ��c
*/
#pragma once
//�C���N���[�h�t�@�C��
#include <d3dx9.h>
#include <d3d11.h>
#include <d3dx10.h>
#include <d3dx11.h>
#include "FbxLoader.h" 

//�K�v�ȃ��C�u�����t�@�C���̃��[�h
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx10.lib")
#pragma comment(lib,"d3dx11.lib")

//�V�F�[�_�ɑ���}�e���A���̒萔�f�[�^
struct MaterialConstantData
{
	D3DXVECTOR4 ambient;
	D3DXVECTOR4 diffuse;
	D3DXVECTOR4 emissive;
	D3DXVECTOR4 specular;
};

//�}�e���A���f�[�^(�e�N�X�`�����܂�)
struct MaterialData
{
	D3DXVECTOR4		ambient;
	D3DXVECTOR4		diffuse;
	D3DXVECTOR4		emissive;
	D3DXVECTOR4		specular;
	float						specularPower;
	float						transparency;

	MaterialConstantData				constantData;	//�萔�f�[�^
	ID3D11ShaderResourceView* srv;					//�e�N�X�`��
	ID3D11SamplerState*			sampler;			//�e�N�X�`���T���v��
	ID3D11Buffer*						materialCB;		//�萔�o�b�t�@

	MaterialData();
	void Release();

};

//�V�F�[�_�ɑ��钸�_�萔�f�[�^
struct VertexConstantData
{
	D3DXVECTOR3 position;					//���W
	D3DXVECTOR3 normal;					//�@��
	D3DXVECTOR2 textureCoordinate;	//UV���W
};

//���b�V���f�[�^
struct MeshNode
{
	//�C���f�b�N�X�Ŏg���r�b�g��
	enum IndexBit
	{
		eNone,
		e16Bit,
		e32Bit,
	};
	IndexBit						indexBit;

	ID3D11Buffer*			vb;					//�o�[�e�b�N�X�o�b�t�@
	ID3D11Buffer*			ib;						//�C���f�b�N�X�o�b�t�@
	ID3D11InputLayout*	inputLayout;		//�C���v�b�g���C�A�E�g
	DWORD						vertexCount;		//���_��
	DWORD						indexCount;		//�C���f�b�N�X��
	MaterialData				materialData;		//�}�e���A���f�[�^
	D3DXMATRIX				mat4x4;			//���f���s��

	MeshNode();
	void Release();
	void SetIndexBit(const size_t indexCount);
};

class FbxModel
{
private:
	std::vector<MeshNode>			m_meshNodes;	//���b�V���m�[�h�z��
	std::vector<ID3D11Buffer*>	m_IndexBuffer;
	// ���_�f�[�^�̍\�z
	bool VertexConstruction(ID3D11Device* device, FbxMeshNode &fbxNode, MeshNode& meshNode);
	// �}�e���A���̍\�z
	bool MaterialConstruction(ID3D11Device*	device, FbxMeshNode &fbxNode, MeshNode& meshNode);
	// �o�[�e�b�N�X�o�b�t�@�̐���
	bool CreateVertexBuffer(ID3D11Device* device, ID3D11Buffer** buffer, void* vertices, uint32_t stride, uint32_t vertexCount);
	// �C���f�b�N�X�o�b�t�@�̐���
	bool CreateIndexBuffer(ID3D11Device* device, ID3D11Buffer** buffer, void* indices, uint32_t indexCount);
public:
	FbxModel();
	~FbxModel();

	// �ǂݍ���FBX����`��\�ȃf�[�^���\�z����
	bool Setup(FbxLoader* loader, ID3D11Device* device, ID3D11DeviceContext* context);
	// �������
	void Release();
	// �`��ɕK�v��InputLayout�𐶐�����
	bool CreateInputLayout(ID3D11Device* device,const void* shaderBytecodeWithInputSignature,
		size_t bytecodeLength,const D3D11_INPUT_ELEMENT_DESC* desc,unsigned int numElements);
	// �S�m�[�h�𓯂��ݒ�ŕ`�悷��
	void RenderAll(ID3D11DeviceContext* context);
	// �m�[�h���w�肵�ĕ`��
	void RenderNode(ID3D11DeviceContext* context, const int nodeId);
	// ���b�V���m�[�h�����擾
	size_t GetNodeCount();
	// �w�肵��ID�̃��b�V���m�[�h���擾
	MeshNode& GetMeshNode(const int nodeId);
	// �w�肵�����b�V���m�[�h����}�e���A���f�[�^���擾
	MaterialData& GetNodeMaterial(const int nodeId);
	// �w�肵�����b�V���m�[�h����s����擾
	D3DMATRIX GetNodeMatrix(const int nodeId, D3DMATRIX* mat4x4);
};

