/*
	@file			FbxModel.h
	@brief		FBX���f���`��y�ю��Ӄf�[�^�̐錾
	@date		2017/02/27
	@author	�m�ȍ��c
*/
#include "FbxModel.h"

//===========================
//	MaterialData�\����
//===========================
/*
	@brief	MaterialData�R���X�g���N�^
*/
MaterialData::MaterialData()
	:srv(nullptr)
	,sampler(nullptr)
	,materialCB(nullptr)
{
}

/*
	@brief ���
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
//	MeshNode�\����
//===========================
/*
	@brief	MeshNode�R���X�g���N�^
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
	@brief	���
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
	@brief	�C���f�b�N�X�̃r�b�g����ݒ肷��
*/
void MeshNode::SetIndexBit(const size_t indexCount)
{
	indexBit = IndexBit::eNone;
	//32�r�b�g�����Ή�
	if (indexCount > 0)
	{
		indexBit = IndexBit::e32Bit;
	}
}

//===========================
//	FbxModel�N���X
//===========================
/*
	@brief	�R���X�g���N�^
*/
FbxModel::FbxModel()
{
}

/*
	@brief	�f�X�g���N�^
*/
FbxModel::~FbxModel()
{
}

/*
	@brief	�ǂݍ���FBX����`��\�ȃf�[�^���\�z����
*/
bool FbxModel::Setup(FbxLoader* loader, ID3D11Device* device, ID3D11DeviceContext* context)
{
	std::size_t nodeCount = loader->GetNodeCount();
	if (nodeCount == 0)
	{
		return false;
	}

	// fbx�m�[�h�����[�v���ă��f�����\�z
	for (auto i = 0; i < nodeCount; i++)
	{
		MeshNode meshNode;
		auto& fbxNode = loader->GetNode(i);

		// ���_�f�[�^�̍\�z
		VertexConstruction(device, fbxNode, meshNode);

		// �C���f�b�N�X�̍\�z
		meshNode.indexCount = static_cast<DWORD>(fbxNode.indices.size());
		meshNode.SetIndexBit(meshNode.indexCount);
		if (fbxNode.indices.size() > 0)
		{
			CreateIndexBuffer(device,
				&meshNode.ib,
				&fbxNode.indices[0],
				static_cast<uint32_t>(fbxNode.indices.size()));
		}

		// �}�e���A���\�z
		MaterialConstruction(device, fbxNode, meshNode);

		// ���f���s���ۑ�
		D3DMATRIX mat =
		{
			fbxNode.mat4x4[0],fbxNode.mat4x4[1],fbxNode.mat4x4[2],fbxNode.mat4x4[3],
			fbxNode.mat4x4[4],fbxNode.mat4x4[5],fbxNode.mat4x4[6],fbxNode.mat4x4[7],
			fbxNode.mat4x4[8],fbxNode.mat4x4[9],fbxNode.mat4x4[10],fbxNode.mat4x4[11],
			fbxNode.mat4x4[12],fbxNode.mat4x4[13],fbxNode.mat4x4[14],fbxNode.mat4x4[15],
		};
		//meshNode.mat4x4 = D3DMATRIX(fbxNode.mat4x4);
		meshNode.mat4x4 = D3DMATRIX(mat);

		// �o���オ�������f�����Avector�z��Ƀv�b�V��
		m_meshNodes.push_back(meshNode);
	}

	return true;
}

/*
	@brief	�������
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
	@brief	�`��ɕK�v��InputLayout�𐶐�����
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
	@brief	�S�m�[�h�𓯂��ݒ�ŕ`�悷��
*/
void FbxModel::RenderAll(ID3D11DeviceContext* context)
{
	if (m_meshNodes.size() == 0)
	{
		return;
	}

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// �m�[�h����C�ɕ`��
	for (auto& node : m_meshNodes)
	{
		if (node.vertexCount == 0)
		{
			continue;
		}

		// �V�F�[�_�ɗ������_�f�[�^�̐ݒ�
		UINT stride = sizeof(VertexConstantData);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &node.vb, &stride, &offset);
		context->IASetInputLayout(node.inputLayout);

		// ����̓C���f�b�N�X���g�p�̃��f���͕`�悵�Ȃ���
		if (node.indexBit != MeshNode::eNone)
		{
			// �C���f�b�N�X�o�b�t�@��ݒ�
			DXGI_FORMAT indexBit = DXGI_FORMAT_R16_UINT;
			if (node.indexBit == MeshNode::e32Bit)
			{
				indexBit = DXGI_FORMAT_R32_UINT;
			}
			context->IASetIndexBuffer(node.ib, indexBit, 0);

			// �`��
			context->DrawIndexed(node.indexCount, 0, 0);
		}
	}
}

/*
	@brief	�m�[�h���w�肵�ĕ`��
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

	// �V�F�[�_�ɗ������_�f�[�^�̐ݒ�
	UINT stride = sizeof(VertexConstantData);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &node.vb, &stride, &offset);
	context->IASetInputLayout(node.inputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ����̓C���f�b�N�X���g�p�̃��f���͕`�悵�Ȃ���
	if (node.indexBit != MeshNode::eNone)
	{
		// �C���f�b�N�X�o�b�t�@��ݒ�
		DXGI_FORMAT indexbit = DXGI_FORMAT_R16_UINT;
		if (node.indexBit == MeshNode::e32Bit)
		{
			indexbit = DXGI_FORMAT_R32_UINT;
		}
		context->IASetIndexBuffer(node.ib, indexbit, 0);

		// �`��
		context->DrawIndexed(node.indexCount, 0, 0);
	}
}

/*
	@brief	���b�V���m�[�h�����擾
*/
size_t FbxModel::GetNodeCount()
{
	return m_meshNodes.size();
}

/*
	@brief	�w�肵��ID�̃��b�V���m�[�h���擾
*/
MeshNode& FbxModel::GetMeshNode(const int nodeId)
{
	return m_meshNodes[nodeId];
}

/*
	@brief	�w�肵�����b�V���m�[�h����}�e���A���f�[�^���擾
*/
MaterialData& FbxModel::GetNodeMaterial(const int nodeId)
{
	return m_meshNodes[nodeId].materialData;
}

/*
	@brief	�w�肵�����b�V���m�[�h����s����擾
*/
D3DMATRIX FbxModel::GetNodeMatrix(const int nodeId, D3DMATRIX* mat4x4)
{
	return m_meshNodes[nodeId].mat4x4;
}

/*
	@brief	���_�f�[�^�̍\�z
*/
bool FbxModel::VertexConstruction(ID3D11Device* device, FbxMeshNode &fbxNode, MeshNode& meshNode)
{
	meshNode.vertexCount = static_cast<DWORD>(fbxNode.positions.size());

	if (!device || meshNode.vertexCount == 0)
	{
		return false;
	}

	// ���_�f�[�^�����߂��ރx�N�^
	// ���_�̃��C�A�E�g�́A���W�E�@���EUV���W�P���ŌŒ�
	// ���̂ւ�̓Q�[���̐ݒ��L�����N�^�̏d�v�x�ɂ���ĕς���K�v�������
	std::vector<VertexConstantData> vertices(meshNode.vertexCount);

	// ���_�����[�v���ăf�[�^���R�s�[���܂�
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

	// �o�[�e�b�N�X�o�b�t�@�̐���
	CreateVertexBuffer(device,
		&meshNode.vb,
		vertices.data(),
		sizeof(VertexConstantData),
		meshNode.vertexCount);

	return true;
}

/*
	@brief	�}�e���A���̍\�z
*/
bool FbxModel::MaterialConstruction(ID3D11Device*	device, FbxMeshNode &fbxNode, MeshNode& meshNode)
{
	if (!device || fbxNode.materials.size() == 0)
	{
		return false;
	}


	//�}�e���A���̐������C���f�b�N�X�o�b�t�@�[���쐬
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

	// �e�N�X�`�����[�h
	if (fbxMaterial.diffuse.textureSets.size() > 0)
	{
		// ����͂P�e�N�X�`�����f�B�t���[�Y�e�N�X�`���̂ݑΉ�
		auto it = std::begin(fbxMaterial.diffuse.textureSets);
		if (it->second.size())
		{
			//�e�N�X�`���p�X��ʂ�
			std::string path = "Assets\\" + it->second[0];

//#if 0	DirectXTK��DirectX11�ɕϊ��������ߕs�v
//			wchar_t  str[512];
//			size_t len = 0;
//			// char����wchar_t�ɕϊ����Ă܂�
//			mbstowcs_s(&len, str, path.size() + 1, path.c_str(), _TRUNCATE);
//
//			// ���ߑł���DDS�e�N�X�`���Ƃ��ă��[�h
//			//DirectX::CreateDDSTextureFromFile(device, str, nullptr, &meshNode.materialData.srv);
//			//png�ǂݍ���
//			//CreateWICTextureFromFile(device, str, nullptr, &meshNode.materialData.srv);
//#endif	
			D3DX11CreateShaderResourceViewFromFileA(device, path.c_str(), NULL, NULL, &meshNode.materialData.srv, NULL);
		}
	}

	HRESULT hr;

	// �e�N�X�`���T���v��
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

	// �}�e���A���f�[�^���V�F�[�_�ɓn�����߂̒萔�o�b�t�@�𐶐�
	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(MaterialConstantData);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	hr = device->CreateBuffer(&bd, nullptr, &meshNode.materialData.materialCB);
	if (FAILED(hr))
	{
		return false;
	}

	// �萔�o�b�t�@�Ƀf�[�^���R�s�[
	// �����������DirectXTK�̃V�F�[�_���g���Ă���̂ŁA�����͖��g�p
	meshNode.materialData.constantData.ambient = meshNode.materialData.ambient;
	meshNode.materialData.constantData.diffuse = meshNode.materialData.diffuse;
	meshNode.materialData.constantData.emissive = meshNode.materialData.emissive;
	meshNode.materialData.constantData.specular = meshNode.materialData.specular;

	return true;
}

/*
	@brief	�o�[�e�b�N�X�o�b�t�@�̐���
*/
bool FbxModel::CreateVertexBuffer(ID3D11Device* device, ID3D11Buffer** buffer, void* vertices, uint32_t stride, uint32_t vertexCount)
{
	if (!device || stride == 0 || vertexCount == 0)
	{
		return false;
	}

	// ���������͂����܂�̗���ł�
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = stride * vertexCount;    // 1���_�̃T�C�Y�~���_�Ńo�b�t�@�T�C�Y�����߂Ă�
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
	@brief	�C���f�b�N�X�o�b�t�@�̐���
*/
bool FbxModel::CreateIndexBuffer(ID3D11Device* device, ID3D11Buffer** buffer, void* indices, uint32_t indexCount)
{
	if (!device || indexCount == 0)
	{
		return false;
	}

	// 32�r�b�g�ɂ����Ή����ĂȂ��̂Ō��ߑł�
	auto stride = sizeof(uint32_t);

	// ���������͂����܂�̗���ł�
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = static_cast<uint32_t>(stride * indexCount);  // �r�b�g�T�C�Y�~�C���f�b�N�X���Ńo�b�t�@�T�C�Y�����߂Ă�
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