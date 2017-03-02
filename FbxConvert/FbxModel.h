/*
	@file			FbxModel.h
	@brief		FBX�V�F�[�_�N���X
	@date		2017/03/01
	@author	�m�ȍ��c
*/
#pragma once
#include "Common.h"
#include "FbxRender.h"

/*
	@brief	�}�g���b�N�X
*/
struct FBX_MATRIX
{
	D3DXMATRIX world;
	D3DXMATRIX view;
	D3DXMATRIX proj;
	D3DXMATRIX wvp;
};

/*
	@brief	�R���X�^���g�o�b�t�@
*/
struct SIMPLECONSTANT_BUFFER
{
	D3DXMATRIX world;//���[���h�s��
	D3DXMATRIX wvp;//���[���h����ˉe�܂ł̕ϊ��s��
	D3DXVECTOR4 lightDir;//���C�g����
	D3DXVECTOR4 eye;//�J�����ʒu
};


/*
	@brief	�C���X�^���X���f�[�^
*/
struct SRV_PER_INSTANCE_DATA
{
	D3DXMATRIX world;
};

/*
	@brief	FBX���f���Ǘ��N���X
*/
class FbxModel
{
public:
	FbxModel();
	~FbxModel();

	HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	HRESULT LoadFBX(const char* fileName);
	void SetCamera(D3DXMATRIX view, D3DXMATRIX proj);
	void Render(D3DXVECTOR3 pos,float scale,float yaw);
	void Release();
private:
	bool											m_instancing;
	const unsigned int						m_instanceMAX;
	FBX_LOADER::FbxRender*			m_fbxRender;
	ID3D11Device*							m_d3dDevice;
	ID3D11DeviceContext*				m_deviceContext;
	ID3D11BlendState*					m_blendState;
	ID3D11RasterizerState*				m_rs;
	ID3D11Buffer*							m_buffer;
	ID3D11VertexShader*				m_vsFBX;
	ID3D11PixelShader*					m_psFBX;
	ID3D11VertexShader*				m_vsFBXInstancing;
	ID3D11InputLayout*					m_inputLayout;
	ID3D11Buffer*							m_transformStructuredBuffer;
	ID3D11ShaderResourceView*		m_transformSRV ;
	FBX_MATRIX*							m_fbxMatrix;
	D3DXMATRIX								m_view;
	D3DXMATRIX								m_proj;

	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShader();
	HRESULT CreateTransdormSRV();
	void SetMatrix();
};