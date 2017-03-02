/*
	@file			FbxLoader.h
	@brief		FBXファイルのローダー
	@date		2017/02/25
	@author	仁科香苗
*/
#include "FbxLoader.h"

//=================================
//	FBX_MATERIAL_ELEMENT構造体
//=================================
/*
	@brief	コンストラクタ
*/
FBX_MATERIAL_ELEMENT::FBX_MATERIAL_ELEMENT()
{
	textureSetArray.clear();
}

/*
	@brief	デストラクタ
*/
FBX_MATERIAL_ELEMENT::~FBX_MATERIAL_ELEMENT()
{
	Release();
}

/*
	@brief	解放
*/
void FBX_MATERIAL_ELEMENT::Release()
{
	for (TextureSet::iterator it = textureSetArray.begin(); it != textureSetArray.end(); ++it)
	{
		it->second.clear();
	}
	textureSetArray.clear();
}

//=================================
//	FBX_MESH_NODE構造体
//=================================
/*
	@brief	デストラクタ
*/
FBX_MESH_NODE::~FBX_MESH_NODE()
{
	Release();
}

/*
	@brief	解放
*/
void FBX_MESH_NODE::Release()
{
	uvsetID.clear();
	texcoordArray.clear();
	materialArray.clear();
	indexArray.clear();
	positionArray.clear();
	normalArray.clear();
}

//=================================
//	CFBXLoader構造体
//=================================
/*
	@brief	コンストラクタ
*/
FbxLoader::FbxLoader()
	:m_sdkManager(nullptr)
	, m_scene(nullptr)
{
}

/*
	@brief	デストラクタ
*/
FbxLoader::~FbxLoader()
{
	Release();
}

/*
	@brief	解放
*/
void FbxLoader::Release()
{
	m_meshNodeArray.clear();

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

	if (m_sdkManager)
	{
		m_sdkManager->Destroy();
		m_sdkManager = nullptr;
	}
}

/*
	@brief	FBXファイルのロード
*/
HRESULT FbxLoader::LoadFBX(const char* fileName)
{
	if (!fileName)
	{
		MessageBox(0, L"Not FBX FileName", NULL, MB_OK);
		return E_FAIL;
	}
	HRESULT hr = S_OK;
	InitializeSdkObjects(m_sdkManager, m_scene);
	if (!m_sdkManager)
	{
		MessageBox(0, L"Can't Init SDK Manager", NULL, MB_OK);
		return E_FAIL;
	}

	//インポータの作成
	int lFileFormat = -1;
	m_importer = fbxsdk::FbxImporter::Create(m_sdkManager, "");
	if (!m_sdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(fileName, lFileFormat))
	{
		//認識できない形式。FBXのバイナリ―にフォールバック
		lFileFormat = m_sdkManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");;
	}

	//ファイル名を指定しインポータを初期化
	if (!m_importer || m_importer->Initialize(fileName, lFileFormat) == false)
	{
		MessageBox(0, L"Can't Init Importer", NULL, MB_OK);
		return E_FAIL;
	}
	if (!m_importer || m_importer->Import(m_scene) == false)
	{
		MessageBox(0, L"Can't Import Scene", NULL, MB_OK);
		return E_FAIL;
	}

	//DirectXに変換
	FbxAxisSystem OurAxisSystem = FbxAxisSystem::DirectX;
	FbxAxisSystem SceneAxisSystem = m_scene->GetGlobalSettings().GetAxisSystem();
	if (SceneAxisSystem != OurAxisSystem)
	{
		FbxAxisSystem::DirectX.ConvertScene(m_scene);
	}

#if 0
	// 単位系の統一
	// 不要でもいいかも
	FbxSystemUnit SceneSystemUnit = m_scene->GetGlobalSettings().GetSystemUnit();
	if (SceneSystemUnit.GetScaleFactor() != 1.0)
	{
		// センチメーター単位にコンバートする
		FbxSystemUnit::cm.ConvertScene(m_scene);
	}
#endif

	// 三角形化(三角形以外のデータでもコレで安心)
	TriangulateRecursive(m_scene->GetRootNode());

	Setup();

	return hr;
}

/*
	@brief	SDK周りの初期化
*/
void FbxLoader::InitializeSdkObjects(FbxManager*& manager, FbxScene*& scene)
{
	//FBXマネージャの作成
	manager = FbxManager::Create();
	if (!manager)
	{
		MessageBox(0, L"Not SDK Manager", NULL, MB_OK);
		exit(1);
	}

	//IOSettingsオブジェクトの作成。全てのインポート・エクスポート設定を保持する。
	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);

	//実行可能ディレクトリからプラグインをロード
	FbxString lPath = FbxGetApplicationDirectory();
	manager->LoadPluginsDirectory(lPath.Buffer());

	//FBXシーンの作成。ファイルからインポート/エクスポートされるほとんどのオブジェクトを保持する。
	scene = FbxScene::Create(manager, "My Scene");
	if (!scene)
	{
		MessageBox(0, L"Not FbxScene", NULL, MB_OK);
		exit(1);
	}
}

/*
	@brief	三角形化
*/
void FbxLoader::TriangulateRecursive(FbxNode* node)
{
	FbxNodeAttribute* lNodeAttribute = node->GetNodeAttribute();

	if (lNodeAttribute)
	{
		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbsSurface ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch)
		{
			FbxGeometryConverter lConverter(node->GetFbxManager());
			lConverter.Triangulate(m_scene, true);
		}
	}

	const int lChildCount = node->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		// 子ノードを探索
		TriangulateRecursive(node->GetChild(lChildIndex));
	}
}

/*
	@brief	ルートノードの取得
*/
FbxNode& FbxLoader::GetRootNode()
{
	return *m_scene->GetRootNode();
}

/*
	@brief	パース
*/
void FbxLoader::Setup()
{
	if (m_scene->GetRootNode())
	{
		SetupNode(m_scene->GetRootNode(), "null");
	}
}

/*
	@brief	ノードの設定
*/
void FbxLoader::SetupNode(FbxNode* node, std::string parentName)
{
	if (!node)
	{
		return;
	}

	FBX_MESH_NODE meshNode;
	meshNode.name = node->GetName();
	meshNode.parentName = parentName;
	ZeroMemory(&meshNode.elements, sizeof(MESH_ELEMENTS));

	FbxMesh* lMesh = node->GetMesh();
	if (lMesh)
	{
		const int lVertexCount = lMesh->GetControlPointsCount();

		if (lVertexCount > 0)
		{
			//頂点があるならノードにコピー
			CopyVertexData(lMesh, &meshNode);
		}
	}

	//マテリアル
	const int lMaterialCount = node->GetMaterialCount();
	for (int i = 0; i < lMaterialCount; i++)
	{
		FbxSurfaceMaterial* mat = node->GetMaterial(i);
		if (!mat)
		{
			continue;
		}
		FBX_MATERIAL_NODE destMat;
		CopyMatrialData(mat, &destMat);

		meshNode.materialArray.push_back(destMat);
	}

	ComputeNodeMatrix(node, &meshNode);
	m_meshNodeArray.push_back(meshNode);
	const int lCount = node->GetChildCount();
	for (int i = 0; i < lCount; i++)
	{
		SetupNode(node->GetChild(i), meshNode.name);
	}
}

/*
	@brief	FBXのカラー情報セット
*/
void FbxLoader::SetFbxColor(FBX_MATERIAL_ELEMENT& destColor, const FbxDouble3 srcColor)
{
	destColor.a = 1.0f;
	destColor.r = static_cast<float>(srcColor[0]);
	destColor.g = static_cast<float>(srcColor[1]);
	destColor.b = static_cast<float>(srcColor[2]);
}

/*
	@brief	マテリアルプロパティの取得
*/
FbxDouble3 FbxLoader::GetMaterialProperty(const FbxSurfaceMaterial * pMaterial, const char * pPropertyName,
	const char * pFactorPropertyName, FBX_MATERIAL_ELEMENT*	pElement)
{
	pElement->type = FBX_MATERIAL_ELEMENT::ELEMENT_NONE;

	FbxDouble3 lResult(0, 0, 0);
	const FbxProperty lProperty = pMaterial->FindProperty(pPropertyName);
	const FbxProperty lFactorProperty = pMaterial->FindProperty(pFactorPropertyName);
	if (lProperty.IsValid() && lFactorProperty.IsValid())
	{
		lResult = lProperty.Get<FbxDouble3>();
		double lFactor = lFactorProperty.Get<FbxDouble>();
		if (lFactor != 1)
		{
			lResult[0] *= lFactor;
			lResult[1] *= lFactor;
			lResult[2] *= lFactor;
		}

		pElement->type = FBX_MATERIAL_ELEMENT::ELEMENT_COLOR;
	}

	if (lProperty.IsValid())
	{
		int existTextureCount = 0;
		const int lTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();

		for (int i = 0; i < lTextureCount; i++)
		{
			FbxFileTexture* lFileTexture = lProperty.GetSrcObject<FbxFileTexture>(i);
			if (!lFileTexture)
				continue;

			FbxString uvsetName = lFileTexture->UVSet.Get();
			std::string uvSetString = uvsetName.Buffer();
			std::string filepath = lFileTexture->GetFileName();

			pElement->textureSetArray[uvSetString].push_back(filepath);
			existTextureCount++;
		}

		const int lLayeredTextureCount = lProperty.GetSrcObjectCount<FbxLayeredTexture>();

		for (int i = 0; i < lLayeredTextureCount; i++)
		{
			FbxLayeredTexture* lLayeredTexture = lProperty.GetSrcObject<FbxLayeredTexture>(i);

			const int lTextureFileCount = lLayeredTexture->GetSrcObjectCount<FbxFileTexture>();

			for (int j = 0; j < lTextureFileCount; j++)
			{
				FbxFileTexture* lFileTexture = lLayeredTexture->GetSrcObject<FbxFileTexture>(j);
				if (!lFileTexture)
					continue;

				FbxString uvsetName = lFileTexture->UVSet.Get();
				std::string uvSetString = uvsetName.Buffer();
				std::string filepath = lFileTexture->GetFileName();

				pElement->textureSetArray[uvSetString].push_back(filepath);
				existTextureCount++;
			}
		}

		if (existTextureCount > 0)
		{
			if (pElement->type == FBX_MATERIAL_ELEMENT::ELEMENT_COLOR)
				pElement->type = FBX_MATERIAL_ELEMENT::ELEMENT_BOTH;
			else
				pElement->type = FBX_MATERIAL_ELEMENT::ELEMENT_TEXTURE;
		}
	}

	return lResult;
}

/*
	@brief	マテリアルデータのコピー
*/
void FbxLoader::CopyMatrialData(FbxSurfaceMaterial* mat, FBX_MATERIAL_NODE* destMat)
{
	if (!mat)
	{
		return;
	}

	if (mat->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		destMat->type = FBX_MATERIAL_NODE::MATERIAL_LAMBERT;
	}
	else if (mat->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		destMat->type = FBX_MATERIAL_NODE::MATERIAL_PHONG;
	}

	const FbxDouble3 lEmissive = GetMaterialProperty(mat,FbxSurfaceMaterial::sEmissive, 
		FbxSurfaceMaterial::sEmissiveFactor, &destMat->emmisive);
	SetFbxColor(destMat->emmisive, lEmissive);

	const FbxDouble3 lAmbient = GetMaterialProperty(mat,FbxSurfaceMaterial::sAmbient, 
		FbxSurfaceMaterial::sAmbientFactor, &destMat->ambient);
	SetFbxColor(destMat->ambient, lAmbient);

	const FbxDouble3 lDiffuse = GetMaterialProperty(mat,FbxSurfaceMaterial::sDiffuse, 
		FbxSurfaceMaterial::sDiffuseFactor, &destMat->diffuse);
	SetFbxColor(destMat->diffuse, lDiffuse);

	const FbxDouble3 lSpecular = GetMaterialProperty(mat,FbxSurfaceMaterial::sSpecular, 
		FbxSurfaceMaterial::sSpecularFactor, &destMat->specular);
	SetFbxColor(destMat->specular, lSpecular);

	//
	FbxProperty lTransparencyFactorProperty = mat->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
	if (lTransparencyFactorProperty.IsValid())
	{
		double lTransparencyFactor = lTransparencyFactorProperty.Get<FbxDouble>();
		destMat->transparencyFactor = static_cast<float>(lTransparencyFactor);
	}

	// Specular Power
	FbxProperty lShininessProperty = mat->FindProperty(FbxSurfaceMaterial::sShininess);
	if (lShininessProperty.IsValid())
	{
		double lShininess = lShininessProperty.Get<FbxDouble>();
		destMat->shininess = static_cast<float>(lShininess);
	}
}

/*
	@brief	ノードの行列の計算
*/
void FbxLoader::ComputeNodeMatrix(FbxNode* pNode, FBX_MESH_NODE* meshNode)
{
	if (!pNode || !meshNode)
	{
		return;
	}

	//FbxAnimEvaluator* lEvaluator = mScene->GetEvaluator();
	FbxAnimEvaluator* lEvaluator = m_scene->GetAnimationEvaluator();
	FbxMatrix lGlobal;
	lGlobal.SetIdentity();

	if (pNode != m_scene->GetRootNode())
	{
		lGlobal = lEvaluator->GetNodeGlobalTransform(pNode);

		FBXMatrixToFloat16(&lGlobal, meshNode->mat4x4);
	}
	else
	{
		FBXMatrixToFloat16(&lGlobal, meshNode->mat4x4);
	}
}

/*
	@brief	バーテックスデータのコピー
*/
void FbxLoader::CopyVertexData(FbxMesh*	pMesh, FBX_MESH_NODE* meshNode)
{
	if (!pMesh)
	{
		return;
	}

	int lPolygonCount = pMesh->GetPolygonCount();

	FbxVector4 pos, nor;

	meshNode->elements.numPosition = 1;
	meshNode->elements.numNormal = 1;

	unsigned int indx = 0;

	for (int i = 0; i<lPolygonCount; i++)
	{
		// ポリゴン内の頂点数(一応、三角形化してるので3点のはずだがチェック)
		int lPolygonsize = pMesh->GetPolygonSize(i);

		for (int pol = 0; pol<lPolygonsize; pol++)
		{
			int index = pMesh->GetPolygonVertex(i, pol);
			meshNode->indexArray.push_back(indx);

			pos = pMesh->GetControlPointAt(index);
			pMesh->GetPolygonVertexNormal(i, pol, nor);

			meshNode->positionArray.push_back(pos);
			meshNode->normalArray.push_back(nor);

			++indx;
		}
	}

	// UV処理(UVは2つ以上ある場合があるので別処理)
	FbxStringList	uvsetName;
	pMesh->GetUVSetNames(uvsetName);
	int numUVSet = uvsetName.GetCount();
	meshNode->elements.numUVSet = numUVSet;

	bool unmapped = false;

	for (int uv = 0; uv<numUVSet; uv++)
	{
		meshNode->uvsetID[uvsetName.GetStringAt(uv)] = uv;
		for (int i = 0; i<lPolygonCount; i++)
		{
			int lPolygonsize = pMesh->GetPolygonSize(i);

			for (int pol = 0; pol<lPolygonsize; pol++)
			{
				FbxString name = uvsetName.GetStringAt(uv);
				FbxVector2 texCoord;
				pMesh->GetPolygonVertexUV(i, pol, name, texCoord, unmapped);
				meshNode->texcoordArray.push_back(texCoord);
			}
		}
	}
}

/*
	@brief	ノードの取得
*/
FBX_MESH_NODE& FbxLoader::GetNode(const unsigned int id)
{
	return m_meshNodeArray[id];
}

/*
	@brief	マトリックスをfloatへ変換
*/
void FbxLoader::FBXMatrixToFloat16(FbxMatrix* src, float dest[16])
{
	unsigned int nn = 0;
	for (int i = 0; i<4; i++)
	{
		for (int j = 0; j<4; j++)
		{
			dest[nn] = static_cast<float>(src->Get(i, j));
			nn++;
		}
	}
}