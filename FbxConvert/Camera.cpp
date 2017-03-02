/*
	@file			Camera.cpp
	@brief		カメラ設定
	@date		2017/02/22
	@author	仁科香苗
*/
#include "Camera.h"

D3DXMATRIX Camera::m_view;
D3DXMATRIX Camera::m_proj;

/*
	@brief	コンストラクタ
*/
Camera::Camera()
	:m_pivotPos(0,0,0)
	,m_pos(0,0,0)
	,m_lookAtPos(0,0,0)
	,m_rot(0,0,0)
	,m_fovY(4)
	,m_nearZ(0.1f)
	,m_farZ(1000.0f)
{
}

/*
	@brief	デストラクタ
*/
Camera::~Camera()
{
}

/*
	@brief	初期化
*/
void Camera::Init()
{
}

/*
	@brief	解放
*/
void Camera::Destroy()
{

}

/*
	@brief	更新
*/
void Camera::Update()
{

}

/*
	@brief	描画
*/
void Camera::Render()
{
	D3DXVECTOR3 upVec(0, 1.0f, 0);	//上方位置

	D3DXMATRIX world, trans, rot;
	D3DXMatrixTranslation(&trans, m_pivotPos.x, m_pivotPos.y, m_pivotPos.z);
	D3DXMatrixRotationYawPitchRoll(&rot, m_rot.y, m_rot.x, m_rot.z);
	world = rot*trans;

	D3DXVec3TransformCoord(&m_pos, &m_pos, &world);
	D3DXVec3TransformCoord(&m_lookAtPos, &m_lookAtPos, &world);
	D3DXMatrixLookAtLH(&m_view, &m_pos, &m_lookAtPos, &upVec);
	// プロジェクショントランスフォーム（射影変換）
	D3DXMatrixPerspectiveFovLH(&m_proj, D3DX_PI / m_fovY, (FLOAT)window_width / (FLOAT)window_height, m_nearZ, m_farZ);
}