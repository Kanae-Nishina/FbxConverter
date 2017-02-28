/*
	@file			FbxLoader.h
	@brief		FBXファイルのローダー
	@date		2017/02/25
	@author	仁科香苗
*/
#include "FbxLoader.h"

//========================
//	FbxMaterialElement構造体
//========================
/*
	@brief	FbxMaterialElementデストラクタ
*/
FbxMaterialElement::~FbxMaterialElement()
{
	Release();
}

/*
	@brief	解放
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
//	MeshElements構造体
//========================
/*
	@brief	MeshElementsコンストラクタ
*/
MeshElements::MeshElements()
	:numPosition(0)
	,numNormal(0)
	,numUVSet(0)
{
}

//========================
//	FbxMeshNode構造体
//========================
/*
	@brief	FbxMeshNodeデストラクタ
*/
FbxMeshNode::~FbxMeshNode()
{
	Release();
}

/*
	@brief	解放
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
//	FbxLoaderクラス
//========================
/*
	@brief	FbxLoaderコンストラクタ
*/
FbxLoader::FbxLoader()
	:m_manager(nullptr)
	,m_scene(nullptr)
	,m_importer(nullptr)
{
}

/*
	@brief	FbxLoaderデストラクタ
*/
FbxLoader::~FbxLoader()
{
	Release();
}

/*
	@brief	モデルのロード
*/
bool FbxLoader::LoadModel(const char* filename)
{
	//fbxオブジェクトの生成
	m_manager = FbxManager::Create();
	auto ios = FbxIOSettings::Create(m_manager, IOSROOT);
	m_manager->SetIOSettings(ios);

	auto path = FbxGetApplicationDirectory();
	m_manager->LoadPluginsDirectory(path.Buffer());

	m_scene = FbxScene::Create(m_manager, "Scene");
	m_importer = FbxImporter::Create(m_manager, "");

	// インポート準備
	int fileFormat = -1;
	if (!m_manager->GetIOPluginRegistry()->DetectReaderFileFormat(filename, fileFormat))
	{
		fileFormat = m_manager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");
	}

	// データのインポート
	if (!m_importer->Initialize(filename, fileFormat))
	{
		return false;
	}

	if (!m_importer->Import(m_scene))
	{
		return false;
	}

	// fbxの軸系統をDirectX系にしておく
	if (m_scene->GetGlobalSettings().GetAxisSystem() != FbxAxisSystem::DirectX)
	{
		FbxAxisSystem::DirectX.ConvertScene(m_scene);
	}

	// ジオメトリコンバータの作成
	FbxGeometryConverter geometryConverter(m_manager);
	// ポリゴンを全て三角形化
	geometryConverter.Triangulate(m_scene, true);
	// マテリアルごとにメッシュを分割
	//geometryConverter.SplitMeshesPerMaterial(scene_, true);

	// パース開始
	SetUp();


	return true;
}

/*
	@brief	データの解放
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
	@brief	メッシュノードの取得
*/
FbxMeshNode& FbxLoader::GetNode(const uint32_t id)
{
	return m_meshNodes[id];
}

/*
	@brief	メッシュノード数の取得
*/
std::size_t FbxLoader::GetNodeCount()
{
	return m_meshNodes.size();
}

/*
	@breif	FBX全体のパース
*/
void FbxLoader::SetUp()
{
	if (m_scene->GetRootNode())
	{
		// 最初のノード名はnullって名前にしておく
		std::string name = "null";

		SetUpNode(m_scene->GetRootNode(), name);
	}
}

/*
	@brief	ノードをパース
*/
void FbxLoader::SetUpNode(FbxNode* node, std::string& parentName)
{
	if (m_scene->GetRootNode())
	{
		// 最初のノード名はnullって名前にしておく
		std::string name = "null";

		SetUpNode(m_scene->GetRootNode(), name);
	}
}

/*
	@brief	頂点データのコピー
*/
void FbxLoader::CopyVertex(FbxMesh* mesh, FbxMeshNode* meshNode)
{
	if (!mesh)
	{
		return;
	}

	// 頂点・法線データは１セットのみ対応
	meshNode->elements.numPosition = 1;
	meshNode->elements.numNormal = 1;

	// 頂点インデックス
	uint32_t polygonIndex = 0;

	// ポリゴン数を取得
	int polygonCount = mesh->GetPolygonCount();
	for (int i = 0; i < polygonCount; i++)
	{
		// ポリゴンサイズを取得
		int polygonSize = mesh->GetPolygonSize(i);
		for (int j = 0; j < polygonSize; j++)
		{
			// ポリゴンの番号と、頂点の番号を使ってインデックスを取得
			int index = mesh->GetPolygonVertex(i, j);
			meshNode->indices.push_back(polygonIndex);

			// インデックスから頂点座標を取得
			FbxVector4 position = mesh->GetControlPointAt(index);

			// ポリゴンの番号と、頂点の番号を使って法線データを取得
			FbxVector4 normal;
			mesh->GetPolygonVertexNormal(i, j, normal);

			// 頂点・法線のプッシュ
			meshNode->positions.push_back(position);
			meshNode->normals.push_back(normal);

			// インデックスをインクリメント
			++polygonIndex;
		}
	}

	// UVデータの構築
	FbxStringList uvSetNames;
	mesh->GetUVSetNames(uvSetNames);

	// UVは２つ以上あるときもある
	int uvSetCount = uvSetNames.GetCount();
	meshNode->elements.numUVSet = uvSetCount;

	bool unmapped = false;

	// UV数分ループ
	for (int uvIndex = 0; uvIndex < uvSetCount; uvIndex++)
	{
		// UVセットの番号を設定
		meshNode->uvSetID[uvSetNames.GetStringAt(uvIndex)] = uvIndex;
		for (int i = 0; i < polygonCount; i++)
		{
			int polygonSize = mesh->GetPolygonSize(i);
			for (int j = 0; j < polygonSize; j++)
			{
				FbxString name = uvSetNames.GetStringAt(uvIndex);

				FbxVector2 texCoord;
				// ポリゴンの番号と、頂点の番号・UVセット名を使ってUV座標を取得
				mesh->GetPolygonVertexUV(i, j, name, texCoord, unmapped);
				meshNode->texCoords.push_back(texCoord);
			}
		}
	}
}

/*
	@brief	マテリアルのコピー
*/
void FbxLoader::CopyMaterial(FbxSurfaceMaterial* material, FbxMaterialNode* materialNode)
{
	if (!material)
	{
		return;
	}

	// マテリアルの計算方法の設定
	if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		materialNode->type = FbxMaterialNode::eLambert;
	}
	else if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		materialNode->type = FbxMaterialNode::ePhong;
	}

	// マテリアル要素を取得

	// アンビエントカラー
	const FbxDouble3 ambient = GetMaterialProperty(material,
		FbxSurfaceMaterial::sAmbient,
		FbxSurfaceMaterial::sAmbientFactor,
		&materialNode->ambient);
	SetFbxColor(&materialNode->ambient, ambient);

	// ディフューズカラー
	const FbxDouble3 diffuse = GetMaterialProperty(material,
		FbxSurfaceMaterial::sDiffuse,
		FbxSurfaceMaterial::sDiffuseFactor,
		&materialNode->diffuse);
	SetFbxColor(&materialNode->diffuse, diffuse);

	// エミッシブカラー
	const FbxDouble3 emissive = GetMaterialProperty(material,
		FbxSurfaceMaterial::sEmissive,
		FbxSurfaceMaterial::sEmissiveFactor,
		&materialNode->emissive);
	SetFbxColor(&materialNode->emissive, emissive);

	// スペキュラカラー
	const FbxDouble3 specular = GetMaterialProperty(material,
		FbxSurfaceMaterial::sSpecular,
		FbxSurfaceMaterial::sSpecularFactor,
		&materialNode->specular);
	SetFbxColor(&materialNode->specular, specular);


	// 輝きのつよさ
	FbxProperty shininessProperty = material->FindProperty(FbxSurfaceMaterial::sShininess);
	if (shininessProperty.IsValid())
	{
		double shininess = shininessProperty.Get<FbxDouble>();
		materialNode->shininess = static_cast<float>(shininess);
	}

	// マテリアル自体の透過度
	FbxProperty transparencyProperty = material->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
	if (transparencyProperty.IsValid())
	{
		double transparency = transparencyProperty.Get<FbxDouble>();
		materialNode->transparency = static_cast<float>(transparency);
	}
}

/*
	@brief	ノードの行列を計算
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
	@brief	FBX用カラーデータをマテリアルにコピー
*/
void FbxLoader::SetFbxColor(FbxMaterialElement* element, const FbxDouble3 color)
{
	element->a = 1.0f;
	element->r = static_cast<float>(color[0]);
	element->g = static_cast<float>(color[1]);
	element->b = static_cast<float>(color[2]);
}

/*
	@brief	FBX用の行列データを通常にfloat配列にコピー
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
	@brief	マテリアル要素の取得
*/
FbxDouble3 FbxLoader::GetMaterialProperty(const FbxSurfaceMaterial* material,const char* propertyName, const char* factorPropertyName, FbxMaterialElement* element)
{
	element->type = FbxMaterialElement::eNone;

	FbxDouble3 result(0.0, 0.0, 0.0);

	// 引数を使ってマテリアルプロパティを取得
	const FbxProperty property = material->FindProperty(propertyName);
	const FbxProperty factorProperty = material->FindProperty(factorPropertyName);

	// マテリアルカラーの取得
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

	// テクスチャの取得処理
	if (property.IsValid())
	{
		// 正しく取得できたテクスチャを数える
		int existTextureCount = 0;

		// マテリアルにあるテクスチャ数を取得
		const int textureCount = property.GetSrcObjectCount<FbxFileTexture>();
		for (int i = 0; i < textureCount; i++)
		{
			// ファイル名を取得
			FbxFileTexture* file = property.GetSrcObject<FbxFileTexture>(i);

			// ファイル名が取れなければ無視
			if (!file)
			{
				continue;
			}

			// UVセット名を取得
			FbxString uvSet = file->UVSet.Get();
			std::string uvSetString = uvSet.Buffer();

			// ファイル名はフルパスで入っているので、余分なパスをカットしてファイル名のみにする処理
			std::string filename = FbxPathUtils::GetFileName(file->GetFileName());

			// UVセット名を使って、ファイル名を保存
			element->textureSets[uvSetString].push_back(filename);

			// テクスチャ数をインクリメント
			existTextureCount++;
		}

		// マルチレイヤ時のテクスチャ取得処理もしておく
		// 基本的なながれは上と同じ。
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

		// テクスチャがあればマテリアル要素の設定
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