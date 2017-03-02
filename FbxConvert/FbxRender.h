/*
	@file			FbxRender.h
	@brief		FBXファイルの描画
	@date		2017/02/28
	@author	仁科香苗
	@note		参照(https://github.com/shaderjp/FBXLoader2015forDX11)
*/
#pragma once
#include "Common.h"
#include "FbxLoader.h"

namespace FBX_LOADER
{
	//バーテックスデータ
	struct VERTEX_DATA
	{
		D3DXVECTOR3 position;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 textureCood;
	};

	//マテリアルの定数データ
	struct MATERIAL_CONSTANT_DATA
	{
		D3DXVECTOR4 ambient;
		D3DXVECTOR4 diffuse;
		D3DXVECTOR4 specular;
		D3DXVECTOR4 emmisive;
	};

	//マテリアルデータ
	struct MATERIAL_DATA
	{
		D3DXVECTOR4 ambient;
		D3DXVECTOR4 diffuse;
		D3DXVECTOR4 specular;
		D3DXVECTOR4 emmisive;
		float specularPower;
		float transparencyFactor;
		MATERIAL_CONSTANT_DATA materialConstantData;
		ID3D11ShaderResourceView*	srv;
		ID3D11SamplerState*         sampler;
		ID3D11Buffer*				materialCb;

		MATERIAL_DATA();
		void Release();
	};

	//メッシュノード
	struct MESH_NODE
	{
		//インテックスバッファのビット
		enum IndexBit
		{
			INDEX_NOINDEX = 0,
			INDEX_16BIT,
			INDEX_32BIT,
		};
		IndexBit indexBit;
		ID3D11Buffer*				vb;
		ID3D11Buffer*				ib;
		ID3D11InputLayout*		inputLayout;

		DWORD		vertexCount;
		DWORD		indexCount;

		MATERIAL_DATA materialData;

		//float	mat4x4[16];
		D3DXMATRIX mat4x4;
		MESH_NODE();
		void Release();
	};

	class FbxRender
	{
	public:
		FbxRender();
		~FbxRender();

		void Release();

		HRESULT LoadFBX(const char* filename, ID3D11Device*	pd3dDevice);
		HRESULT CreateInputLayout(ID3D11Device*	pd3dDevice, const void* pShaderBytecodeWithInputSignature, size_t BytecodeLength, D3D11_INPUT_ELEMENT_DESC* pLayout, unsigned int layoutSize);

		HRESULT RenderAll(ID3D11DeviceContext* pImmediateContext);
		HRESULT RenderNode(ID3D11DeviceContext* pImmediateContext, const size_t nodeId, ID3D11InputLayout* inputLayout);
		HRESULT RenderNodeInstancing(ID3D11DeviceContext* pImmediateContext, const size_t nodeId, const uint32_t InstanceCount);
		HRESULT RenderNodeInstancingIndirect(ID3D11DeviceContext* pImmediateContext, const size_t nodeId, ID3D11Buffer* pBufferForArgs, const uint32_t AlignedByteOffsetForArgs);

		size_t GetNodeCount() { return m_meshNodeArray.size(); }

		MESH_NODE& GetNode(const int id) { return m_meshNodeArray[id]; };
		D3DXMATRIX	GetNodeMatrix(const int id) { return m_meshNodeArray[id].mat4x4; };
		MATERIAL_DATA& GetNodeMaterial(const size_t id) { return m_meshNodeArray[id].materialData; };
	private:
		FbxLoader*		m_loader;
		FBX_MATERIAL_NODE* m_fbxMaterial;
		std::vector<MESH_NODE>	m_meshNodeArray;

		HRESULT CreateNodes(ID3D11Device*	pd3dDevice);
		HRESULT VertexConstruction(ID3D11Device*	pd3dDevice, FBX_MESH_NODE &fbxNode, MESH_NODE& meshNode);
		HRESULT MaterialConstruction(ID3D11Device*	pd3dDevice, FBX_MESH_NODE &fbxNode, MESH_NODE& meshNode);
		HRESULT CreateVertexBuffer(ID3D11Device*	pd3dDevice, ID3D11Buffer** pBuffer, void* pVertices, uint32_t stride, uint32_t vertexCount);
		HRESULT CreateIndexBuffer(ID3D11Device*	pd3dDevice, ID3D11Buffer** pBuffer, void* pIndices, uint32_t indexCount);

	};
};