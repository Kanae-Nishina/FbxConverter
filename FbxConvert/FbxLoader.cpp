/*
	@file			FbxLoader.h
	@brief		FBX�t�@�C���̃��[�_�[
	@date		2017/02/25
	@author	�m�ȍ��c
*/
#include "FbxLoader.h"

//========================
//	FbxMaterialElement�\����
//========================
/*
	@brief	FbxMaterialElement�f�X�g���N�^
*/
FbxMaterialElement::~FbxMaterialElement()
{
	Release();
}

/*
	@brief	���
*/
void FbxMaterialElement::Release()
{
	for (auto it : textureSets)
	{
		it.second.clear();
	}
	textureSets.clear();
}

//========================
//	MeshElements�\����
//========================
/*
	@brief	MeshElements�R���X�g���N�^
*/
MeshElements::MeshElements()
	:numPosition(0)
	,numNormal(0)
	,numUVSet(0)
{
}

//========================
//	FbxMeshNode�\����
//========================
/*
	@brief	FbxMeshNode�f�X�g���N�^
*/
FbxMeshNode::~FbxMeshNode()
{
	Release();
}

/*
	@brief	���
*/
void FbxMeshNode::Release()
{
	uvSetID.clear();
	materials.clear();
	indices.clear();
	positions.clear();
	normals.clear();
	texCoords.clear();
}
//========================
//	FbxLoader�N���X
//========================
/*
	@brief	FbxLoader�R���X�g���N�^
*/
FbxLoader::FbxLoader()
	:m_manager(nullptr)
	,m_scene(nullptr)
	,m_importer(nullptr)
{
}

/*
	@brief	FbxLoader�f�X�g���N�^
*/
FbxLoader::~FbxLoader()
{
	Release();
}

/*
	@brief	���f���̃��[�h
*/
bool FbxLoader::LoadModel(const char* filename)
{
	//fbx�I�u�W�F�N�g�̐���
	m_manager = FbxManager::Create();
	auto ios = FbxIOSettings::Create(m_manager, IOSROOT);
	m_manager->SetIOSettings(ios);

	auto path = FbxGetApplicationDirectory();
	m_manager->LoadPluginsDirectory(path.Buffer());

	m_scene = FbxScene::Create(m_manager, "Scene");
	m_importer = FbxImporter::Create(m_manager, "");

	// �C���|�[�g����
	int fileFormat = -1;
	if (!m_manager->GetIOPluginRegistry()->DetectReaderFileFormat(filename, fileFormat))
	{
		fileFormat = m_manager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");
	}

	// �f�[�^�̃C���|�[�g
	if (!m_importer->Initialize(filename, fileFormat))
	{
		return false;
	}

	if (!m_importer->Import(m_scene))
	{
		return false;
	}

	// fbx�̎��n����DirectX�n�ɂ��Ă���
	if (m_scene->GetGlobalSettings().GetAxisSystem() != FbxAxisSystem::DirectX)
	{
		FbxAxisSystem::DirectX.ConvertScene(m_scene);
	}

	// �W�I���g���R���o�[�^�̍쐬
	FbxGeometryConverter geometryConverter(m_manager);
	// �|���S����S�ĎO�p�`��
	geometryConverter.Triangulate(m_scene, true);
	// �}�e���A�����ƂɃ��b�V���𕪊�
	//geometryConverter.SplitMeshesPerMaterial(scene_, true);

	// �p�[�X�J�n
	SetUp();


	return true;
}

/*
	@brief	�f�[�^�̉��
*/
void FbxLoader::Release()
{
	if (m_importer)
	{
		m_importer->Destroy();
		m_importer = nullptr;
	}

	if (m_scene)
	{
		m_scene->Destroy();
		m_scene = nullptr;
	}

	if (m_manager)
	{
		m_manager->Destroy();
		m_manager = nullptr;
	}
}

/*
	@brief	���b�V���m�[�h�̎擾
*/
FbxMeshNode& FbxLoader::GetNode(const uint32_t id)
{
	return m_meshNodes[id];
}

/*
	@brief	���b�V���m�[�h���̎擾
*/
std::size_t FbxLoader::GetNodeCount()
{
	return m_meshNodes.size();
}

/*
	@breif	FBX�S�̂̃p�[�X
*/
void FbxLoader::SetUp()
{
	if (m_scene->GetRootNode())
	{
		// �ŏ��̃m�[�h����null���Ė��O�ɂ��Ă���
		std::string name = "null";

		SetUpNode(m_scene->GetRootNode(), name);
	}
}

/*
	@brief	�m�[�h���p�[�X
*/
void FbxLoader::SetUpNode(FbxNode* node, std::string& parentName)
{
	if (m_scene->GetRootNode())
	{
		// �ŏ��̃m�[�h����null���Ė��O�ɂ��Ă���
		std::string name = "null";

		SetUpNode(m_scene->GetRootNode(), name);
	}
}

/*
	@brief	���_�f�[�^�̃R�s�[
*/
void FbxLoader::CopyVertex(FbxMesh* mesh, FbxMeshNode* meshNode)
{
	if (!mesh)
	{
		return;
	}

	// ���_�E�@���f�[�^�͂P�Z�b�g�̂ݑΉ�
	meshNode->elements.numPosition = 1;
	meshNode->elements.numNormal = 1;

	// ���_�C���f�b�N�X
	uint32_t polygonIndex = 0;

	// �|���S�������擾
	int polygonCount = mesh->GetPolygonCount();
	for (int i = 0; i < polygonCount; i++)
	{
		// �|���S���T�C�Y���擾
		int polygonSize = mesh->GetPolygonSize(i);
		for (int j = 0; j < polygonSize; j++)
		{
			// �|���S���̔ԍ��ƁA���_�̔ԍ����g���ăC���f�b�N�X���擾
			int index = mesh->GetPolygonVertex(i, j);
			meshNode->indices.push_back(polygonIndex);

			// �C���f�b�N�X���璸�_���W���擾
			FbxVector4 position = mesh->GetControlPointAt(index);

			// �|���S���̔ԍ��ƁA���_�̔ԍ����g���Ė@���f�[�^���擾
			FbxVector4 normal;
			mesh->GetPolygonVertexNormal(i, j, normal);

			// ���_�E�@���̃v�b�V��
			meshNode->positions.push_back(position);
			meshNode->normals.push_back(normal);

			// �C���f�b�N�X���C���N�������g
			++polygonIndex;
		}
	}

	// UV�f�[�^�̍\�z
	FbxStringList uvSetNames;
	mesh->GetUVSetNames(uvSetNames);

	// UV�͂Q�ȏ゠��Ƃ�������
	int uvSetCount = uvSetNames.GetCount();
	meshNode->elements.numUVSet = uvSetCount;

	bool unmapped = false;

	// UV�������[�v
	for (int uvIndex = 0; uvIndex < uvSetCount; uvIndex++)
	{
		// UV�Z�b�g�̔ԍ���ݒ�
		meshNode->uvSetID[uvSetNames.GetStringAt(uvIndex)] = uvIndex;
		for (int i = 0; i < polygonCount; i++)
		{
			int polygonSize = mesh->GetPolygonSize(i);
			for (int j = 0; j < polygonSize; j++)
			{
				FbxString name = uvSetNames.GetStringAt(uvIndex);

				FbxVector2 texCoord;
				// �|���S���̔ԍ��ƁA���_�̔ԍ��EUV�Z�b�g�����g����UV���W���擾
				mesh->GetPolygonVertexUV(i, j, name, texCoord, unmapped);
				meshNode->texCoords.push_back(texCoord);
			}
		}
	}
}

/*
	@brief	�}�e���A���̃R�s�[
*/
void FbxLoader::CopyMaterial(FbxSurfaceMaterial* material, FbxMaterialNode* materialNode)
{
	if (!material)
	{
		return;
	}

	// �}�e���A���̌v�Z���@�̐ݒ�
	if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		materialNode->type = FbxMaterialNode::eLambert;
	}
	else if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		materialNode->type = FbxMaterialNode::ePhong;
	}

	// �}�e���A���v�f���擾

	// �A���r�G���g�J���[
	const FbxDouble3 ambient = GetMaterialProperty(material,
		FbxSurfaceMaterial::sAmbient,
		FbxSurfaceMaterial::sAmbientFactor,
		&materialNode->ambient);
	SetFbxColor(&materialNode->ambient, ambient);

	// �f�B�t���[�Y�J���[
	const FbxDouble3 diffuse = GetMaterialProperty(material,
		FbxSurfaceMaterial::sDiffuse,
		FbxSurfaceMaterial::sDiffuseFactor,
		&materialNode->diffuse);
	SetFbxColor(&materialNode->diffuse, diffuse);

	// �G�~�b�V�u�J���[
	const FbxDouble3 emissive = GetMaterialProperty(material,
		FbxSurfaceMaterial::sEmissive,
		FbxSurfaceMaterial::sEmissiveFactor,
		&materialNode->emissive);
	SetFbxColor(&materialNode->emissive, emissive);

	// �X�y�L�����J���[
	const FbxDouble3 specular = GetMaterialProperty(material,
		FbxSurfaceMaterial::sSpecular,
		FbxSurfaceMaterial::sSpecularFactor,
		&materialNode->specular);
	SetFbxColor(&materialNode->specular, specular);


	// �P���̂悳
	FbxProperty shininessProperty = material->FindProperty(FbxSurfaceMaterial::sShininess);
	if (shininessProperty.IsValid())
	{
		double shininess = shininessProperty.Get<FbxDouble>();
		materialNode->shininess = static_cast<float>(shininess);
	}

	// �}�e���A�����̂̓��ߓx
	FbxProperty transparencyProperty = material->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
	if (transparencyProperty.IsValid())
	{
		double transparency = transparencyProperty.Get<FbxDouble>();
		materialNode->transparency = static_cast<float>(transparency);
	}
}

/*
	@brief	�m�[�h�̍s����v�Z
*/
void FbxLoader::ComputeNodeMatrix(FbxNode* node, FbxMeshNode* meshNode)
{
	if (!node || !meshNode)
	{
		return;
	}

	FbxAnimEvaluator* evaluator = m_scene->GetAnimationEvaluator();

	FbxMatrix globalMat;
	globalMat.SetIdentity();

	if (node != m_scene->GetRootNode())
	{
		globalMat = evaluator->GetNodeGlobalTransform(node);
	}
	FbxMatrixToFloat16(&globalMat, meshNode->mat4x4);
}

/*
	@brief	FBX�p�J���[�f�[�^���}�e���A���ɃR�s�[
*/
void FbxLoader::SetFbxColor(FbxMaterialElement* element, const FbxDouble3 color)
{
	element->a = 1.0f;
	element->r = static_cast<float>(color[0]);
	element->g = static_cast<float>(color[1]);
	element->b = static_cast<float>(color[2]);
}

/*
	@brief	FBX�p�̍s��f�[�^��ʏ��float�z��ɃR�s�[
*/
void FbxLoader::FbxMatrixToFloat16(FbxMatrix* matrix, float array[16])
{
	uint32_t index = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			array[index] = static_cast<float>(matrix->Get(i, j));
			index++;
		}
	}
}

/*
	@brief	�}�e���A���v�f�̎擾
*/
FbxDouble3 FbxLoader::GetMaterialProperty(const FbxSurfaceMaterial* material,const char* propertyName, const char* factorPropertyName, FbxMaterialElement* element)
{
	element->type = FbxMaterialElement::eNone;

	FbxDouble3 result(0.0, 0.0, 0.0);

	// �������g���ă}�e���A���v���p�e�B���擾
	const FbxProperty property = material->FindProperty(propertyName);
	const FbxProperty factorProperty = material->FindProperty(factorPropertyName);

	// �}�e���A���J���[�̎擾
	if (property.IsValid() && factorProperty.IsValid())
	{
		result = property.Get<FbxDouble3>();
		double factor = factorProperty.Get<FbxDouble>();
		if (factor != 1)
		{
			result[0] *= factor;
			result[1] *= factor;
			result[2] *= factor;
		}
		element->type = FbxMaterialElement::eColor;
	}

	// �e�N�X�`���̎擾����
	if (property.IsValid())
	{
		// �������擾�ł����e�N�X�`���𐔂���
		int existTextureCount = 0;

		// �}�e���A���ɂ���e�N�X�`�������擾
		const int textureCount = property.GetSrcObjectCount<FbxFileTexture>();
		for (int i = 0; i < textureCount; i++)
		{
			// �t�@�C�������擾
			FbxFileTexture* file = property.GetSrcObject<FbxFileTexture>(i);

			// �t�@�C���������Ȃ���Ζ���
			if (!file)
			{
				continue;
			}

			// UV�Z�b�g�����擾
			FbxString uvSet = file->UVSet.Get();
			std::string uvSetString = uvSet.Buffer();

			// �t�@�C�����̓t���p�X�œ����Ă���̂ŁA�]���ȃp�X���J�b�g���ăt�@�C�����݂̂ɂ��鏈��
			std::string filename = FbxPathUtils::GetFileName(file->GetFileName());

			// UV�Z�b�g�����g���āA�t�@�C������ۑ�
			element->textureSets[uvSetString].push_back(filename);

			// �e�N�X�`�������C���N�������g
			existTextureCount++;
		}

		// �}���`���C�����̃e�N�X�`���擾���������Ă���
		// ��{�I�ȂȂ���͏�Ɠ����B
		const int layeredTextureCount = property.GetSrcObjectCount<FbxLayeredTexture>();
		for (int i = 0; i < layeredTextureCount; i++)
		{
			FbxLayeredTexture* layeredTexture = property.GetSrcObject<FbxLayeredTexture>(i);
			const int fileCount = layeredTexture->GetSrcObjectCount<FbxFileTexture>();
			for (int j = 0; j < fileCount; j++)
			{
				FbxFileTexture* file = layeredTexture->GetSrcObject<FbxFileTexture>(j);
				if (!file)
				{
					continue;
				}

				FbxString uvSet = file->UVSet.Get();
				std::string uvSetString = uvSet.Buffer();
				std::string filename = file->GetFileName();

				element->textureSets[uvSetString].push_back(filename);
				existTextureCount++;
			}
		}

		// �e�N�X�`��������΃}�e���A���v�f�̐ݒ�
		if (existTextureCount > 0)
		{
			if (element->type == FbxMaterialElement::eColor)
			{
				element->type = FbxMaterialElement::eBoth;
			}
			else
			{
				element->type = FbxMaterialElement::eTexture;
			}
		}
	}

	return result;
}